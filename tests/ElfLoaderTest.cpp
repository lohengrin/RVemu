#include "ElfLoader.h"
#include "Memory.h"
#include "Defines.h"

#include <gtest/gtest.h>
#include <fstream>
#include <cstring>

class ElfLoaderTest : public ::testing::Test {
protected:
    void SetUp() override {
        mem = std::make_unique<Memory>(8192); // Larger memory for testing with data/bss
        eloader = std::make_unique<ElfLoader>();
    }

    std::unique_ptr<Memory> mem;
    std::unique_ptr<ElfLoader> eloader;
};

TEST_F(ElfLoaderTest, InvalidMagic_ReturnsFalse)
{
    bool result = eloader->load("test_elfs/invalid_magic.elf", mem.get());
    EXPECT_FALSE(result);
}

TEST_F(ElfLoaderTest, InvalidMagic_MemoryUnchanged)
{
    mem->store(0, 64, 0xDEADBEEFDEADBEEFULL);
    bool result = eloader->load("test_elfs/invalid_magic.elf", mem.get());
    EXPECT_FALSE(result);
    EXPECT_EQ(mem->load(0, 64), 0xDEADBEEFDEADBEEFULL);
}

TEST_F(ElfLoaderTest, Not64Bit_ReturnsFalse)
{
    bool result = eloader->load("test_elfs/32bit.elf", mem.get());
    EXPECT_FALSE(result);
}

TEST_F(ElfLoaderTest, NotLittleEndian_ReturnsFalse)
{
    bool result = eloader->load("test_elfs/bigendian.elf", mem.get());
    EXPECT_FALSE(result);
}

TEST_F(ElfLoaderTest, Valid64Bit_LoadsSuccessfully)
{
    bool result = eloader->load("test_elfs/simple_code_proper.elf", mem.get());
    EXPECT_TRUE(result);
}

TEST_F(ElfLoaderTest, Valid64Bit_CapturesEntryPoint)
{
    eloader->load("test_elfs/simple_code_proper.elf", mem.get());
    EXPECT_EQ(eloader->start, 0x80000000ULL);
}

TEST_F(ElfLoaderTest, Valid64Bit_LoadsCodeAtCorrectAddress)
{
    eloader->load("test_elfs/simple_code_proper.elf", mem.get());
    
    uint64_t dram_offset = 0x80000000 - DRAM_BASE;
    EXPECT_EQ(dram_offset, 0);
    
    uint8_t first_byte = mem->load(dram_offset, 8);
    EXPECT_EQ(first_byte, 0x13);
}

TEST_F(ElfLoaderTest, Valid64Bit_LoadsDataAtCorrectAddress)
{
    eloader->load("test_elfs/with_data_bss_fixed.elf", mem.get());
    
    uint64_t data_vaddr = 0x80000010;
    uint64_t dram_offset = data_vaddr - DRAM_BASE;
    
    uint64_t data_value = mem->load(dram_offset, 64);
    EXPECT_EQ(data_value, 0xDEADBEEFCAFEBABEULL);
}

TEST_F(ElfLoaderTest, Valid64Bit_BssIsZeroFilled)
{
    eloader->load("test_elfs/with_data_bss_fixed.elf", mem.get());
    
    uint64_t bss_start = 0x80000018;
    uint64_t bss_end = bss_start + 0x1008;
    
    for (uint64_t addr = bss_start; addr < std::min(bss_end, DRAM_BASE + mem->size()); addr += 8) {
        uint64_t dram_offset = addr - DRAM_BASE;
        EXPECT_EQ(mem->load(dram_offset, 64), 0x0ULL) << "BSS not zero-filled at offset 0x" << std::hex << addr;
    }
}

TEST_F(ElfLoaderTest, Valid64Bit_DoesNotCorruptOtherMemory)
{
    mem->store(100, 64, 0xF0F0F0F0F0F0F0F0ULL);
    mem->store(200, 64, 0xAAAAAAAAAAAAAAAAULL);
    
    eloader->load("test_elfs/valid_64bit.elf", mem.get());
    
    EXPECT_EQ(mem->load(100, 64), 0xF0F0F0F0F0F0F0F0ULL);
    EXPECT_EQ(mem->load(200, 64), 0xAAAAAAAAAAAAAAAAULL);
}

TEST_F(ElfLoaderTest, SymbolFields_DefaultToZero)
{
    eloader->load("test_elfs/valid_64bit.elf", mem.get());
    
    EXPECT_EQ(eloader->begin_signature, 0ULL);
    EXPECT_EQ(eloader->end_signature, 0ULL);
    EXPECT_EQ(eloader->ram_start, 0ULL);
    EXPECT_EQ(eloader->mtvec, 0ULL);
}

TEST_F(ElfLoaderTest, NonexistentFile_ReturnsFalse)
{
    bool result = eloader->load("nonexistent_file.elf", mem.get());
    EXPECT_FALSE(result);
}

TEST_F(ElfLoaderTest, EmptyMemory_BeforeLoad)
{
    for (uint64_t addr = 0; addr < std::min(uint64_t(64), mem->size()); addr += 8) {
        EXPECT_EQ(mem->load(addr, 64), 0x0ULL) << "Memory not zero-initialized at offset 0x" << std::hex << addr;
    }
}

TEST_F(ElfLoaderTest, EntryPoint_IsUsedForPC)
{
    eloader->load("test_elfs/simple_code_proper.elf", mem.get());
    
    EXPECT_EQ(eloader->start, 0x80000000ULL);
}

TEST_F(ElfLoaderTest, MultipleLoad_SameFile)
{
    eloader->load("test_elfs/simple_code_proper.elf", mem.get());
    uint64_t first_load_entry = eloader->start;
    
    auto eloader2 = std::make_unique<ElfLoader>();
    eloader2->load("test_elfs/simple_code_proper.elf", mem.get());
    uint64_t second_load_entry = eloader2->start;
    
    EXPECT_EQ(first_load_entry, second_load_entry);
}