#ifndef SERIALISE_HEADER
#define SERIALISE_HEADER

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// Constants
#define SENTINEL_1 0xAA
#define SENTINEL_2 0x55

// --- 1) Enumerate your packet types ---
typedef enum {
    STRING_PACKET = 0,
    STREAK_DATA   = 1,
    BEEP_EVENT    = 2,
    BUTTON_PRESS  = 3,
    MORSE_MESSAGE = 4
} MessageType;

// --- 2) Define the payloads you’ll send ---
typedef struct {
    uint8_t length;
    char   *data;
} StringPacket;

typedef struct {
    uint32_t streak;
} StreakData;

typedef struct {
    uint32_t timestamp;   // HAL_GetTick() value
} BeepData;

typedef struct {
    uint32_t timestamp;   // HAL_GetTick() value
} ButtonPressData;

typedef struct {
    uint8_t *morseString;
    uint8_t morseComplete;
} MorseMessageData;

// --- 3) A union of all possible payloads ---
typedef union {
    StringPacket  string_packet;  // for STRING_PACKET
    StreakData    streak_data;    // for STREAK_DATA
    BeepData     beep_data;
    ButtonPressData   button_press_data;
    MorseMessageData  morse_message_data;
} Data;

// --- 4) Packet header (always 6 bytes) ---
typedef struct {
    uint8_t  sentinel1;     // == SENTINEL_1
    uint8_t  sentinel2;     // == SENTINEL_2
    uint16_t message_type;  // see MessageType
    uint16_t data_length;   // length of the payload in bytes
} Header;

// --- 5) Your pack/unpack API ---
uint16_t pack_buffer(uint8_t *buffer,
                     MessageType message_type,
                     Data *data);

bool unpack_buffer(const uint8_t *buffer,
                   Data *output_data,
                   MessageType *output_message_type,
                   uint16_t *output_data_length);

#endif // SERIALISE_HEADER
