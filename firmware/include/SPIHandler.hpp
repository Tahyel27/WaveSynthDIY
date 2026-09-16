#pragma once

#include "pico/stdlib.h"
#include "hardware/spi.h"

// minimal SPI handler
// only implements what I need
// doesnt handle chip select at all
class SPIHandler
{
    spi_inst_t *spi_port = nullptr;

    void free()
    {
        if (spi_port != nullptr)
        {
            spi_deinit(spi_port);
        }
    }

public:
    SPIHandler(spi_inst_t *spi, uint pin_miso, uint pin_mosi, uint pin_sck, uint baudrate)
    {
        spi_port = spi;
        spi_init(spi_port, baudrate);
        gpio_set_function(pin_miso, GPIO_FUNC_SPI);
        gpio_set_function(pin_mosi, GPIO_FUNC_SPI);
        gpio_set_function(pin_sck, GPIO_FUNC_SPI);
    }

    int write_blocking(const uint8_t *source, size_t len)
    {
        return spi_write_blocking(spi_port, source, len);
    }

    ~SPIHandler()
    {
        free();
    }
};