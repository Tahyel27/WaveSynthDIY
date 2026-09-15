#pragma once

#include "pico/stdlib.h"
#include "pico/util/queue.h"
#include "hardware/uart.h"
#include "StaticQueue.hpp"
#include "Events.hpp"

#define MIDI_BAUD_RATE 31250

static queue_t MIDI_UART_QUEUE;

void on_uart0_rx()
{
    while (uart_is_readable(uart0))
    {
        char byte = uart_getc(uart0);

        queue_try_add(&MIDI_UART_QUEUE, &byte);
    }
}

void on_uart1_rx()
{
    while (uart_is_readable(uart1))
    {
        char byte = uart_getc(uart1);

        queue_try_add(&MIDI_UART_QUEUE, &byte);
    }
}

enum class MidiState
{
    WAIT,
    STATUS,
    DATA
};

class MidiReceiver 
{
    uart_inst_t * uart = nullptr;
    MidiState midi_state;
    MidiMessage midi_message;

    MidiReceiver() = default;

    bool is_byte_status(char byte)
    {
        return byte & 0x80;
    }

    void cleanup()
    {
        uart_set_irq_enables(uart, false, false);
        int uart_irq = (uart == uart0) ? UART0_IRQ : UART1_IRQ;
        irq_set_enabled(uart_irq, false);
        uart_deinit(uart);
    }
public:
    MidiReceiver(MidiReceiver &&other) 
        : uart(other.uart), midi_state(other.midi_state), midi_message(other.midi_message)
    {
        other.uart == nullptr;
    }
    
    MidiReceiver(const MidiReceiver &other) = delete;
    MidiReceiver& operator=(const MidiReceiver &other) = delete;

    ~MidiReceiver() {
        if (uart != nullptr)
        {
            cleanup();
        }
    }

    static MidiReceiver acquire_uart0(uint rx_pin)
    {
        MidiReceiver receiver{};
        
        queue_init(&MIDI_UART_QUEUE, sizeof(char), 128);

        uart_init(uart0, MIDI_BAUD_RATE);
        receiver.uart = uart0;

        gpio_set_function(rx_pin, GPIO_FUNC_UART);

        int uart_irq = (receiver.uart == uart0) ? UART0_IRQ : UART1_IRQ;
        irq_set_exclusive_handler(uart_irq, on_uart0_rx);
        irq_set_enabled(uart_irq, true);

        uart_set_irq_enables(receiver.uart, true, false);

        return receiver;
    }

    static MidiReceiver acquire_uart1(uint rx_pin)
    {
        MidiReceiver receiver{};

        queue_init(&MIDI_UART_QUEUE, sizeof(char), 128);

        uart_init(uart1, MIDI_BAUD_RATE);
        receiver.uart = uart1;

        gpio_set_function(rx_pin, GPIO_FUNC_UART);

        int uart_irq = (receiver.uart == uart0) ? UART0_IRQ : UART1_IRQ;
        irq_set_exclusive_handler(uart_irq, on_uart1_rx);
        irq_set_enabled(uart_irq, true);

        uart_set_irq_enables(receiver.uart, true, false);

        return receiver;
    }

    template<size_t N>
    void poll(staticQueue<Event, N> &event_queue)
    {
        char byte;
        while(queue_try_remove(&MIDI_UART_QUEUE, &byte))
        {
            if(midi_state == MidiState::WAIT)
            {
                if (is_byte_status(byte)) {
                    midi_state = MidiState::STATUS;
                    midi_message.status = byte;
                }
            } else if (midi_state == MidiState::STATUS)
            {
                if (is_byte_status(byte)) {
                    midi_message.status = byte;
                    //do not send status only messages since i have no use for them
                } else {
                    midi_state = MidiState::DATA;
                    midi_message.data_1 = byte;
                }
            } else
            {
                if (is_byte_status(byte)) {
                    midi_state = MidiState::STATUS;
                    event_queue.push(Event::midi_message(midi_message));
                    midi_message.status = byte;
                } else {
                    midi_state = MidiState::WAIT;
                    midi_message.data_2 = byte;
                    event_queue.push(Event::midi_message(midi_message));
                }
            }
        }
    }
};