/**
 * @file hex.c
 * @author IR
 * @brief Source file for the HEX input scheme
 * @version 0.1
 * @date 2024-04-06
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "hex.h"

#include <pico/stdlib.h>

#include "bootloader_config.h"
#include "led.h"

struct HEX_t HEX;

#if defined(BOOT_INPUT_HEX_USB)

    #include <stdio.h>

    #include "dma_util.h"
    #include "hardware/dma.h"

    // Length of our buffer (bytes)
    #define BUFFER_LEN 44

static volatile unsigned char buffer[BUFFER_LEN] = {0x00};
static volatile unsigned char load_buffer[BUFFER_LEN] = {0x00};

static uint32_t buf_count;   // Counts received bytes
static int dma_checksum;     // Performs checksum
static int dma_clear_buffer; // Clears buffer

/**
 * @brief Retrieve and buffer a line of Hex (ASCII)
 *
 * @param buf Buffer to push to
 */
void getLine(volatile unsigned char *buf) {
    // Clear the buffer (DMA for speed)
    dma_channel_set_write_addr(dma_clear_buffer, buf, true);
    dma_channel_wait_for_finish_blocking(dma_clear_buffer);

    // Count characters as we receive them
    buf_count = 0;

    // Buffer until the starting char ':'
    char ch;

    // FIXME: Handle case where ':' is not found / getchar timesout
    do {
        ch = getchar();
    } while (ch != ':');

    ch = getchar();

    // FIXME: Handle case where no '\n' or '\r' is given
    while ((ch != '\n') && (ch != '\r')) {
        *buf++ = ch;
        buf_count++;
        ch = getchar();
    };

    // Null-terminate the string
    *buf++ = 0;
}

void char2hex(volatile char val, unsigned char *num) {
    if (val >= 'A')
        *num |= val - 'A' + 10;
    else
        *num |= val - '0';
}

// Tranforms line from ASCII to binary representation
void transformLine(volatile unsigned char *buf) {
    uint32_t i = 0;

    for (; i < buf_count / 2; i++) {
        uint8_t num = 0;

        char2hex(buf[i * 2], &num);
        num <<= 4;
        char2hex(buf[(i * 2) + 1], &num);

        buf[i] = num;
    }

    // Zero-out the rest of the buffer
    while (i < BUFFER_LEN) {
        buf[i++] = 0;
    }
}

// Performs a checksum on the received hex line
uint32_t checkLine(volatile unsigned char *src, volatile unsigned char *dst) {

    // Reset the sniff register to zero
    dma_sniffer_set_data_accumulator(0x00000000);

    // Reset DMA read and write addresses, trigger and wait for finish
    dma_channel_set_read_addr(dma_checksum, src, false);
    dma_channel_set_write_addr(dma_checksum, dst, true);
    dma_channel_wait_for_finish_blocking(dma_checksum);

    // Return 2's complement of LSB of checksum. If zero, check passed
    return ((~(dma_sniffer_get_data_accumulator()) + 1) & 0x000000FF);
}

// Parse the verified line into length, address, type, data, and checksum
void parseLine(volatile unsigned char *buf) {

    // Iterator
    int i;

    // Grab length, address, hextype
    HEX.count = buf[0];
    HEX.address = (buf[1] << 8) | buf[2];
    HEX.type = buf[3];

    // Grab the data
    for (i = 0; i < HEX.count; i++) {
        HEX.data[i] = buf[i + 4];
    }

    // Grab the checksum
    HEX.checksum = buf[HEX.count + 4];
}

// Acquire a single hexline. Return 1 if valid, 0 if checksum failed
int acquireLine() {
    // Did the checksum pass?
    uint32_t valid;

    putchar(0);
    stdio_flush();

    // Get a raw line of ASCII characters
    getLine(buffer);

    // Transform those ASCII characters to binary
    transformLine(buffer);

    // Perform a checksum
    valid = checkLine(buffer, load_buffer);
    // valid = 0; // FIXME: checksum not working?

    // If passed, parse the line. Else return 0.
    if (valid == 0) {
        // Next line
        putchar(1);
        stdio_flush();
        parseLine(load_buffer);
        return 1;
    } else {
        blink(LED2_PIN, 1, 50);
        return 0;
    }
}

void hex_init(void) {
    stdio_usb_init();

    dma_checksum = dma_init(load_buffer, buffer, BUFFER_LEN, DMA_SIZE_8, true, true);
    dma_clear_buffer = dma_init(buffer, &nil, BUFFER_LEN, DMA_SIZE_8, false, true);

    // Configure the sniffer
    dma_sniffer_enable(dma_checksum, 0x0F, true);
    hw_set_bits(&dma_hw->sniff_data, 0x0);
}

void hex_deinit(void) {
    // TODO: is there anyway / reason to deinit stdio_usb?

    dma_deinit(dma_checksum);
    dma_deinit(dma_clear_buffer);

    // Disable the sniffer
    dma_sniffer_disable();

    puts("Branching\n");
    stdio_flush();
}

bool hex_load(void) {
    while (!acquireLine()) {
    }

    return true;
}

#elif defined(BOOT_INPUT_HEX_SPI_FLASH)
    #error "SPI Flash input for hex files not defined"
#endif