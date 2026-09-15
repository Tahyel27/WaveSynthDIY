#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/util/queue.h"
#include "hardware/uart.h"

// UART defines
// By default the stdout UART is `uart0`, so we will use the second one
#define UART_ID uart1
#define BAUD_RATE 31250

// Use pins 4 and 5 for UART1
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define UART_TX_PIN 4
#define UART_RX_PIN 5

typedef enum MidiState {
    WAIT,
    STATUS,
    DATA
} MidiState;

typedef struct MidiMessage {
    char status;
    char data_1;
    char data_2;
} MidiMessage;

static queue_t queue;

void on_uart_rx()
{
    while(uart_is_readable(UART_ID))
    {
        char byte = uart_getc(UART_ID);

        queue_try_add(&queue, &byte);
    }
}

bool is_byte_status(char byte)
{
    return byte & 0x80;
}

void print_message(MidiMessage msg) {
    printf("Midi message received: %x %x %x\n", msg.status, msg.data_1, msg.data_2);
}

int main()
{
    stdio_init_all();

    queue_init(&queue, sizeof(char), 20);

    // Set up our UART
    uart_init(UART_ID, BAUD_RATE);
    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    int uart_irq = (UART_ID == uart0) ? UART0_IRQ : UART1_IRQ;
    irq_set_exclusive_handler(uart_irq, on_uart_rx);
    irq_set_enabled(uart_irq, true);

    uart_set_irq_enables(UART_ID, true, false);
    // Use some the various UART functions to send out data
    // In a default system, printf will also output via the default UART
    
    // Send out a string, with CR/LF conversions
    //uart_puts(UART_ID, " Hello, UART!\n");
    
    // For more examples of UART use see https://github.com/raspberrypi/pico-examples/tree/master/uart

    MidiState midi_state = WAIT;
    MidiMessage message;

    while (true) {
        char byte;
        if (queue_try_remove(&queue, &byte)) 
        {
            //printf("Byte received: %x\n", byte);
            if (midi_state == WAIT) {
                if (is_byte_status(byte)) {
                    midi_state = STATUS;
                    message.status = byte;
                }
            } else if (midi_state == STATUS) {
                if (!is_byte_status(byte)) {
                    midi_state = DATA;
                    message.data_1 = byte;
                } else {
                    message.status = byte;
                    //doesnt send status only messages, i have no use for those now
                }
            } else { //state == DATA
                if (!is_byte_status(byte)) { //we do not know what the next status will be -> we need to go to wait
                    midi_state = WAIT;
                    message.data_2 = byte;
                    print_message(message);
                } else {
                    midi_state = STATUS;
                    print_message(message);
                    message.status = byte;
                }
            }
        }
        sleep_ms(5);
    }
}
