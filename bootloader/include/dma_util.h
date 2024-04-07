/**
 * @file dma_util.h
 * @author IR
 * @brief Header file for dma utility functions
 * @version 0.1
 * @date 2024-04-06
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "hardware/dma.h"

/**
 * @brief Used for DMA's clearing buffers
 */
static const char nil = 0;

int dma_init(volatile void *write_addr, const volatile void *read_addr, uint transfer_count, enum dma_channel_transfer_size transfer_size, bool read_inc, bool write_inc);

void dma_deinit(int handle);
