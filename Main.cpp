#include "SerialIn.h"
#include "ConnectCC1101.h"
#include "UniqueID.h"
#include "Logger.h"
#include "LedControl.h"

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"

pico_unique_board_id_t CLIENT_ID = {{0xA2, 0x04, 0xC4, 0x42, 0xD4, 0x33, 0x1A, 0x97}};
pico_unique_board_id_t SERVER_ID = {{0xC4, 0xF3, 0xB0, 0x07, 0xB8, 0xAB, 0xC3, 0xE8}};

#define SERVER_ADDRESS 0x01
#define CLIENT_ADDRESS 0x02

ConnectCC1101 *g_radio = nullptr;
LedControl led;

void core1_main()
{
    SerialIn serial_in;
    while (true)
    {
        serial_in.update();
        if (g_radio && g_radio->is_connected())
            g_radio->update();
        led.update();
    }
}

bool is_server()
{
    return check_id(&SERVER_ID);
}

void run_server()
{
    Logger::print(LogLevel::DEBUG, "Running server...\n");
    ConnectCC1101 radio(CC1101_FREQ_434MHZ, 0x5, 0x00, SERVER_ADDRESS);
    g_radio = &radio;
    while (true)
    {
        led.set_blink_speed(LED_BLINK_SLOW);
        while (!radio.is_connected())
        {
            if (radio.accept(5000))
            {
                Logger::print(LogLevel::INFO, "Client connected!\n");
            }
            else
            {
                Logger::print(LogLevel::WEAK_WARNING, "No connection attempts received. Retrying...\n");
            }
        }
        led.set_blink_speed(LED_BLINK_FAST);
        auto time_since_last_msg_ms = to_ms_since_boot(get_absolute_time());
        while (radio.is_connected())
        {
            if (radio.have_data())
            {
                Msg msg;
                if (radio.receive(msg, 5000))
                {

                    // Logger::print(LogLevel::DEBUG, "Received message (%d bytes): %s\n", msg.length, (char *)msg.data);
                    // remove padding from the msg and print it
                    Logger::print(LogLevel::INFO, "Received message: %s\n", (char *)msg.data + msg.length - 5);
                    time_since_last_msg_ms = to_ms_since_boot(get_absolute_time());
                }
                else
                {
                    Logger::print(LogLevel::ERROR, "Timeout or error while receiving message payload.\n");
                    break;
                }
            }
            else if (to_ms_since_boot(get_absolute_time()) - time_since_last_msg_ms > 10000)
                radio.disconnect();
            sleep_ms(1);
        }
    }
}

void run_client()
{
    Logger::print(LogLevel::TRACE, "Running client...\n");
    ConnectCC1101 radio(CC1101_FREQ_434MHZ, 0x5, 0x00, CLIENT_ADDRESS);
    g_radio = &radio;
    while (true)
    {
        int counter = 1;
        led.set_blink_speed(LED_BLINK_SLOW);
        while (!radio.is_connected())
        {
            if (radio.connect(SERVER_ADDRESS, 3000))
            {
                Logger::print(LogLevel::INFO, "connected!\n");
            }
            else
            {
                Logger::print(LogLevel::WEAK_WARNING, "Connection attempt failed. Retrying in 1 second...\n");
                sleep_ms(1000);
            }
        }
        auto start_time = to_ms_since_boot(get_absolute_time());
        led.set_blink_speed(LED_BLINK_FAST);
        while (radio.is_connected())
        {
            // every x second, send a message to the server with the current counter value, padded to 1000 bytes
            if (radio.is_idle())
            // if (to_ms_since_boot(get_absolute_time()) - start_time >= 1000)
            {
                // for (size_t i = 0; i < 5; i++)
                // {
                Msg msg;
                std::string data = std::to_string(counter++);
                // uint string_length = get_rand_32() % 1000;
                data = std::string(1000 - data.length(), '0') + data;
                msg.length = strlen(data.c_str()) + 1;
                memcpy(msg.data, data.c_str(), msg.length);
                Logger::print(LogLevel::INFO, "Sending message %d (%d)...\n", counter - 1, msg.length);
                radio.send(msg);
                start_time = to_ms_since_boot(get_absolute_time());
                // }
            }
            sleep_ms(1);
        }
    }
}

int main()
{

    stdio_init_all();
    wait_for_serial(10000);
    multicore_launch_core1(core1_main);
    Logger::set_level(LogLevel::TRACE);
    Logger::print(LogLevel::DEBUG, "Starting!\n");
    print_id();
    if (is_server())
    {
        run_server();
    }
    else
    {
        run_client();
    }
}
