#include "Bus.h"
#include "Trap.h"

#include <iostream>


//! Device configuration
bool Bus::addDevice(uint64_t baseaddr, Device* dev)
{
	myDevices[baseaddr] = dev;
	return true;
}

Device* Bus::getDevice(uint64_t baseaddr)
{
	auto it = myDevices.find(baseaddr);
	if (it != myDevices.end())
		return it->second;
	return nullptr;
}


uint64_t Bus::load(uint64_t addr, uint8_t size) const
{
	for (auto dev : myDevices)
	{
		if (dev.first <= addr && addr < dev.first + dev.second->size())
			return dev.second->load(addr - dev.first, size);
	}

	throw(CpuException(Except::LoadAccessFault));
}

void Bus::store(uint64_t addr, uint8_t size, uint64_t value)
{
	for (auto dev : myDevices)
	{
		if (dev.first <= addr && addr < dev.first + dev.second->size())
		{
			dev.second->store(addr - dev.first, size, value);
			return;
		}
	}

	throw(CpuException(Except::StoreAMOAccessFault));
}
