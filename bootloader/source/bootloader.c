/**
 * @file bootloader.c
 * @author IR
 * @brief Source file for core bootloader functionality
 * @version 0.1
 * @date 2024-04-06
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "bootloader.h"

#include <hardware/watchdog.h>

#include "flash.h"
#include "led.h"

#if defined(BOOT_INPUT_HEX)
    #include "hex.h"
    #define input_init hex_init
    #define input_deinit hex_deinit
#elif defined(BOOT_INPUT_ELF)
    #include "elf.h"
    #define input_init elf_init
    #define input_deinit elf_deinit
#elif defined(BOOT_INPUT_BIN)
    #include "bin.h"
    #define input_init bin_init
    #define input_deinit bin_deinit
#endif

bool bootloader_load_program() {
#if defined(BOOT_INPUT_HEX)
    uint16_t msb_addr = 0;
    bool new_addr = false;
    while (hex_load()) {
        sleep_ms(1); // FIXME: delay is needed, otherwise pico hard crashes. Issue with flash writing speed?
        switch (HEX.type) {
            case HEX_Data:
                if (new_addr) {
                    flash_new_address(((msb_addr << 16) | HEX.address) - XIP_BASE);
                    new_addr = false;
                }
                flash_intake(HEX.data, HEX.count);
                continue;
            case HEX_EndOfFile:
                if (new_addr) {
                    flash_new_address(((msb_addr << 16) | HEX.address) - XIP_BASE);
                    new_addr = false;
                }
                flash_finalize();
                blink(LED2_PIN, 10, 50);
                return true;
            case HEX_ExtendedLinearAddress:
                msb_addr = (HEX.data[0] << 8) | HEX.data[1];
                new_addr = true;
                continue;
            default:
                continue;
        }
    }
#elif defined(BOOT_INPUT_ELF)
    #error "Protocol for loading .ELF files NOT defined"
#elif defined(BOOT_INPUT_BIN)
    #error "Protocol for loading .BIN files NOT defined"
#else
    #error "Valid protocol for loading a binary NOT declared"
#endif
    return false;
}

void bootloader_exit() {
    asm volatile(
        "mov r0, %[start]\n"
        "ldr r1, =%[vtable]\n"
        "str r0, [r1]\n"
        "ldmia r0, {r0, r1}\n"
        "msr msp, r0\n"
        "bx r1\n"
        :
        : [start] "r"(XIP_BASE + MAIN_APP_FLASH_OFFSET), [vtable] "X"(PPB_BASE + M0PLUS_VTOR_OFFSET)
        :);
}

void bootloader_init(void) {
    led_init();
    flash_init();
    input_init();

    blink(LED_PIN, 4, 100);
}

void bootloader_deinit(void) {
    input_deinit();
    flash_deinit();
    led_deinit();

    // Disable systick
    // hw_set_bits((io_rw_32 *)0xe000e010, 0xFFFFFFFF);

    // Turn off interrupts (NVIC ICER, NVIC ICPR)
    hw_set_bits((io_rw_32 *)0xe000e180, 0xFFFFFFFF);
    hw_set_bits((io_rw_32 *)0xe000e280, 0xFFFFFFFF);

    // SysTick->CTRL &= ~1;
}

void bootloader_soft_reset() {
    watchdog_hw->scratch[0] = 1;
    watchdog_reboot(0, 0, 1);
    while (1) {
    }
}

bool bootloader_should_run() {
    static uint8_t *flash_target_contents = (uint8_t *)(XIP_BASE + MAIN_APP_FLASH_OFFSET + 265);
    uint8_t invalid_program = (flash_target_contents[0] != 0xF2) || (flash_target_contents[1] != 0xEB) || (flash_target_contents[2] != 0x88) || (flash_target_contents[3] != 0x71);

    if (watchdog_hw->scratch[0] || invalid_program) {
        // Reset WD scratch on soft-reset into bootloader
        watchdog_hw->scratch[0] = 0;

        return true;
    }

    return false;
}
