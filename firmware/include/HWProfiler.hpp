#pragma once

#include <hardware/gpio.h>

struct HWProfiler
{
    static constexpr uint DEFAULT_PIN = 1;
    static constexpr uint DEFAULT_SECOND_PIN = 2;

    static void init()
    {
        gpio_init(DEFAULT_PIN);
        gpio_set_dir(DEFAULT_PIN, true);

        gpio_init(DEFAULT_SECOND_PIN);
        gpio_set_dir(DEFAULT_SECOND_PIN, true);
    }

    inline static void putHI()
    {
        gpio_put(DEFAULT_PIN, 1);
    }

    inline static void putLO()
    {
        gpio_put(DEFAULT_PIN, 0);
    }

    inline static void putSHI()
    {
        gpio_put(DEFAULT_SECOND_PIN, true);
    }

    inline static void putSLO()
    {
        gpio_put(DEFAULT_SECOND_PIN, false);
    }

};
