#pragma once

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "consteval_assert.hpp"
#include "pico/stdlib.h"

namespace Common {

class Interface {
protected:
    bool _enabled = false;

public:
    const bool &enabled = _enabled;
    virtual int init(void) = 0;
    virtual int deinit(void) = 0;

    // Power-on self-test
    virtual int POST(void) = 0;

    inline int reinit(void) {
        if (deinit())
            return -1;
        return init();
    }
};

struct Device {
    static const char MAX_NAME_SIZE = 32;
    const char *const name;
    consteval Device(const char *const name) : name(name) {
        consteval_assert(strlen(name) < MAX_NAME_SIZE, "Device name too long.");
    };
};

} // namespace Common
