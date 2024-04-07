/**
 * @file bootloader.h
 * @author IR
 * @brief Header file for core bootloader functionality
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

#include "bootloader_config.h"

/**
 * @brief Exit out of the bootloader and branch into the program loaded in flash
 *
 * @details Sets the VTOR register and stack pointer then jumps to the reset vector in the main app. Basically copied from crt0.S.
 */
void bootloader_exit();

/**
 * @brief Initialize bootloader peripherals and variables.
 */
void bootloader_init(void);

/**
 * @brief Deinitialize bootloader peripherals and variables.
 */
void bootloader_deinit(void);

/**
 * @brief Loads a new program into flash.
 *
 * @details Exact method for loading a new program is managed internally
 *
 * @retval true Successfully loaded a new program
 * @retval false Failed to load a new program
 */
bool bootloader_load_program();

/**
 * @brief Returns whether the bootloader should run
 *
 * @retval true Invalid program loaded or manual entry set for bootloader
 * @retval false Valid program loaded, call bootloader_exit
 */
bool bootloader_should_run(void);

/**
 * @brief Triggers a soft reset
 *
 * @note This function should not return
 */
void bootloader_soft_reset(void);