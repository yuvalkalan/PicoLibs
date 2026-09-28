#include "ESC.h"

#define MOTOR_1_PIN 26
#define MOTOR_2_PIN 27
#define MOTOR_3_PIN 28
#define MOTOR_4_PIN 29

#define MAX_POWER 100
int main()
{
    stdio_init_all();
    ESC::MotorsConfig esc_config = {
        MOTOR_1_PIN,
        MOTOR_2_PIN,
        MOTOR_3_PIN,
        MOTOR_4_PIN,
        pio0};
    ESC esc(esc_config);
    esc.init();
    ESC::ESCConfig config = {0, 0, 0, 0};
    while (true)
    {
        sleep_ms(5000);
        for (size_t i = DShot600::MIN_THROTTLE; i < MAX_POWER; i++)
        {
            config.throttle = i;
            esc.update(config);
            sleep_ms(10);
        }
        sleep_ms(5000);

        for (size_t i = MAX_POWER; i >= DShot600::MIN_THROTTLE; i--)
        {
            config.throttle = i;
            esc.update(config);
            sleep_ms(10);
        }
    }
    return 0;
}