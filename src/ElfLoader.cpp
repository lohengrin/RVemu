#include "ElfLoader.h"
#include "Memory.h"


#include <elfio/elfio.hpp>
#include <iostream>
#include <iomanip>


// Load program as ELF
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

    //Read ELF file sections info 
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
                /* for compliance test */
                else if (name == "_start")
                    ram_start = value;
                /* for zephyr */
                else if (name == "__reset")
                    ram_start = value;
                else if (name == "__irq_wrapper")
                    mtvec = value;
            }
        }
    }

    for (int i = 0; i < sec_num; ++i)
    {
        const ELFIO::section* psec = reader.sections[i];
        std::cout << "  [" << i << "] " << psec->get_name() << "\t" << psec->get_size() << std::endl;

        if (psec->get_type() == ELFIO::SHT_PROGBITS)
        {
            //memcpy(&mem->dram[0] + psec->get_address() - ram_start, psec->get_data(), psec->get_size());
            for (size_t i = 0; i < psec->get_size(); i++) {
                mem->dram[psec->get_address() + i] = ((uint8_t*)psec->get_data())[i];
            }
        }
        else
        {
            std::cout << "ignoring section " << psec->get_name() << " at address 0x" << std::hex << psec->get_address() << std::endl;
        }
    }

    return true;
}