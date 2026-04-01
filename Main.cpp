#include "SerialIn.h"
#include "ConnectCC1101.h"
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
    CC1101 radio(CC1101_FREQ_434MHZ, 0x5, 0x00, SENDER_ADDRESS);
    radio.fstxon_workmode(); // calibrate frequency synthesizer
    int counter = 0;
    uint32_t start_time = to_ms_since_boot(get_absolute_time());
    while (true)
    {
        std::string msg = std::to_string(counter);
        msg = std::string(59 - msg.length(), '0') + msg;
        Packet packet;
        memcpy(packet.payload, msg.c_str(), msg.length() + 1);
        packet.header.length = msg.length() + sizeof(PacketHeader); // payload length + header length
        packet.header.rx_addr = CC1101_BROADCAST_ADDRESS;
        radio.send_packet(packet);
        if (counter % 1000 == 0)
        {
            uint32_t current_time = to_ms_since_boot(get_absolute_time());
            printf("Time: %d ms\n", current_time - start_time);
            start_time = current_time;
        }
        // printf("Sent: %s\r\n", msg.c_str());
        // sleep_ms(10);
        counter++;
    }
}

void run_receiver()
{
    printf("Running receiver...\n");
    CC1101 radio(CC1101_FREQ_434MHZ, 0x5, 0x00, RECEIVER_ADDRESS);
    int expected_msg = 0;
    while (true)
    {
        if (radio.packet_available())
        {
            Packet packet;
            int8_t rssi_dbm;
            uint8_t lqi;
            if (radio.get_payload(packet, rssi_dbm, lqi)) // read package in buffer
            {
                int msg = atoi((char *)packet.payload);
                if (msg != expected_msg)
                {
                    printf("Received: %d rssi: %d lqi: %d ----- %d\r\n", msg, rssi_dbm, lqi, expected_msg);
                    expected_msg = msg;
                }
                expected_msg++;

                // printf("Received: %s rssi: %d lqi: %d\r\n", packet.payload, rssi_dbm, lqi);
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
