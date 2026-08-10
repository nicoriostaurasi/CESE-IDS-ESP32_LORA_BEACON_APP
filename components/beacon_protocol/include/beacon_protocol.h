#ifndef BEACON_PROTOCOL_H
#define BEACON_PROTOCOL_H

#include <stddef.h>
#include <stdint.h>

#define BEACON_PROTOCOL_MAX_WIRE_LEN 32U

typedef enum {
    BEACON_PROTOCOL_OK = 0,
    BEACON_PROTOCOL_ERR_INVALID_ARG,
    BEACON_PROTOCOL_ERR_FORMAT,
    BEACON_PROTOCOL_ERR_SENDER,
    BEACON_PROTOCOL_ERR_SEQUENCE,
    BEACON_PROTOCOL_ERR_BUFFER_TOO_SMALL,
} beacon_protocol_result_t;

typedef struct {
    uint8_t sender_id;
    uint32_t sequence;
} beacon_message_t;

beacon_protocol_result_t beacon_protocol_parse(
    const uint8_t *data, size_t length, beacon_message_t *message);
beacon_protocol_result_t beacon_protocol_format(
    const beacon_message_t *message, uint8_t *buffer, size_t buffer_size, size_t *written);

#endif
