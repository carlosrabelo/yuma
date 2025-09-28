#pragma once

#include <Arduino.h>

struct Metrics {
    unsigned long shares_ok = 0;
    unsigned long shares_bad = 0;
    unsigned long jobs_received = 0;
    unsigned long uptime_start = 0;
    bool pool_connected = false;
    double current_difficulty = 0;
    String last_job_id = "";
    unsigned long last_share_time = 0;
    int connected_miners_count = 0;
};
