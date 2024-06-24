#pragma once

#include <FreeRTOS.h>
#include <pico/stdlib.h>

#include "task.h"

namespace Common {

static inline uint32_t millis() {
    return xTaskGetTickCount() * (1000 / configTICK_RATE_HZ);
}

} // namespace Common
