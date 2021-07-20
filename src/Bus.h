#pragma once

#include "Device.h"

#include <map>

class Bus {
public:
	Bus() {}
	virtual ~Bus() {}

	//! Device configuration
	bool addDevice(uint64_t baseaddr, Device* dev);
	Device* getDevice(uint64_t baseaddr);

	uint64_t load(uint64_t addr, uint8_t size) const;
	void store(uint64_t addr, uint8_t size, uint64_t value);

protected:

	std::map<uint64_t, Device*> myDevices;
};