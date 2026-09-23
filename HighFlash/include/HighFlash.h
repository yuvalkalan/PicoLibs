#pragma once
#include <cstdint>
#include <cstring>
#include "pico/stdlib.h"
#include "hardware/flash.h"
#include "hardware/sync.h"

/**
 * HighFlash library for managing flash memory operations.
 * NOTE: this library is not finished yet
 */

/**  TODO List:
 * 1.   Protect against Out-of-Physical-Flash
 * 2.   Protect against multicore (should turn off all interrupts on all cores)
 */

// Symbol provided by the Pico SDK linker script
extern char __flash_binary_end;

class HighFlash
{
private:
    uint32_t m_offset;
    size_t m_size;

    // Static function to track the global flash offset across all instances
    static uint32_t &get_global_offset();

public:
    // Constructor
    HighFlash(size_t requested_size);
    // Erases this specific buffer's flash memory.
    // MUST be called before writing new data over old data!
    void erase();
    // Writes data to the buffer. Handles 256-byte page padding automatically.
    // internal_offset must be a multiple of FLASH_PAGE_SIZE (256)
    bool write(const uint8_t *data, size_t len, size_t internal_offset = 0);
    // Reads data directly from the memory-mapped flash
    bool read(uint8_t *out_buffer, size_t len, size_t internal_offset = 0);
    // Helper to see the bounds
    uint32_t get_offset() const;
    size_t get_size() const;
};