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

## Tests

Automated host-side unit tests live under `tests/` (GoogleTest, registered via `gtest_discover_tests`):

```bash
ctest --test-dir build --output-on-failure   # run the suite
ctest --test-dir build -R CpuInstruction      # run one fixture / case by name
```

Coverage is the RV64I core: `Memory`/`Bus`/`Clint`/`Plic`/`VirtIO`/`Uart` devices, and the `Cpu` instruction matrix (OP-IMM, OP, RV32W, U-type, branches, `jal`/`jalr`, load/store at all widths with sign/zero extension, `x0` hardwiring, CSR ops). Tests drive the real `fetch -> forwardPC -> decode -> execute` pipeline on hand-encoded instruction words.

Two PC convention gotchas when adding CPU tests: `forwardPC()` is applied **before** `execute()`, so inside `execute` `pc` already points past the current instruction (PC-relative forms subtract 4 internally). And load/store addresses must be `>= DRAM_BASE` — the Bus faults (fatally) on unmapped addresses, so build addresses with `auipc`/`addi` rather than as bare immediates.

The separate `test/` directory (note: singular, distinct from `tests/`) holds RISC-V **guest** programs (`.s`/`.c` plus prebuilt `.bin`/`.elf`) that you run *under the emulator* to validate changes by observing UART output. Rebuilding those guest binaries needs a `riscv64` cross-toolchain and is not part of the CMake build.

## Architecture notes

- Headless entry: `src/main.cpp`. GUI entry: `src/GUI/mainGUI.cpp` (only compiled with `WITH_GUI`).
- Components are memory-mapped devices registered on a `Bus` (`Bus.cpp`); device base addresses are the QEMU-virt layout defined in `src/Defines.h`:
  - DRAM `0x80000000`, UART `0x10000000`, VIRTIO `0x10001000`, CLINT `0x02000000`, PLIC `0x0c000000`.
- CPU fetch/decode/execute loop and CSRs live in `Cpu.cpp` / `CpuUtils.cpp` / `Trap.cpp`.
- The main loop in `src/main.cpp` runs a straight `fetch -> forwardPC -> decode -> execute -> check-interrupt` cycle with no per-instruction logging. `Memory::loadbin` still prints a per-byte hex dump of the loaded image once at startup. `bin.txt` and `elf.txt` in the repo root are historical captured debug dumps (from when the loop was verbose), not source or config.

## Style

CMake files use tabs; C++ mixes tabs and spaces — match the surrounding file. Code style otherwise follows existing files.
