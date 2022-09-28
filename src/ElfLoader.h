#pragma once

#include <inttypes.h>
#include <string>
#include <vector>

class Memory;

class ElfLoader
{
public:
	ElfLoader() {}
	virtual ~ElfLoader() {}

	bool load(const std::string& file, Memory * mem);


	/* used when called from the compliance tests */
	uint64_t begin_signature = 0;
	uint64_t end_signature = 0;
	/* program entry point */
	uint64_t start = 0;
	uint64_t mtvec = 0;
	uint64_t ram_start = 0;

	std::vector<uint8_t> program;
	uint64_t program_address = 0;
};