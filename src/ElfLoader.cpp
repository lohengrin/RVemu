#include "ElfLoader.h"
#include "Memory.h"
#include "Defines.h"

#include <elfio/elfio.hpp>
#include <iostream>
#include <iomanip>
#include <cstring>


bool ElfLoader::load(const std::string& file, Memory* mem)
{
    ELFIO::elfio reader;

    if (!reader.load(file))
    {
        std::cout << "Not an ELF file" << std::endl;
        return false;
    }

    if (reader.get_class() != ELFIO::ELFCLASS64)
    {
        std::cerr << "Not an 64 bit ELF" << std::endl;
        return false;
    }
    if (reader.get_encoding() != ELFIO::ELFDATA2LSB)
    {
        std::cerr << "Not an Little Endian ELF" << std::endl;
        return false;
    }

    start = reader.get_entry();

    ELFIO::Elf_Half seg_num = reader.segments.size();
    std::cout << "Number of segments: " << seg_num << std::endl;

    for (int i = 0; i < seg_num; ++i)
    {
        const ELFIO::segment* pseg = reader.segments[i];
        
        if (pseg->get_type() == ELFIO::PT_LOAD)
        {
            uint64_t vaddr = pseg->get_virtual_address();
            uint64_t filesz = pseg->get_file_size();
            uint64_t memsz = pseg->get_memory_size();
            uint64_t offset = pseg->get_offset();
            const char* data = pseg->get_data();

            std::cout << "  PT_LOAD segment [" << i << "] vaddr=0x" << std::hex << vaddr 
                      << " filesz=0x" << filesz << " memsz=0x" << memsz << std::dec << std::endl;

            if (vaddr < DRAM_BASE || filesz == 0)
            {
                if (filesz == 0)
                    std::cout << "  Skipping empty segment" << std::endl;
                else
                    std::cout << "  Segment virtual address 0x" << std::hex << vaddr 
                              << " is below DRAM_BASE 0x" << DRAM_BASE << std::dec << std::endl;
                continue;
            }

            uint64_t dram_offset = vaddr - DRAM_BASE;
            
            if (dram_offset + memsz > mem->size())
            {
                std::cerr << "Segment exceeds memory bounds: offset=0x" << std::hex << dram_offset 
                          << " size=0x" << memsz << " memory_size=0x" << mem->size() << std::dec << std::endl;
                continue;
            }

            if (filesz > 0)
            {
                std::memcpy(&mem->dram[dram_offset], data, filesz);
            }

            if (memsz > filesz)
            {
                std::memset(&mem->dram[dram_offset + filesz], 0, memsz - filesz);
            }
        }
    }

    ELFIO::Elf_Half sec_num = reader.sections.size();
    std::cout << "Number of sections: " << sec_num << std::endl;
    for (int i = 0; i < sec_num; ++i)
    {
        const ELFIO::section* psec = reader.sections[i];
        std::cout << "  [" << i << "] " << psec->get_name() << "\t" << psec->get_size() << std::endl;
        if (psec->get_type() == ELFIO::SHT_SYMTAB)
        {
            const ELFIO::const_symbol_section_accessor symbols(reader, psec);
            for (unsigned int j = 0; j < symbols.get_symbols_num(); ++j)
            {
                std::string   name;
                ELFIO::Elf64_Addr    value;
                ELFIO::Elf_Xword     size;
                unsigned char bind;
                unsigned char type;
                ELFIO::Elf_Half      section_index;
                unsigned char other;

                symbols.get_symbol(j, name, value, size, bind, type, section_index, other);
                if (name == "begin_signature")
                    begin_signature = value;
                else if (name == "end_signature")
                    end_signature = value;
                else if (name == "_start")
                    ram_start = value;
                else if (name == "__reset")
                    ram_start = value;
                else if (name == "__irq_wrapper")
                    mtvec = value;
            }
        }
    }

    return true;
}