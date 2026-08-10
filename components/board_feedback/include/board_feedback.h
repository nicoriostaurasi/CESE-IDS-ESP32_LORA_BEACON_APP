#ifndef BOARD_FEEDBACK_H
#define BOARD_FEEDBACK_H

#include "esp_err.h"

#include <stdint.h>

#ifdef _cplusplus
extern "C" {
#endif

typedef enum {
    BOARD_STATE_BOOT = 0,
    BOARD_STATE_IDLE,
    BOARD_STATE_TX,
    BOARD_STATE_RX,
    BOARD_STATE_FORWARDING,
    BOARD_STATE_ERROR,
    BOARD_STATE_TEST_RUNNING,
} board_state_t;

typedef struct {
    uint8_t node_addr;
    int16_t last_rssi_dbm;
    uint32_t last_rx_number;
    uint32_t last_tx_number;
    uint32_t packet_interval_ms;
} board_status_t;

esp_err_t board_feedback_init(void);
esp_err_t board_feedback_set_state(board_state_t state);
esp_err_t board_feedback_show_status(const board_status_t *status);
esp_err_t board_feedback_show_message(const char *line1, const char *line2, const char *line3);
esp_err_t board_feedback_tick(void);

#ifdef _cplusplus
}
#endif

#endif
