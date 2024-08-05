#pragma once

#include <cstdint>
#include <cstdio>
#include <cstring>

#include "consteval_assert.hpp"
#include "pico/stdlib.h"

namespace Common {

class Interface {
protected:
    bool _enabled = false;

public:
    const bool &enabled = _enabled;
    virtual int32_t init(void) = 0;
    virtual int32_t deinit(void) = 0;

    // Power-on self-test
    virtual int32_t POST(void) = 0;

    inline int32_t reinit(void) {
        if (deinit())
            return -1;
        return init();
    }
};

struct Device {
    static const char MAX_NAME_SIZE = 32u;
    const char *const name;
    consteval Device(const char *const name) : name(name) {
        consteval_assert(strlen(name) < MAX_NAME_SIZE, "Device name too long.");
    };
};

} // namespace Common
