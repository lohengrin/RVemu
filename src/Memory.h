#pragma once

#include "Device.h"

#include <vector>
#include <stdint.h>

// Default is 128MiB
const size_t DEFAULT_MEMORYSIZE = 1024 * 1024 * 128;

class Memory : public Device {
public:
	//! Init
	Memory(size_t size = DEFAULT_MEMORYSIZE);
	virtual ~Memory();

	//! Device Interface
	//!load
	uint64_t load(uint64_t addr, uint8_t size) const;
	//! store
	void store(uint64_t addr, uint8_t size, uint64_t value);
	//! Get address space size of device
	uint64_t size() const {	return dram.size(); }

	//! Preload memory
	void preload(size_t addr, uint8_t* data, size_t size);

	//! Get base memory adress
	const uint8_t* base() const { return &(dram[0]); }


protected:
	uint64_t load8(uint64_t addr) const;
	uint64_t load16(uint64_t addr) const;
	uint64_t load32(uint64_t addr) const;
	uint64_t load64(uint64_t addr) const;

	void store8(uint64_t addr, uint64_t value);
	void store16(uint64_t addr, uint64_t value);
	void store32(uint64_t addr, uint64_t value);
	void store64(uint64_t addr, uint64_t value);

	std::vector<uint8_t> dram;
};