#pragma once
#include "hardware/pio.h"

// Loads a PIO program only if it hasn't been loaded on this PIO instance yet.
// Returns the memory offset.
uint pio_add_program_once(PIO pio, const pio_program_t *program);

// Removes the program from PIO memory and clears it from the cache.
// Returns true if it was found and removed, false if it wasn't loaded.
bool pio_remove_program_cached(PIO pio, const pio_program_t *program);