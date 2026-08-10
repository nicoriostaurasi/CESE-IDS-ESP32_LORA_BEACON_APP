#ifndef BEACON_METRICS_H
#define BEACON_METRICS_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    bool initialized;
    uint32_t received;
    uint32_t missing;
    uint32_t duplicates_or_out_of_order;
    uint32_t last_sequence;
    int16_t rssi_min_dbm;
    int16_t rssi_max_dbm;
    int64_t rssi_sum_dbm;
    int8_t snr_min_db;
    int8_t snr_max_db;
    int64_t snr_sum_db;
    uint32_t interval_samples;
    uint64_t interval_sum_ms;
    uint32_t interval_min_ms;
    uint32_t interval_max_ms;
} beacon_metrics_t;

typedef struct {
    uint32_t received;
    uint32_t missing;
    uint32_t duplicates_or_out_of_order;
    uint32_t delivery_rate_permille;
    int16_t rssi_mean_dbm;
    int16_t rssi_min_dbm;
    int16_t rssi_max_dbm;
    int8_t snr_mean_db;
    int8_t snr_min_db;
    int8_t snr_max_db;
    uint32_t interval_mean_ms;
    uint32_t interval_min_ms;
    uint32_t interval_max_ms;
} beacon_metrics_report_t;

void beacon_metrics_reset(beacon_metrics_t *metrics);
void beacon_metrics_record(beacon_metrics_t *metrics,
                           uint32_t sequence,
                           int16_t rssi_dbm,
                           int8_t snr_db,
                           uint32_t interval_ms);
void beacon_metrics_get_report(const beacon_metrics_t *metrics, beacon_metrics_report_t *report);

#endif
