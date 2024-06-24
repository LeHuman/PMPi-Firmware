#pragma once

#include <FreeRTOS.h>
#include <pico/stdlib.h>

#include "task.h"

namespace Common {

static constexpr inline TickType_t pdUS_TO_TICKS(uint64_t xTimeInUs) {
    return (TickType_t)(((uint64_t)(xTimeInUs) * (uint64_t)configTICK_RATE_HZ) / (uint64_t)1000000U);
}

static inline void delay_ms(uint64_t ms) {
    vTaskDelay(pdMS_TO_TICKS(ms));
}
static inline void delay_us(uint64_t ms) {
    vTaskDelay(pdUS_TO_TICKS(ms));
}

} // namespace Common
