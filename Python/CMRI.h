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
        pass  # Implement this method

    def process_char(self, c):
        pass  # Implement this method

    def transmit(self):
        pass  # Implement this method

    def get_bit(self, n):
        pass  # Implement this method

    def get_byte(self, n):
        pass  # Implement this method

    def set_bit(self, n, b):
        pass  # Implement this method

    def set_byte(self, n, b):
        pass  # Implement this method

    def _decode(self, c):
        pass  # Implement this method
