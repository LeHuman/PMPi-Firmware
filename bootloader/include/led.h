/**
 * @file led.h
 * @author IR
 * @brief Header file for led utility functions
 * @version 0.1
 * @date 2024-04-06
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once

#include <pico/stdlib.h>
#include <stdint.h>

/**
 * @brief Blink a gpio pin
 *
 * @param gpio gpio to blink
 * @param count number of times to blink
 * @param ms length to hold blinks for in ms
 */
void blink(uint gpio, uint32_t count, uint32_t ms);

/**
 * @brief Toggle a gpio pin
 *
 * @param gpio gpio to toggle
 */
void toggle(uint gpio);

void led_init();

void led_deinit();