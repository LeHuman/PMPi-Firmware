/**
 * @file dma_util.c
 * @author IR
 * @brief Source file for dma utility functions
 * @version 0.1
 * @date 2024-04-06
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "dma_util.h"

int dma_init(volatile void *write_addr, const volatile void *read_addr, uint transfer_count, enum dma_channel_transfer_size transfer_size, bool read_inc, bool write_inc) {
    int handle = dma_claim_unused_channel(true);

    dma_channel_config config = dma_channel_get_default_config(handle);
    channel_config_set_transfer_data_size(&config, transfer_size);
    channel_config_set_read_increment(&config, read_inc);
    channel_config_set_write_increment(&config, write_inc);

    dma_channel_configure(
        handle,         // Channel to be configured
        &config,        // The configuration we just created
        write_addr,     // write address
        read_addr,      // read address
        transfer_count, // Number of transfers; in this case each is 1 byte.
        false           // Don't start immediately.
    );

    return handle;
}

void dma_deinit(int handle) {
    dma_channel_cleanup(handle);
    dma_channel_unclaim(handle);
}