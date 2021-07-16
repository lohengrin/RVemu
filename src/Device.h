#pragma once

#include "Defines.h"

class Device {
public:
	virtual ~Device() {}

	//!load
	virtual uint64_t load(uint64_t addr, uint8_t size) const = 0;

	//! store
	virtual void store(uint64_t addr, uint8_t size, uint64_t value) = 0;
};