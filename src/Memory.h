#pragma once

#include <vector>
#include <stdint.h>

// Default is 128MiB
const size_t DEFAULT_MEMORYSIZE = 1024 * 1024 * 128;
const uint64_t DRAM_BASE = 0x80000000;

class Memory {
public:
	//! Init
	Memory(size_t size = DEFAULT_MEMORYSIZE);
	virtual ~Memory();

	//! Preload memory
	void preload(size_t addr, uint8_t* data, size_t size);

	//! Get base memory adress
	const uint8_t* base() const { return &(dram[0]); }

	//!load
	uint64_t load(uint64_t addr, uint8_t size);

	//! store
	void store(uint64_t addr, uint8_t size, uint64_t value);

protected:
	uint64_t load8(uint64_t addr);
	uint64_t load16(uint64_t addr);
	uint64_t load32(uint64_t addr);
	uint64_t load64(uint64_t addr);

	void store8(uint64_t addr, uint64_t value);
	void store16(uint64_t addr, uint64_t value);
	void store32(uint64_t addr, uint64_t value);
	void store64(uint64_t addr, uint64_t value);

	std::vector<uint8_t> dram;
};