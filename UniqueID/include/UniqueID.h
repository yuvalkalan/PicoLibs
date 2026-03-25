#pragma once

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/unique_id.h"
void print_id();
bool check_id(pico_unique_board_id_t *id);
