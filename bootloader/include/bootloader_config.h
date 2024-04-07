/**
 * @file bootloader_config.h
 * @author IR
 * @brief Custom bootloader configuration header
 * @details This header file defines information about the bootloader size, program offsets, and flash hardware.
 * Offset values must match with linker script to ensure proper flashing.
 * @version 0.1
 * @date 2024-04-06
 *
 * @copyright Copyright (c) 2024
 *
 */

// The LED is connected to GPIO 25
#define LED_PIN 25
#define LED2_PIN 26

// Sector and page sizes for flash memory
#define SECTOR_SIZE 4096
#define PAGE_SIZE 256

// Application program offset in flash
// This should agree with the linker script for the application program.
#ifndef BOOTLOADER_FLASH_SIZE
    #define BOOTLOADER_FLASH_SIZE (4096 * 9)
    #define MAIN_APP_FLASH_OFFSET BOOTLOADER_FLASH_SIZE
#endif

#define BOOT_INPUT_HEX

#if defined(BOOT_INPUT_HEX)
    #define BOOT_INPUT_HEX_USB
// #define BOOT_INPUT_HEX_SPI_FLASH
#elif defined(BOOT_INPUT_ELF)
    #define BOOT_INPUT_ELF_USB
// #define BOOT_INPUT_ELF_SPI_FLASH
#elif defined(BOOT_INPUT_BIN)
    #define BOOT_INPUT_BIN_USB
// #define BOOT_INPUT_BIN_SPI_FLASH
#endif