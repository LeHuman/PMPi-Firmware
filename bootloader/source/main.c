/**
 * @file main.c
 * @author IR
 * @brief Main source file for the custom bootloader. Handles high level logic of bootloader.
 * @version 0.1
 * @date 2024-04-06
 *
 * @copyright Copyright (c) 2024
 *
 * @ref https://github.com/vha3/Hunter-Adams-RP2040-Demos/tree/master/Bootloaders/Serial_bootloader
 * @ref https://github.com/usedbytes/rp2040-serial-bootloader
 */

#include <pico/stdlib.h>

#include "bootloader.h"
#include "bootloader_config.h"

int main() {
#ifndef NDEBUG
    // All timers are paused when core 0 is not specifically chosen for debug
    // https://github.com/raspberrypi/pico-sdk/issues/1152
    timer_hw->dbgpause = 0;
#endif

    if (!bootloader_should_run()) {
        bootloader_exit();
    }

    bootloader_init();

    if (!bootloader_load_program()) {
        // TODO: fallback program
        // Soft reset into bootloader on fail
        bootloader_deinit();
        bootloader_soft_reset();
    }

    bootloader_deinit();
    bootloader_exit(); // TODO: Check if program is valid before exiting
}
