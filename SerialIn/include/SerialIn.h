#pragma once

#include <tusb.h>
#include <string>
#include "pico/bootrom.h"
#include <stdio.h>
#include <hardware/watchdog.h>

#define CTRL_C 3
#define CTRL_D 4
#define CTRL_X 24
#define CTRL_Z 26

class SerialIn
{
private:
    std::string m_message;
    void reset_bootsel();
    void reboot();

public:
    void update();
    SerialIn();
};