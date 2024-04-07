/**
 * @file led.c
 * @author IR
 * @brief Source file for led utility functions
 * @version 0.1
 * @date 2024-04-06
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "led.h"

#include "bootloader_config.h"

void blink(uint gpio, uint32_t count, uint32_t ms) {
    for (uint32_t i = 0; i < count; i++) {
        gpio_put(gpio, 1);
        sleep_ms(ms);
        gpio_put(gpio, 0);
        sleep_ms(ms);
    }
}

void toggle(uint gpio) {
    gpio_put(gpio, !gpio_get(gpio));
}

void led_init() {
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_init(LED2_PIN);
    gpio_set_dir(LED2_PIN, GPIO_OUT);
}

void led_deinit() {
    gpio_deinit(LED_PIN);
    gpio_deinit(LED2_PIN);
}