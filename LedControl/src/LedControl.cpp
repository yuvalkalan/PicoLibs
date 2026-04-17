#include "LedControl.h"

auto_init_mutex(led_mutex);

LedControl::LedControl() : blink_speed_ms(LED_BLINK_SLOW), is_on(false), last_blink_time(get_absolute_time())
{

    // init the on-board LED
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_put(PICO_DEFAULT_LED_PIN, is_on);
}

void LedControl::update()
{
    mutex_enter_blocking(&led_mutex);

    if (get_absolute_time() - last_blink_time >= blink_speed_ms * 1000)
    {
        toggle();
        last_blink_time = get_absolute_time();
    }
    mutex_exit(&led_mutex);
}

void LedControl::toggle()
{
    is_on = !is_on;
    gpio_put(PICO_DEFAULT_LED_PIN, is_on);
}

void LedControl::set_blink_speed(uint32_t speed_ms)
{
    mutex_enter_blocking(&led_mutex);
    blink_speed_ms = speed_ms;
    mutex_exit(&led_mutex);
}