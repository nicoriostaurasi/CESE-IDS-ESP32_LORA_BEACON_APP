#include "beacon_metrics.h"

#include <assert.h>
#include <stdio.h>

int main(void)
{
    beacon_metrics_t metrics;
    beacon_metrics_reset(&metrics);
    beacon_metrics_record(&metrics, 10U, -40, 10, 0U);
    beacon_metrics_record(&metrics, 11U, -42, 8, 3000U);
    beacon_metrics_record(&metrics, 14U, -44, 6, 3100U);
    beacon_metrics_record(&metrics, 14U, -1, 1, 1U);

    beacon_metrics_report_t report;
    beacon_metrics_get_report(&metrics, &report);
    assert(report.received == 3U);
    assert(report.missing == 2U);
    assert(report.duplicates_or_out_of_order == 1U);
    assert(report.delivery_rate_permille == 600U);
    assert(report.rssi_mean_dbm == -42);
    assert(report.rssi_min_dbm == -44);
    assert(report.rssi_max_dbm == -40);
    assert(report.snr_mean_db == 8);
    assert(report.interval_mean_ms == 3050U);
    assert(report.interval_min_ms == 3000U);
    assert(report.interval_max_ms == 3100U);

    beacon_metrics_record(&metrics, 1U, -50, 5, 3000U);
    beacon_metrics_get_report(&metrics, &report);
    assert(report.received == 1U);
    assert(metrics.last_sequence == 1U);
    assert(report.missing == 0U);
    puts("PASS beacon_metrics host tests");
    return 0;
}
