#pragma once

#include <stdint.h>

#include "hardware/gpio.h"
#include "types.hpp"

namespace GPIB {

enum BusMode {
    InputPullup,
    OutputHigh,
    OutputLow,
};

class GPIBBus {
public:
    const Config config;

    static inline uint32_t get(GPIBMask_t mask) {
        // TODO: Check if bus is set to input
        return gpio_get_all() & mask;
    }

    static inline void put(GPIBMask_t mask, uint32_t value) {
        // TODO: Check if bus is set to output
        return gpio_put_masked(mask, value);
    }

    void setMode(GPIBMask_t mask, BusMode state);

    consteval GPIBBus(const Config &config) : config(config) {}
};

} // namespace GPIB