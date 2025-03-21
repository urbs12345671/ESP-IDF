#pragma once

#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

// Command structure for the job queue
typedef struct {
    int job_id;
} job_command_t;

// Result structure for the result queue
typedef struct {
    int job_id;
    int progress;
    bool completed;
    bool sensor_digital_result;
} job_result_t;



