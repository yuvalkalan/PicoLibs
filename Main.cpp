#include "SerialIn.h"
#include "ConnectCC1101.h"
#include "UniqueID.h"

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
    printf("Running server...\n");
    ConnectCC1101 radio(CC1101_FREQ_434MHZ, 0x5, 0x00, SERVER_ADDRESS);
    // radio.fstxon_workmode(); // calibrate frequency synthesizer
    int counter = 0;
    uint32_t start_time = to_ms_since_boot(get_absolute_time());

    // while (true)
    // {
    //     std::string msg = std::to_string(counter);
    //     msg = std::string(59 - msg.length(), '0') + msg;
    //     Packet packet;
    //     memcpy(packet.payload, msg.c_str(), msg.length() + 1);
    //     packet.header.length = msg.length() + sizeof(PacketHeader); // payload length + header length
    //     packet.header.rx_addr = CC1101_BROADCAST_ADDRESS;
    //     radio.send_packet(packet);

    //     if (counter % 1000 == 0)
    //     {
    //         uint32_t current_time = to_ms_since_boot(get_absolute_time());
    //         printf("Time: %d ms\n", current_time - start_time);
    //         start_time = current_time;
    //     }
    //     // printf("Sent: %s\r\n", msg.c_str());
    //     // sleep_ms(10);
    //     counter++;
    // }

    while (true)
    {
        printf("Waiting for connection (accepting)...\n");

        // המתנה לחיבור נכנס, פסק זמן של 5 שניות לכל סיבוב המאפשר ללולאה להמשיך לרוץ
        if (radio.accept(5000))
        {
            printf("Client connected! Waiting to receive message...\n");

            Msg msg;
            // המתנה לקבלת ההודעה לאחר שהחיבור בוסס
            if (radio.receive(msg, 3000))
            {
                // הדפסת הנתונים שהתקבלו (הנחה שמדובר במחרוזת תווים)
                printf("Received message (%d bytes): %s\n", msg.length, (char *)msg.data);
            }
            else
            {
                printf("Timeout or error while receiving message payload.\n");
            }
            // update radio for 2000 ms to process any remaining packets or send pending acks
            uint32_t start_time = to_ms_since_boot(get_absolute_time());
            while (to_ms_since_boot(get_absolute_time()) - start_time < 2000)
            {
                radio.update();
            }
        }
        else
        {
            // פסק הזמן של ה-accept פג, פשוט נחזור לתחילת הלולאה וננסה שוב
        }
    }
}

void run_client()
{
    printf("Running client...\n");
    ConnectCC1101 radio(CC1101_FREQ_434MHZ, 0x5, 0x00, CLIENT_ADDRESS);
    int counter = 0;
    while (true)
    {
        printf("Attempting to connect to server 0x%02X...\n", SERVER_ADDRESS);

        // ניסיון התחברות עם פסק זמן של 3 שניות
        if (radio.connect(SERVER_ADDRESS, 3000))
        {
            printf("Connected successfully! Sending message...\n");

            // הכנת ההודעה
            Msg msg;
            std::string data = std::to_string(counter++);
            data = std::string(1000 - data.length(), '0') + data;
            // const char text_payload[] = "Hello Server"; //, this is a reliable message from the Client! This message is longer than 61 bytes to test fragmentation and reassembly of packets in the ConnectCC1101 class.";

            // אורך ה-payload בלבד (הפונקציה send כבר מוסיפה את גודל שדה ה-length)
            msg.length = strlen(data.c_str()) + 1; // +1 כדי לכלול את תו הסיום '\0'
            memcpy(msg.data, data.c_str(), msg.length);

            // שליחת ההודעה (הפונקציה תחלק לפקטות ותדאג לאמינות)
            radio.send(msg);

            printf("Message sent. Waiting before next attempt...\n");
            // update radio for 5000 ms
            uint32_t start_time = to_ms_since_boot(get_absolute_time());
            while (to_ms_since_boot(get_absolute_time()) - start_time < 5000)
            {
                radio.update();
            }
        }
        else
        {
            printf("Connection failed. Retrying in 2 seconds...\n");
            sleep_ms(2000);
        }
    }
}

int main()
{
    stdio_init_all();
    wait_for_serial(10000);
    multicore_launch_core1(serial_core);
    if (is_server())
    {
        run_server();
    }
    else
    {
        run_client();
    }
}
