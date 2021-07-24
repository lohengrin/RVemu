#include "Bus.h"
#include "Trap.h"

#include <iostream>


//! Device configuration
bool Bus::addDevice(uint64_t baseaddr, Device* dev)
{
	if (!dev)
		return false;
	myDevices.push_back(DeviceEntry(baseaddr, dev->size(), dev));
	return true;
}

Device* Bus::getDevice(uint64_t baseaddr)
{
	for (auto&& dev : myDevices)
	{
		if (dev.base == baseaddr)
			return dev.device;
	}
	return nullptr;
}


uint64_t Bus::load(uint64_t addr, uint8_t size) const
{
	for (auto&& dev : myDevices)
	{
		if (dev.base <= addr && addr < dev.end)
			return dev.device->load(addr - dev.base, size);
	}

	throw(CpuException(Except::LoadAccessFault));
}

void Bus::store(uint64_t addr, uint8_t size, uint64_t value)
{
	for (const auto& dev : myDevices)
	{
		if (dev.base <= addr && addr < dev.end)
		{
			dev.device->store(addr - dev.base, size, value);
			return;
		}
	}

	throw(CpuException(Except::StoreAMOAccessFault));
}
