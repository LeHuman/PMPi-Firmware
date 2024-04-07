/**
 * @file flash.c
 * @author IR
 * @brief Source file for bootloader flash API
 * @note Not to be confused with hardware/flash. This is an API specifically for the bootloader to interface with to allow abstraction in handling how hardware is flashed.
 * @version 0.1
 * @date 2024-04-06
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "flash.h"

#include <dma_util.h>
#include <hardware/dma.h>
#include <hardware/flash.h>

#include "bootloader_config.h"
#include "led.h"

// For keeping track of where we are in programming/erasing flash
uint32_t programming_address = 0;
uint32_t erasure_address = 0;

int dma_flash_clear; // Clears flashbuffer
int dma_flash_copy;  // Copies flashbuffer to first_page_buffer

// Flash memory buffer and index
unsigned char flashbuffer[PAGE_SIZE] = {0};
unsigned char flashdex = 0;

// We're going to program the first page LAST, so we don't
// accidentally vector into a partially-programmed application
unsigned char first_page = 0;
unsigned char first_page_buffer[PAGE_SIZE] = {0};

void flash_init() {
    first_page = 0;
    programming_address = 0;
    erasure_address = 0;
    dma_flash_clear = dma_init(flashbuffer, &nil, PAGE_SIZE, DMA_SIZE_8, false, true);
    dma_flash_copy = dma_init(first_page_buffer, flashbuffer, PAGE_SIZE, DMA_SIZE_8, true, true);
}

void flash_deinit() {
    dma_deinit(dma_flash_clear);
    dma_deinit(dma_flash_copy);
}

// After we've received a data hexline, buffer that data into the flashbuffer.
// If the flashbuffer is full, then write the buffer to flash memory at the
// programming address. If there's no more space in the erased sector of
// flash memory, then erase the next sector before programming.
void flash_intake(unsigned char *src, size_t sz) {
    for (int i = 0; i < sz; i++) {
        // Store a byte from the data buffer into the flashbuffer.
        // Increment the flashdex. This will overflow from 255->0.
        flashbuffer[flashdex++] = src[i];

        // Did we just overflow to zero? If so, it's time to program
        // the buffer into flash memory.
        if (flashdex == 0) {
            // Is there space remaining in the erased sector?
            // If so ...
            if (programming_address < erasure_address) {
                // If this is the first page, copy it to the first_page_buffer
                // and reset first_page
                if (!first_page) {
                    // Copy contents of flashbuffer to first_page_buffer
                    dma_channel_set_read_addr(dma_flash_copy, flashbuffer, false);
                    dma_channel_set_write_addr(dma_flash_copy, first_page_buffer, true);
                    dma_channel_wait_for_finish_blocking(dma_flash_copy);
                    // Reset first_page
                    first_page = 1;
                } else {
                    // Program flash memory at the programming address
                    flash_range_program(programming_address, flashbuffer, PAGE_SIZE);
                }
            } else { // If not ...
                // Erase the next sector (4096 bytes)
                flash_range_erase(erasure_address, SECTOR_SIZE);

                // Increment the erasure pointer by 4096 bytes
                erasure_address += SECTOR_SIZE;

                // Program flash memory at the programming address
                flash_range_program(programming_address, flashbuffer, PAGE_SIZE);
            }

            // Increment the programming address by page size (256 bytes)
            programming_address += PAGE_SIZE;

            // Clear the flashbuffer. Flashdex autowraps to 0.
            dma_channel_set_write_addr(dma_flash_clear, flashbuffer, true);
            dma_channel_wait_for_finish_blocking(dma_flash_clear);
        }
    }
    toggle(LED_PIN);
}

void flash_new_address(uint32_t address) {
    // Flash remaining data from flash buffer to sector
    if (flashdex) {
        // IMPROVE: better handle for overflow in the case of switching to a new address
        if (programming_address < erasure_address) {
            // Program flash memory at the programming address
            flash_range_program(programming_address, flashbuffer, PAGE_SIZE);
        } else { // If not . . .
            // Erase the next sector (4096 bytes)
            flash_range_erase(erasure_address, SECTOR_SIZE);
            // Program flash memory at the programming address
            flash_range_program(programming_address, flashbuffer, PAGE_SIZE);
        }
        // NOTE: This sector may be partially full, but *should* not be accessed again
    }

    // Clear the flashbuffer
    dma_channel_set_write_addr(dma_flash_clear, flashbuffer, true);
    dma_channel_wait_for_finish_blocking(dma_flash_clear);

    // Reset the flashdex to 0
    flashdex = 0;

    // Erase the next sector defined by address (4096 bytes)
    flash_range_erase(address, SECTOR_SIZE);

    // Set the programming address and next sector to erase
    programming_address = address;
    erasure_address = address + SECTOR_SIZE;
}

void flash_finalize() {
    // We may have a partially-full buffer when we get the end of file hexline.
    // If so, write that last buffer to flash memory.

    // Program the final flashbuffer.
    // Is there space remaining in the erased sector?
    // If so . . .
    if (programming_address < erasure_address) {
        // Program flash memory at the programming address
        flash_range_program(programming_address, flashbuffer, PAGE_SIZE);
    } else { // If not . . .
        // Erase the next sector (4096 bytes)
        flash_range_erase(erasure_address, SECTOR_SIZE);
        // Program flash memory at the programming address
        flash_range_program(programming_address, flashbuffer, PAGE_SIZE);
    }

    // We program the first page LAST, since we use it to figure out if there's
    // a valid program to vector into.
    flash_range_program(MAIN_APP_FLASH_OFFSET, first_page_buffer, PAGE_SIZE);
    // TODO: use a crc header instead of static values
}

// Initializes erasure address and programming address.
// Clears the flashbuffer, zeroes the flashdex.
// Erases the first sector of flash, and increments erasure
// address by the sector size (4096 bytes).
// void flash_start() {
//     // Reset the erasure pointer
//     erasure_address = MAIN_APP_FLASH_OFFSET;

//     // Reset the programming pointer
//     programming_address = MAIN_APP_FLASH_OFFSET;

//     // // Clear the flashbuffer
//     // dma_channel_set_write_addr(dma_flash_clear, flashbuffer, true);
//     // dma_channel_wait_for_finish_blocking(dma_flash_clear);

//     // Reset the first_page flag to 0
//     first_page = 0;

//     // Erase the first sector (4096 bytes)
//     flash_range_erase(erasure_address, SECTOR_SIZE);

//     // Increment the erasure pointer by 4096 bytes
//     erasure_address += SECTOR_SIZE;
// }