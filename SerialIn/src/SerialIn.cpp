#include "SerialIn.h"

SerialIn::SerialIn() : m_message("")
{
}
void SerialIn::reset_bootsel()
{
    reset_usb_boot(0, 0);
}
void SerialIn::reboot()
{
    watchdog_enable(0, 0);
    while (true)
        tight_loop_contents();
}
void SerialIn::update()
{
    // Check to see if anything is available in the serial receive buffer
    tud_task();
    while (tud_cdc_available())
    {
        char chr = (char)tud_cdc_read_char();
        if (chr == CTRL_C)
            reset_bootsel();
        else if (chr == CTRL_D)
            reboot();
        // printf("char is %c (value=%d)\n", chr, chr);
        m_message += chr;
    }
    if (m_message.length() != 0)
    {
        // process_command(settings);
        m_message = "";
    }
}

void serial_core()
{
    SerialIn serial_in;
    while (true)
    {
        serial_in.update();
    }
}

void wait_for_serial(int timeout_ms)
{
    absolute_time_t timeout = make_timeout_time_ms(timeout_ms);
    while (!tud_cdc_connected())
    {
        if (time_reached(timeout))
            return;
        sleep_ms(100);
    }
}
