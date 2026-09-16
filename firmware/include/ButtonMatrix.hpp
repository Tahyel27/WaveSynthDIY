#pragma once

#include <hardware/spi.h>
#include <pico/stdlib.h>
#include <StaticQueue.hpp>
#include <PioHandler.hpp>
#include <SPIHandler.hpp>
#include <Events.hpp>
#include <array>
#include <optional>

#include "button_array.pio.h"

/*
Button matrix driver:
 - 8x32 max
 - rows are sequentieally turned on by a SIPO shift register adressed through SPI
 - each row is read in parallel using the PIO (Same code as ButtonArray.hpp) 
*/

//to increase limit we would just need to write 4 bytes using SPI, not necessary now 
constexpr int MAX_MATRIX_ROWS = 8; 
constexpr int MAX_MATRIX_COLUMNS = 32;

class ButtonMatrix 
{
    //external shift register controls
    SPIHandler * spi = nullptr; //used to turn on rows
    PioHandler   pio; //used to shift in a whole row

    int rows = 0; int columns = 0;

    std::array<uint32_t, MAX_MATRIX_ROWS> prev_row_states;

    ButtonMatrix() = default;

    bool is_pressed(int button, uint32_t state)
    {
        const uint32_t mask = 0x7FFFFFFF;

        return (state << button | mask) == 0xFFFFFFFF;
    }
public:
    //row set SPI is connected to the shift register that turns on rows
    //row read pins are connected to the the shift register that connects to each column
    static std::optional<ButtonMatrix> claim(SPIHandler * row_set_spi, uint row_read_data,
        uint row_read_clock, uint row_read_latch,
        int rows, int columns) 
    {
        auto matrix = ButtonMatrix{};
        matrix.spi = row_set_spi;
        matrix.rows = rows;
        matrix.columns = columns;

        auto pio_opt = PioHandler::acquire(&button_array_program, button_array_program_get_default_config);
        if (!pio_opt.has_value())
            return std::nullopt;
        matrix.pio = std::move(pio_opt.value());

        matrix.pio.set_in_pins(row_read_data);
        matrix.pio.set_set_pins(row_read_latch);
        matrix.pio.set_sideset_pins(row_read_clock);

        matrix.pio.set_in_shift(false, false, 32);
        matrix.pio.set_clkdiv_int_frac8(3, 1);

        matrix.pio.init();
        matrix.pio.set_enabled(true);

        return matrix;
    }

    ButtonMatrix(ButtonMatrix && other) 
        : spi(other.spi), pio(std::move(other.pio)),
          columns(other.columns), rows(other.rows)
    {
        other.spi = nullptr; //invalidate other
        std::copy(other.prev_row_states.begin(), other.prev_row_states.end(), prev_row_states.begin()); //copy state
    }

    ButtonMatrix(const ButtonMatrix &other) = delete;
    ButtonMatrix& operator=(const ButtonMatrix &other) = delete;

    template <size_t N>
    void poll(staticQueue<Event, N> &event_queue)
    {
        uint8_t row_set_byte = 0b0000'0000;
        //loop over every row
        for (uint r = 0; r < rows; r++)
        {
            //select row
            row_set_byte = 0b0000'0001 << r;
            spi->write_blocking((uint8_t*)&row_set_byte, 1);

            pio.clear_fifos();
            //read row

            //weird double reading hack needed for unknown reasons to me
            uint32_t row_state = pio.get_blocking();
            row_state = pio.get_blocking();

            uint32_t prev_row_state = prev_row_states[r];

            //loop over the columns
            for (uint c = 0; c < columns; c++)
            {
                if (is_pressed(c, row_state)) 
                {
                    if (!is_pressed(c, prev_row_state))
                    {
                        event_queue.push(Event::button_press(c + r*columns));
                    }
                } 
                else
                {
                    if(is_pressed(c, prev_row_state))
                    {
                        event_queue.push(Event::button_release(c + r*columns));
                    }
                }
            }

            prev_row_states[r] = row_state;
        }
    }
};