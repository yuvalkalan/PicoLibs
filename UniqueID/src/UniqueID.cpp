#include "UniqueID.h"

void print_id()
{
    pico_unique_board_id_t board_id;
    pico_get_unique_board_id(&board_id);
    printf("Unique ID: ");
    for (const unsigned char i : board_id.id)
    {
        printf("%02X", i);
    }
    printf("\n");
}

bool check_id(pico_unique_board_id_t *id)
{
    pico_unique_board_id_t board_id;
    pico_get_unique_board_id(&board_id);
    for (int i = 0; i < PICO_UNIQUE_BOARD_ID_SIZE_BYTES; i++)
    {
        if (board_id.id[i] != id->id[i])
        {
            return false;
        }
    }
    return true;
}
