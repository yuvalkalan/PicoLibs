#include "UniqueID.h"

void print_id()
{
    const size_t id_len = 2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1;
    char id_str[id_len];

    // The SDK automatically fetches the ID and formats it as a hex string
    pico_get_unique_board_id_string(id_str, id_len);

    Logger::print(LogLevel::INFO, "Unique ID: 0x%s\n", id_str);
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
