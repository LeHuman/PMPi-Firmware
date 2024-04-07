/**
 * @file hex.h
 * @author IR
 * @brief Header file for the HEX input scheme
 * @version 0.1
 * @date 2024-04-06
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once
#include <stdbool.h>
#include <stdint.h>

typedef enum HexType {
    HEX_Data = 0x00,
    HEX_EndOfFile = 0x01,
    HEX_ExtendedSegmentAddress = 0x02,
    HEX_StartSegmentAddress = 0x03,
    HEX_ExtendedLinearAddress = 0x04,
    HEX_ExtendedStartAddress = 0x05
} HexType;

/**
 * @brief Global HEX structure used to intake lines of hex
 */
extern struct HEX_t {
    uint8_t count;
    uint16_t address;
    HexType type;
    uint8_t data[16];
    uint8_t checksum;
} HEX;

/**
 * @brief Initialize peripherals for this input scheme
 */
void hex_init(void);

/**
 * @brief Deinitialize peripherals for this input scheme
 */
void hex_deinit(void);

/**
 * @brief Load a line of hex into the HEX structure
 *
 * @retval true Sucessfully loaded into HEX
 * @retval false Failed to load into HEX
 */
bool hex_load(void);
