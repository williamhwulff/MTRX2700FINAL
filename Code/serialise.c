#include "serialise.h"
#include <string.h>

// Function to pack data into a buffer for transmission
uint16_t pack_buffer(uint8_t *buffer, MessageType message_type, Data *data) {
    Header header = {
        .sentinel1    = SENTINEL_1,
        .sentinel2    = SENTINEL_2,
        .message_type = message_type,
        .data_length  = 0
    };
    uint16_t buffer_idx  = 0;
    uint16_t data_length = 0;

    // Determine payload size based on message type
    switch (message_type) {
        case STRING_PACKET:
            data_length = data->string_packet.length;
            break;
        case STREAK_DATA:
            data_length = sizeof(data->streak_data);
            break;
        case BEEP_EVENT:
            data_length = sizeof(data->beep_data);
            break;
        case BUTTON_PRESS:
            data_length = sizeof(data->button_press_data);
            break;
        case MORSE_MESSAGE:
            data_length = sizeof(data->morse_message_data);
            break;
        default:
            return 0;  // unknown message type
    }
    header.data_length = data_length;

    // 1) Copy header
    memcpy(buffer + buffer_idx, &header, sizeof(Header));
    buffer_idx += sizeof(Header);

    // 2) Copy payload
    if (message_type == STRING_PACKET) {
        memcpy(buffer + buffer_idx,
               data->string_packet.data,
               data_length);
    } else if (message_type == STREAK_DATA) {
        memcpy(buffer + buffer_idx,
               &data->streak_data,
               data_length);
    } else if (message_type == BEEP_EVENT) {
        memcpy(buffer + buffer_idx,
               &data->beep_data,
               data_length);
    } else if (message_type == BUTTON_PRESS) {
        memcpy(buffer + buffer_idx,
               &data->button_press_data,
               data_length);
    } else if (message_type == MORSE_MESSAGE) {
        memcpy(buffer + buffer_idx,
               &data->morse_message_data,
               data_length);
    }

    buffer_idx += data_length;
    return buffer_idx;
}


// Function to unpack the buffer and check for sentinel bytes
bool unpack_buffer(const uint8_t *buffer,
                   Data        *output_data,
                   MessageType *output_message_type,
                   uint16_t    *output_data_length)
{
    Header header;
    // 1) Copy header
    memcpy(&header, buffer, sizeof(Header));

    // 2) Validate sentinel bytes
    if (header.sentinel1 != SENTINEL_1 || header.sentinel2 != SENTINEL_2) {
        return false;
    }

    *output_message_type = (MessageType)header.message_type;
    *output_data_length  = header.data_length;

    const uint8_t *payload = buffer + sizeof(Header);

    // 3) Extract payload based on message type
    switch (*output_message_type) {
        case STRING_PACKET:
            output_data->string_packet.length = header.data_length;
            output_data->string_packet.data   = (char *)payload;
            break;

        case STREAK_DATA:
            memcpy(&output_data->streak_data,
                   payload,
                   sizeof(output_data->streak_data));
            break;

        case BEEP_EVENT:
            memcpy(&output_data->beep_data,
                   payload,
                   sizeof(output_data->beep_data));
            break;

        case BUTTON_PRESS:
            memcpy(&output_data->button_press_data,
                   payload,
                   sizeof(output_data->button_press_data));
            break;
        
        case MORSE_MESSAGE:
            memcpy(&output_data->morse_message_data,
                   payload,
                   sizeof(output_data->morse_message_data));

        default:
            return false;
    }

    return true;
}
