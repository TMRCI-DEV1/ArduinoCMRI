#ifndef CMRI_h
#define CMRI_h

#define _CMRI_VERSION 1.5 // version of this library
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/uart.h"

// Define missing constants
#define STX 0x02
#define ETX 0x03
#define ESC 0x10
#define POLL 0x80
#define SET 0x40
#define GET 0x00

// Parsing state constants
enum CMRI_mode
{
    PREAMBLE_1,
    PREAMBLE_2,
    PREAMBLE_3,
    DECODE_ADDR,
    DECODE_CMD,
    IGNORE_CMD,
    DECODE_DATA,
    IGNORE_DATA,
    DECODE_ESC_DATA,
    IGNORE_ESC_DATA,
    POSTAMBLE_OTHER,
    NOOP
};

class CMRI
{
    public:
        CMRI(unsigned int address = 0, unsigned int input_bits = 24, unsigned int output_bits = 48, uart_inst_t* uart_instance = uart0);
        void set_address(unsigned int address);

        bool process();
        bool process_char(char c);
        void transmit();

        bool get_bit(int n);
        char get_byte(int n);

        bool set_bit(int n, bool b);
        bool set_byte(int n, char b);

        const uint8_t* get_output_bytes();
        void set_input_bytes(const uint8_t* data, size_t length);

private:
    // Add missing variables
    unsigned int _address;
    unsigned int _rx_length;
    unsigned int _tx_length;
    char *_rx_buffer;
    char *_tx_buffer;
    uart_inst_t *_uart;
    CMRI_mode _mode;
    unsigned int _rx_index;

    uint8_t _decode(uint8_t c);
};

#endif