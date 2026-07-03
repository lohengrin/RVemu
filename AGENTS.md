# AGENTS.md

RISC-V (RV64) emulator in C++20, built with CMake. Targets the QEMU `virt` machine layout. Not a monorepo.

## Build

Requires CMake >= 4.0, a C++20 compiler, and the vcpkg toolchain (manifest mode — `vcpkg.json` pins `elfio` and `gtest`).

```bash
cmake -B build -DCMAKE_TOOLCHAIN_FILE=/home/lohengrin/Dev/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build          # installs the RVemu binary to ./bin/
```

The toolchain file is already set in `.vscode/settings.json`. C++20 is required: ELFIO 3.12 uses `std::string_view` (C++17+), and the build is verified with GCC 15.

CMake options (top-level `CMakeLists.txt`):
- `WITH_ELF` (default `ON`) — enables ELF loading via the ELFIO library. Requires `elfio::elfio` (from vcpkg). With `-DWITH_ELF=OFF` only raw `.bin` images can be loaded.
- `WITH_GUI` (default `OFF`) — Qt5 debugging GUI. Needs Qt5 Widgets; switches the entrypoint from `src/main.cpp` to `src/GUI/mainGUI.cpp`. Tests are skipped when `WITH_GUI` is on.
- `WITH_TESTS` (default `ON`, ignored under `WITH_GUI`) — builds the GoogleTest suite under `tests/`.

On Linux the core library additionally links `pthread`.

The emulator core (`Cpu`/`Bus`/`Memory`/`Uart`/`VirtIO`/`Clint`/`Plic`/`Trap`, plus `ElfLoader` when `WITH_ELF`) is built as the **`RVemuCore` static library** in `src/CMakeLists.txt`; both the `RVemu` executable and the test executable link it.

## Run

```bash
bin/RVemu <image> [disk.img]
```

 - `<image>` may be an ELF (`*.elf`, when `WITH_ELF=ON`) or raw binary (`*.bin`); ELF is auto-detected, otherwise loaded raw at `DRAM_BASE`.
- Optional second arg is a virtio disk image.
- Boot xv6: `bin/RVemu test/xv6-kernel.bin test/xv6-fs.img`.
- Guest UART output (writes to `0x10000000`) goes to stdout.

## Benchmark

Build a separate `RVemuBench` executable that measures xv6 boot time:

```bash
cmake -B build -DWITH_BENCHMARK=ON
cmake --build build
cmake --install build
bin/RVemuBench test/xv6-kernel.bin test/xv6-fs.img
```

The benchmark boots xv6, monitors UART output for `"init: starting sh"`, and prints the wall-clock time to completion. Times out after 240 seconds if the string never appears. The `RVemuBench` binary is skipped when `WITH_BENCHMARK=OFF` (the default).

CMake option:
- `WITH_BENCHMARK` (default `OFF`) — builds the `RVemuBench` executable from `src/benchmark_main.cpp`.

## Validation

Build a separate `RVemuValidate` executable that verifies xv6 boots to userspace:

```bash
cmake -B build -DWITH_VALIDATION=ON
cmake --build build
build/src/RVemuValidate test/xv6-kernel.bin test/xv6-fs.img
```

The validation boots xv6, monitors UART output for `"init: starting sh"`, and exits with code 0 on success or 1 on failure/timeout. Times out after 240 seconds. The `RVemuValidate` binary is skipped when `WITH_VALIDATION=OFF` (the default).

CMake option:
- `WITH_VALIDATION` (default `OFF`) — builds the `RVemuValidate` executable from `src/validation_main.cpp`.

## ELF Loading

The ELF loader (`src/ElfLoader.cpp`) supports loading RV64 ELF executables when `WITH_ELF=ON`. Key features:
- Uses program headers (PT_LOAD segments) instead of section headers for proper executable loading
- Handles memory address translation with DRAM_BASE (0x80000000) offset
- Supports multiple loadable segments (code, data, BSS)
- Properly zero-fills BSS segments when `p_memsz > p_filesz`
- Captures entry point from ELF header for PC initialization
- Bounds checking to prevent memory corruption
- Symbol extraction for compliance tests (begin_signature, end_signature, _start, __reset, __irq_wrapper)

Test ELF files are compiled using the riscv64-unknown-elf toolchain and stored in `test/*.elf` for validation.

## Tests

Automated host-side unit tests live under `tests/` (GoogleTest, registered via `gtest_discover_tests`):

```bash
ctest --test-dir build --output-on-failure   # run the suite
ctest --test-dir build -R CpuInstruction      # run one fixture / case by name
```

Coverage is the RV64I core: `Memory`/`Bus`/`Clint`/`Plic`/`VirtIO`/`Uart` devices, `ElfLoader`, and the `Cpu` instruction matrix (OP-IMM, OP, RV32W, U-type, branches, `jal`/`jalr`, load/store at all widths with sign/zero extension, `x0` hardwiring, CSR ops). Tests drive the real `fetch -> forwardPC -> decode -> execute` pipeline on hand-encoded instruction words.

Two PC convention gotchas when adding CPU tests: `forwardPC()` is applied **before** `execute()`, so inside `execute` `pc` already points past the current instruction (PC-relative forms subtract 4 internally). And load/store addresses must be `>= DRAM_BASE` — the Bus faults (fatally) on unmapped addresses, so build addresses with `auipc`/`addi` rather than as bare immediates.

The separate `test/` directory (note: singular, distinct from `tests/`) holds RISC-V **guest** programs (`.s`/`.c` plus prebuilt `.bin`/`.elf`) that you run *under the emulator* to validate changes by observing UART output. Rebuilding those guest binaries needs a `riscv64` cross-toolchain and is not part of the CMake build.

## Architecture notes

- Headless entry: `src/main.cpp`. GUI entry: `src/GUI/mainGUI.cpp` (only compiled with `WITH_GUI`).
- Components are memory-mapped devices registered on a `Bus` (`Bus.cpp`); device base addresses are the QEMU-virt layout defined in `src/Defines.h`:
  - DRAM `0x80000000`, UART `0x10000000`, VIRTIO `0x10001000`, CLINT `0x02000000`, PLIC `0x0c000000`.
- CPU fetch/decode/execute loop and CSRs live in `Cpu.cpp` / `CpuUtils.cpp` / `Trap.cpp`.
- The main loop in `src/main.cpp` runs a straight `fetch -> forwardPC -> decode -> execute -> check-interrupt` cycle with no per-instruction logging. `Memory::loadbin` still prints a per-byte hex dump of the loaded image once at startup. `bin.txt` and `elf.txt` in the repo root are historical captured debug dumps (from when the loop was verbose), not source or config.

## Performance considerations

Current implementation has several optimization opportunities for improved emulation speed:

### Hot path optimizations (high impact):
- **Bus device lookup**: Linear search O(n) on every memory access — consider sorting devices and using binary search or caching DRAM pointer
- **Exception handling**: Try/catch in hot paths — move to higher-level error handling
- **Interrupt checking**: Called every instruction, rarely triggered — use `[[unlikely]]` attribute and cache device pointers
- **TLB implementation**: No translation lookaside buffer — add simple TLB for paging performance (20-40% improvement with paging)

### Medium impact optimizations:
- **Type-punning**: Undefined behavior in Memory.cpp — use `std::memcpy` for better optimization
- **Register storage**: Vector allocation vs fixed arrays — use `std::array` for better cache locality
- **Dispatch table**: Large switch statement — consider computed goto or jump table for branch prediction

### Low effort, high impact wins:
1. Fix type-punning in Memory.cpp (`std::memcpy` with endian-aware byte swapping)
2. Cache device pointers for interrupt checking
3. Remove redundant x0 register writes
4. Use `[[unlikely]]` for rarely-taken branches
5. Sort Bus devices for binary search lookup

Estimated total potential improvement: **40-80%** depending on workload (paging vs. no paging, I/O vs. compute-bound).

## Style

CMake files use tabs; C++ mixes tabs and spaces — match the surrounding file. Code style otherwise follows existing files.
