#include "beacon_protocol.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    beacon_message_t message = { .sender_id = 1U, .sequence = 42U };
    uint8_t buffer[BEACON_PROTOCOL_MAX_WIRE_LEN] = { 0 };
    size_t length = 0U;
    assert(beacon_protocol_format(&message, buffer, sizeof(buffer), &length) == BEACON_PROTOCOL_OK);
    assert(length == strlen("SIMAI,1,042"));
    assert(memcmp(buffer, "SIMAI,1,042", length) == 0);

    beacon_message_t parsed = { 0 };
    assert(beacon_protocol_parse(buffer, length, &parsed) == BEACON_PROTOCOL_OK);
    assert(parsed.sender_id == 1U);
    assert(parsed.sequence == 42U);
    assert(beacon_protocol_parse((const uint8_t *)"SIMAI,2,001", 11U, &parsed) ==
           BEACON_PROTOCOL_ERR_SENDER);
    puts("PASS beacon_protocol host tests");
    return 0;
}
