#include "beacon_protocol.h"

#include <stdio.h>
#include <string.h>

static const uint8_t prefix[] = { 'S', 'I', 'M', 'A', 'I', ',' };

beacon_protocol_result_t beacon_protocol_parse(
    const uint8_t *data, size_t length, beacon_message_t *message)
{
    if (data == NULL || message == NULL) {
        return BEACON_PROTOCOL_ERR_INVALID_ARG;
    }
    if (length < sizeof(prefix) + 3U || memcmp(data, prefix, sizeof(prefix)) != 0) {
        return BEACON_PROTOCOL_ERR_FORMAT;
    }

    size_t index = sizeof(prefix);
    if ((data[index] != '0' && data[index] != '1') || data[index + 1U] != ',') {
        return BEACON_PROTOCOL_ERR_SENDER;
    }
    message->sender_id = (uint8_t)(data[index] - '0');
    index += 2U;

    uint32_t sequence = 0U;
    size_t digits = 0U;
    while (index < length) {
        uint8_t character = data[index++];
        if (character < '0' || character > '9') {
            return BEACON_PROTOCOL_ERR_SEQUENCE;
        }
        uint32_t digit = (uint32_t)(character - '0');
        if (sequence > (UINT32_MAX - digit) / 10U) {
            return BEACON_PROTOCOL_ERR_SEQUENCE;
        }
        sequence = sequence * 10U + digit;
        ++digits;
    }
    if (digits == 0U) {
        return BEACON_PROTOCOL_ERR_SEQUENCE;
    }
    message->sequence = sequence;
    return BEACON_PROTOCOL_OK;
}

beacon_protocol_result_t beacon_protocol_format(
    const beacon_message_t *message, uint8_t *buffer, size_t buffer_size, size_t *written)
{
    if (message == NULL || buffer == NULL || written == NULL || buffer_size == 0U) {
        return BEACON_PROTOCOL_ERR_INVALID_ARG;
    }
    if (message->sender_id > 1U) {
        return BEACON_PROTOCOL_ERR_SENDER;
    }
    int count = snprintf((char *)buffer, buffer_size, "SIMAI,%u,%03lu",
                         message->sender_id, (unsigned long)message->sequence);
    if (count < 0 || (size_t)count >= buffer_size) {
        return BEACON_PROTOCOL_ERR_BUFFER_TOO_SMALL;
    }
    *written = (size_t)count;
    return BEACON_PROTOCOL_OK;
}
