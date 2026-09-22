#include "HighPIO.h"
#include "pico/stdlib.h" // For panic()

// A PIO block only has 32 instruction slots in total.
// It is physically impossible to load more than ~16 unique programs anyway,
// so a small fixed-size array is perfectly safe and avoids dynamic memory (malloc).
#define MAX_CACHED_PROGRAMS 16

typedef struct
{
    PIO pio;
    const pio_program_t *program;
    uint offset;
} pio_cache_entry_t;

static pio_cache_entry_t program_cache[MAX_CACHED_PROGRAMS];
static uint8_t cache_count = 0;

uint pio_add_program_once(PIO pio, const pio_program_t *program)
{
    // 1. Search the cache to see if this program is already on this PIO block
    for (uint8_t i = 0; i < cache_count; i++)
    {
        if (program_cache[i].pio == pio && program_cache[i].program == program)
        {
            return program_cache[i].offset; // Found it! Return the existing offset
        }
    }

    // 2. Not found. Ensure we haven't exceeded our cache size limit
    if (cache_count >= MAX_CACHED_PROGRAMS)
    {
        panic("PIO program cache full! Increase MAX_CACHED_PROGRAMS.");
    }

    // 3. Load the program into the PIO
    uint offset = pio_add_program(pio, program);

    // 4. Save the details to the cache for next time
    program_cache[cache_count].pio = pio;
    program_cache[cache_count].program = program;
    program_cache[cache_count].offset = offset;
    cache_count++;

    return offset;
}

bool pio_remove_program_cached(PIO pio, const pio_program_t *program)
{
    // 1. Search the cache for the specific program on the specific PIO
    for (uint8_t i = 0; i < cache_count; i++)
    {
        if (program_cache[i].pio == pio && program_cache[i].program == program)
        {

            // 2. Actually remove it from the hardware PIO instruction memory.
            // The SDK requires the original program pointer (to know how many
            // instructions to free) and the offset where it was loaded.
            pio_remove_program(pio, program, program_cache[i].offset);

            // 3. Remove it from our cache array.
            // "Swap-and-Pop" technique: To avoid wasting CPU cycles shifting
            // all subsequent array elements down by 1, we just take the very
            // last item in the array and overwrite the one we want to delete.
            cache_count--;
            if (i < cache_count)
            {
                program_cache[i] = program_cache[cache_count];
            }

            return true; // Successfully removed
        }
    }

    return false; // Not found in the cache
}