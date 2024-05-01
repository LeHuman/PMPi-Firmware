/**
 * @file flash.h
 * @author IR
 * @brief Header file for bootloader flash API
 * @note Not to be confused with hardware/flash. This is an API specifically for the bootloader to interface with to allow abstraction in handling how hardware is flashed.
 * @version 0.1
 * @date 2024-04-06
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once
#include <pico/stdlib.h>
#include <stdbool.h>
#include <stdint.h>

void flash_init();
void flash_deinit();
void flash_intake(uint16_t address, unsigned char *src, size_t sz);
void flash_finalize();
/**
 * @brief Set a new address to begin programming from
 *
 * @details This address defines what sector should be erased and where a page should start writing
 *
 * @warning Must be called at least once before ingesting data
 * @warning `address` MUST be aligned to a 4096 byte address, or whatever the size of a flash sector is
 *
 * @param address Flash address to begin programming from
 */
void flash_new_address(uint32_t address);
// void flash_start();
