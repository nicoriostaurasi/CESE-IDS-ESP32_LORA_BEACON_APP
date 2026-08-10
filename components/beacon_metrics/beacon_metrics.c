#include "beacon_metrics.h"

#include <limits.h>
#include <string.h>

void beacon_metrics_reset(beacon_metrics_t *metrics)
{
    if (metrics == NULL) {
        return;
    }
    memset(metrics, 0, sizeof(*metrics));
    metrics->rssi_min_dbm = INT16_MAX;
    metrics->rssi_max_dbm = INT16_MIN;
    metrics->snr_min_db = INT8_MAX;
    metrics->snr_max_db = INT8_MIN;
    metrics->interval_min_ms = UINT32_MAX;
}

void beacon_metrics_record(beacon_metrics_t *metrics,
                           uint32_t sequence,
                           int16_t rssi_dbm,
                           int8_t snr_db,
                           uint32_t interval_ms)
{
    if (metrics == NULL) {
        return;
    }
    /* A peer reboot restarts its sequence at 1. Start a fresh acceptance
     * window instead of classifying the whole new run as out-of-order. */
    if (metrics->initialized && sequence <= 3U && sequence < metrics->last_sequence) {
        beacon_metrics_reset(metrics);
    }
    if (!metrics->initialized) {
        metrics->initialized = true;
    } else if (sequence > metrics->last_sequence) {
        metrics->missing += sequence - metrics->last_sequence - 1U;
    } else {
        ++metrics->duplicates_or_out_of_order;
        return;
    }
    metrics->last_sequence = sequence;
    ++metrics->received;
    metrics->rssi_sum_dbm += rssi_dbm;
    metrics->snr_sum_db += snr_db;
    if (rssi_dbm < metrics->rssi_min_dbm) metrics->rssi_min_dbm = rssi_dbm;
    if (rssi_dbm > metrics->rssi_max_dbm) metrics->rssi_max_dbm = rssi_dbm;
    if (snr_db < metrics->snr_min_db) metrics->snr_min_db = snr_db;
    if (snr_db > metrics->snr_max_db) metrics->snr_max_db = snr_db;
    if (interval_ms > 0U) {
        ++metrics->interval_samples;
        metrics->interval_sum_ms += interval_ms;
        if (interval_ms < metrics->interval_min_ms) metrics->interval_min_ms = interval_ms;
        if (interval_ms > metrics->interval_max_ms) metrics->interval_max_ms = interval_ms;
    }
}

void beacon_metrics_get_report(const beacon_metrics_t *metrics, beacon_metrics_report_t *report)
{
    if (metrics == NULL || report == NULL) {
        return;
    }
    memset(report, 0, sizeof(*report));
    report->received = metrics->received;
    report->missing = metrics->missing;
    report->duplicates_or_out_of_order = metrics->duplicates_or_out_of_order;
    uint32_t expected = metrics->received + metrics->missing;
    report->delivery_rate_permille = expected == 0U ? 0U : (metrics->received * 1000U) / expected;
    if (metrics->received > 0U) {
        report->rssi_mean_dbm = (int16_t)(metrics->rssi_sum_dbm / metrics->received);
        report->rssi_min_dbm = metrics->rssi_min_dbm;
        report->rssi_max_dbm = metrics->rssi_max_dbm;
        report->snr_mean_db = (int8_t)(metrics->snr_sum_db / metrics->received);
        report->snr_min_db = metrics->snr_min_db;
        report->snr_max_db = metrics->snr_max_db;
    }
    if (metrics->interval_samples > 0U) {
        report->interval_mean_ms = (uint32_t)(metrics->interval_sum_ms / metrics->interval_samples);
        report->interval_min_ms = metrics->interval_min_ms;
        report->interval_max_ms = metrics->interval_max_ms;
    }
}
