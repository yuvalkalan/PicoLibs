#include "HighFlash.h"

uint32_t &HighFlash::get_global_offset()
{
    static uint32_t current_offset = 0;

    // Initialize on first call using the safe binary end marker
    if (current_offset == 0)
    {
        uintptr_t end_address = (uintptr_t)&__flash_binary_end;

        uint32_t base = end_address - XIP_BASE;

        // Round up to the very next 4096-byte sector boundary
        current_offset = (base + FLASH_SECTOR_SIZE - 1) & ~(FLASH_SECTOR_SIZE - 1);
    }
    return current_offset;
}

HighFlash::HighFlash(size_t requested_size)
{
    uint32_t &global_offset = get_global_offset();

    m_offset = global_offset;

    // Adjust buffer size: Round up to the nearest FLASH_SECTOR_SIZE (4096 bytes)
    // If we don't do this, two instances might share a sector,
    // meaning erasing one would destroy the other.
    m_size = (requested_size + FLASH_SECTOR_SIZE - 1) & ~(FLASH_SECTOR_SIZE - 1);

    // Advance the global tracker so the next instance starts after this one
    global_offset += m_size;
}

void HighFlash::erase()
{
    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(m_offset, m_size);
    restore_interrupts(ints);
}

bool HighFlash::write(const uint8_t *data, size_t len, size_t internal_offset)
{
    if (internal_offset + len > m_size)
        return false; // Out of bounds
    if (internal_offset % FLASH_PAGE_SIZE != 0)
        return false; // Bad alignment

    size_t padded_len = (len + FLASH_PAGE_SIZE - 1) & ~(FLASH_PAGE_SIZE - 1);

    // Program page by page (256 bytes at a time)
    for (size_t i = 0; i < padded_len; i += FLASH_PAGE_SIZE)
    {
        // Fill with 0xFF (the default erased state of flash memory)
        uint8_t page_buf[FLASH_PAGE_SIZE];
        memset(page_buf, 0xFF, FLASH_PAGE_SIZE);

        // Copy data into our padded buffer to prevent reading out of bounds
        size_t bytes_to_copy = (len > i) ? (len - i) : 0;
        if (bytes_to_copy > FLASH_PAGE_SIZE)
            bytes_to_copy = FLASH_PAGE_SIZE;

        if (bytes_to_copy > 0)
        {
            memcpy(page_buf, data + i, bytes_to_copy);
        }
        uint32_t ints = save_and_disable_interrupts(); // disable interrupts only for the writing to prevent starvation
        flash_range_program(m_offset + internal_offset + i, page_buf, FLASH_PAGE_SIZE);
        restore_interrupts(ints);
    }

    return true;
}

bool HighFlash::read(uint8_t *out_buffer, size_t len, size_t internal_offset)
{
    if (internal_offset + len > m_size)
        return false; // Out of bounds

    const uint8_t *flash_ptr = (const uint8_t *)(XIP_BASE + m_offset + internal_offset);
    memcpy(out_buffer, flash_ptr, len);

    return true;
}

uint32_t HighFlash::get_offset() const
{
    return m_offset;
}
size_t HighFlash::get_size() const
{
    return m_size;
}