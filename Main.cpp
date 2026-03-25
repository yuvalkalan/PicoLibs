#include "SerialIn.h"
#include "CC1101.h"
#include "UniqueID.h"

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"

pico_unique_board_id_t RECEIVER_ID = {{0xE6, 0x61, 0x41, 0x04, 0x03, 0x5C, 0xBE, 0x2D}};
pico_unique_board_id_t SENDER_ID = {{0xE6, 0x62, 0x5C, 0x05, 0xE7, 0x55, 0x24, 0x2A}};

#define SENDER_ADDRESS 0x01
#define RECEIVER_ADDRESS 0x02

bool is_sender()
{
    return check_id(&SENDER_ID);
}
void run_sender()
{
    printf("Running sender...\n");
    CC1101 radio(CC1101_FREQ_434MHZ, 0x1, 0x00, SENDER_ADDRESS);
    int counter = 0;
    while (true)
    {
        std::string msg = "hello world! " + std::to_string(counter);
        radio.sent_packet(RECEIVER_ADDRESS, (uint8_t *)msg.c_str(), msg.length() + 1);
        printf("Sent: %s\r\n", msg.c_str());
        sleep_ms(1000);
        counter++;
    }
}

void run_receiver()
{
    printf("Running receiver...\n");
    CC1101 radio(CC1101_FREQ_434MHZ, 0x1, 0x00, RECEIVER_ADDRESS);
    while (true)
    {
        if (radio.packet_available())
        {
            // printf("have data!\r\n");
            uint8_t pktlen, sender, lqi;
            int8_t rssi_dbm;
            if (radio.get_payload(pktlen, sender, rssi_dbm, lqi))
            {
                printf("Received: %s rssi: %d lqi: %d\r\n", radio.rx_buffer + 3, rssi_dbm, lqi);
            }
        }
    }
}

int main()
{
    stdio_init_all();
    wait_for_serial(10000);
    multicore_launch_core1(serial_core);
    if (is_sender())
    {
        run_sender();
    }
    else
    {
        run_receiver();
    }
}
