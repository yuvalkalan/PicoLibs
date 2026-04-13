#include "SerialIn.h"
#include "ConnectCC1101.h"
#include "UniqueID.h"
#include "Logger.h"

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"

pico_unique_board_id_t CLIENT_ID = {{0xE6, 0x61, 0x41, 0x04, 0x03, 0x5C, 0xBE, 0x2D}};
pico_unique_board_id_t SERVER_ID = {{0xE6, 0x62, 0x5C, 0x05, 0xE7, 0x55, 0x24, 0x2A}};

#define SERVER_ADDRESS 0x01
#define CLIENT_ADDRESS 0x02

bool is_server()
{
    return check_id(&SERVER_ID);
}

void run_server()
{
    Logger::print(LogLevel::DEBUG, "Running server...\n");
    ConnectCC1101 radio(CC1101_FREQ_434MHZ, 0x5, 0x00, SERVER_ADDRESS);
    while (!radio.is_connected())
    {
        if (radio.accept(5000))
        {
            Logger::print(LogLevel::DEBUG, "Client connected!\n");
        }
        else
        {
            Logger::print(LogLevel::WARNING, "No connection attempts received. Retrying...\n");
        }
    }
    while (true)
    {
        if (radio.have_data())
        {
            Msg msg;
            if (radio.receive(msg, 5000))
            {

                // Logger::print(LogLevel::DEBUG, "Received message (%d bytes): %s\n", msg.length, (char *)msg.data);

                // remove padding from the msg and print it
                // std::string received_str((char *)msg.data);
                Logger::print(LogLevel::INFO, "Received message: %s\n", (char *)msg.data + msg.length - 5);
            }
            else
            {
                Logger::print(LogLevel::ERROR, "Timeout or error while receiving message payload.\n");
                break;
            }
        }
        if (!radio.update())
            break;
    }
    while (true)
        ;
}

void run_client()
{
    Logger::print(LogLevel::TRACE, "Running client...\n");
    ConnectCC1101 radio(CC1101_FREQ_434MHZ, 0x5, 0x00, CLIENT_ADDRESS);
    int counter = 1;
    while (!radio.is_connected())
    {
        if (radio.connect(SERVER_ADDRESS, 3000))
        {
            Logger::print(LogLevel::INFO, "connected!\n");
        }
        else
        {
            Logger::print(LogLevel::WARNING, "Connection attempt failed. Retrying in 1 second...\n");
            sleep_ms(1000);
        }
    }
    auto start_time = to_ms_since_boot(get_absolute_time());
    while (true)
    {
        // every x second, send a message to the server with the current counter value, padded to 1000 bytes
        if (radio.is_idle())
        // if (to_ms_since_boot(get_absolute_time()) - start_time >= 1000)
        {
            Msg msg;
            std::string data = std::to_string(counter++);
            // uint string_length = get_rand_32() % 1000;
            data = std::string(1000 - data.length(), '0') + data;
            msg.length = strlen(data.c_str()) + 1;
            memcpy(msg.data, data.c_str(), msg.length);
            Logger::print(LogLevel::INFO, "Sending message %d (%d)...\n", counter - 1, msg.length);
            radio.send(msg);
            start_time = to_ms_since_boot(get_absolute_time());
        }
        if (!radio.update())
            break;
    }
    while (true)
        ;
}

int main()
{
    stdio_init_all();
    wait_for_serial(10000);
    multicore_launch_core1(serial_core);
    Logger::set_level(LogLevel::TRACE);
    if (is_server())
    {
        run_server();
    }
    else
    {
        run_client();
    }
}
