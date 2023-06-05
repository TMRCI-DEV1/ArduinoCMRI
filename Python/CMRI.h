import serial

# Define missing constants
MAX = 258  # max packet length in bytes (64 i/o cards @ 32 bits each + packet type and address bytes)
INIT = 'I'  # PC is telling us stuff we don't really care about
SET = 'T'  # as in TX from the PC => Arduino, PC is SETing our status
GET = 'R'  # as in TX from Arduino => PC, PC is GETing our status
POLL = 'P'  # PC wants to know our status
NOOP = 0x00  # do nothing
STX = 0x02  # start byte
ETX = 0x03  # end byte
ESC = 0x10  # escape byte

# Parsing state constants
PREAMBLE_1, PREAMBLE_2, PREAMBLE_3, DECODE_ADDR, DECODE_CMD, DECODE_DATA, DECODE_ESC_DATA, IGNORE_CMD, IGNORE_DATA, IGNORE_ESC_DATA, POSTAMBLE_SET, POSTAMBLE_POLL, POSTAMBLE_OTHER = range(13)

class CMRI:
    def __init__(self, address=0, input_bits=24, output_bits=48, serial_class=serial.Serial()):
        self._address = address
        self._rx_length = (output_bits + 7) // 8
        self._tx_length = (input_bits + 7) // 8
        self._rx_packet_type = None
        self._rx_buffer = [0] * self._rx_length
        self._tx_buffer = [0] * self._tx_length
        self._serial = serial_class
        self._mode = PREAMBLE_1
        self._rx_index = 0

    def set_address(self, address):
        self._address = address

    def process(self):
        while self._serial.in_waiting > 0:
            if self.process_char(self._serial.read()):
                return True
        return False

    def process_char(self, c):
        switcher = {
            POLL: self.transmit,
            SET: lambda: True
        }
        return switcher.get(self._decode(c), lambda: False)()

    def get_bit(self, pos):
        c = self.get_byte(pos // 8)
        return (bool) ((c >> (pos % 8)) & 0x01)

    def get_byte(self, pos):
        if pos > self._rx_length:
            return 0  # out of bounds
        else:
            return self._rx_buffer[pos]

    def set_bit(self, pos, bit):
        if ((pos + 7) // 8 > self._tx_length):
            return False  # out of bounds
        else:
            index = pos // 8
            self._tx_buffer[index] = bit if bit else self._tx_buffer[index] & ~(1 << pos % 8)
            return True

    def set_byte(self, pos, b):
        if pos > self._tx_length:
            return False  # out of bounds
        else:
            self._tx_buffer[pos] = b
            return True

        def transmit(self):
        self._serial.write(bytes([255, 255, STX, 65 + self._address, GET]))
        for i in range(self._tx_length):
            if self._tx_buffer[i] == ETX or self._tx_buffer[i] == ESC:
                self._serial.write(bytes([ESC]))  # escape because this looks like an STX bit or an escape bit (very basic protocol)
            self._serial.write(bytes([self._tx_buffer[i]]))
        self._serial.write(bytes([ETX]))

    def _decode(self, c):
        switcher = {
            PREAMBLE_1: self._preamble_1,
            PREAMBLE_2: self._preamble_2,
            PREAMBLE_3: self._preamble_3,
            DECODE_ADDR: self._decode_addr,
            DECODE_CMD: self._decode_cmd,
            IGNORE_CMD: self._ignore_cmd,
            DECODE_DATA: self._decode_data,
            IGNORE_DATA: self._ignore_data,
            DECODE_ESC_DATA: self._decode_esc_data,
            IGNORE_ESC_DATA: self._ignore_esc_data,
            POSTAMBLE_OTHER: self._postamble_other,
            NOOP: self._noop
        }
        return switcher.get(self._mode, self._noop)(c)

    def _preamble_1(self, c):
        self._rx_index = 0
        if c == 0xFF:
            self._mode = PREAMBLE_2

    def _preamble_2(self, c):
        if c == 0xFF:
            self._mode = PREAMBLE_3
        else:
            self._mode = PREAMBLE_1

    def _preamble_3(self, c):
        if c == STX:
            self._mode = DECODE_ADDR
        else:
            self._mode = PREAMBLE_1

    def _decode_addr(self, c):
        if c == 'A' + self._address:
            self._mode = DECODE_CMD
        elif c >= 'A':
            self._mode = IGNORE_CMD
        else:
            self._mode = PREAMBLE_1

    def _decode_cmd(self, c):
        if c == SET:
            self._mode = DECODE_DATA
        elif c == POLL:
            self._mode = POSTAMBLE_POLL
        else:
            self._mode = POSTAMBLE_OTHER

    def _ignore_cmd(self, c):
        self._mode = IGNORE_DATA

    def _decode_data(self, c):
        if c == ESC:
            self._mode = DECODE_ESC_DATA
        elif c == ETX:
            self._mode = POSTAMBLE_SET
        elif self._rx_index >= self._rx_length:
            pass
        else:
            self._rx_buffer[self._rx_index] = c
            self._rx_index += 1

    def _decode_esc_data(self, c):
        if self._rx_index >= self._rx_length:
            pass
        else:
            self._rx_buffer[self._rx_index] = c
            self._rx_index += 1
        self._mode = DECODE_DATA

    def _ignore_data(self, c):
        if c == ESC:
            self._mode = IGNORE_ESC_DATA
        elif c == ETX:
            self._mode = POSTAMBLE_IGNORE

    def _ignore_esc_data(self, c):
        self._mode = IGNORE_DATA

    def _postamble_other(self, c):
        self._mode = PREAMBLE_1

    def _noop(self, c):
        pass
