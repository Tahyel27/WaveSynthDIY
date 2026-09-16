#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   17
#define PIN_SCK  3
#define PIN_MOSI 2

//minimal SPI handler
//only implements what I need
//doesnt handle chip select at all
class SPIHandler
{
    spi_inst_t * spi_port = nullptr;

    void free()
    {
        if (spi_port != nullptr)
        {
            spi_deinit(spi_port);
        }
    }
public:
    SPIHandler(spi_inst_t * spi, uint pin_miso, uint pin_mosi, uint pin_sck, uint baudrate) 
    {
        spi_port = spi;
        spi_init(spi_port, baudrate);
        gpio_set_function(pin_miso, GPIO_FUNC_SPI);
        gpio_set_function(pin_mosi, GPIO_FUNC_SPI);
        gpio_set_function(pin_sck, GPIO_FUNC_SPI);
    }

    int write_blocking(const uint8_t * source, size_t len)
    {
        return spi_write_blocking(spi_port, source, len);
    }

    ~SPIHandler()
    {
        free();
    }
};

int main()
{
    stdio_init_all();

    // SPI initialisation. This example will use SPI at 1MHz.
    spi_init(SPI_PORT, 1000*1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS,   GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    
    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);
    // For more examples of SPI use see https://github.com/raspberrypi/pico-examples/tree/master/spi

    sleep_ms(2000);
    printf("starting test\n");

    uint8_t byte = 0b0000'0001;
    spi_write_blocking(SPI_PORT, &byte, 1);

    sleep_ms(1000);
    byte = byte << 1;
    spi_write_blocking(SPI_PORT, &byte, 1);

    sleep_ms(1000);
    byte = byte << 1;
    spi_write_blocking(SPI_PORT, &byte, 1);

    sleep_ms(1000);
    byte = byte << 1;
    spi_write_blocking(SPI_PORT, &byte, 1);

    while (true) {
        printf("Hello, world!\n");
        sleep_ms(1000);
    }
}
