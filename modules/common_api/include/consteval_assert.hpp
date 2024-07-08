#pragma once

// TODO: Test that running this in non consteval still runs divide by zero check

/**
 * @brief Assert for `consteval` functions to use function parameters
 *
 * @note Will throw `error: '(1 / 0)' is not a constant expression` on compilation if assertion failed
 *
 * @warning Ensure function is actually `consteval` otherwise this assert will not run and no warning can be issued
 *
 * @param condition The condition to check
 * @param message The optional string literal message
 */
#define consteval_assert(condition, message...) \
    if consteval {                              \
        if (1 / (condition)) {                  \
        }                                       \
    }
