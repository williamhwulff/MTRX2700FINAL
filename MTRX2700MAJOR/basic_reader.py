# basic_reader.py

import serial
import struct
import time


# Configuration
PORT1       = '/dev/cu.usbmodem11303'  # ← your port
PORT2       = 'COM3'
BAUD        = 115200
SENTINEL    = b'\xAA\x55'
STREAK_TYPE = 1
BEEP_TYPE   = 2
BUTTON_PRESS = 3
MORSE_MESSAGE = 4

def read_packet(ser):
    """Read one framed packet or return None."""
    if ser.read(1) != SENTINEL[0:1]:
        return None
    if ser.read(1) != SENTINEL[1:2]:
        return None
    hdr = ser.read(4)
    if len(hdr) < 4:
        return None
    msg_type, length = struct.unpack('<HH', hdr)
    payload = ser.read(length)
    if len(payload) < length:
        return None
    return msg_type, payload

def main():
    time.sleep(0.5)  # Give STM32 time to finish boot
    print("Listening for packets…")
    with serial.Serial(PORT2, BAUD, timeout=1) as ser:
        while True:
            pkt = read_packet(ser)
            if not pkt:
                continue
            msg_type, payload = pkt

            if msg_type == STREAK_TYPE and len(payload) == 4:
                streak, = struct.unpack('<I', payload)
                print(f"Streak = {streak}")

            elif msg_type == BEEP_TYPE and len(payload) == 4:
                timestamp, = struct.unpack('<I', payload)
                print(f"[BEEP] Timestamp = {timestamp} ms")

            elif msg_type == BUTTON_PRESS and len(payload) == 4:
                timestamp, = struct.unpack('<I', payload)
                print(f"[BUTTON] Timestamp = {timestamp} ms")
            
            elif msg_type == MORSE_MESSAGE and len(payload) >= 2:
                morse_str_bytes = payload[:-1]
                morse_complete = payload[-1]
                try:
                    morse_str = morse_str_bytes.decode('ascii', errors='replace')
                    print(f"{morse_str}")
                except Exception as e:
                    print("Decode error:", e)


                if morse_complete == 1:
                    ser.write(b'\x01')


if __name__ == '__main__':
    main()