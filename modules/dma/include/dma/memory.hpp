#pragma once

#include <hardware/dma.h>
#include <pico/types.h>
#include <stdint.h>

#include "dma.hpp"

namespace DMA {

static volatile uint8_t DUMMY_BYTE = 0xFFU; // TODO: should we fix the dummy byte used?

class Memory : public DMA {
private:
    uint dreq;
    volatile void *data_register;

public:
    dma_channel_config config;

    constexpr Memory(volatile void *data_register, uint dreq) : data_register(data_register), dreq(dreq) {}

    int init(void) {
        channel = dma_claim_unused_channel(false);
        if (channel < 0) {
            return channel;
        }
        // Configure General DMA to defaults
        config = dma_channel_get_default_config(channel);
        channel_config_set_transfer_data_size(&config, DMA_SIZE_8);
        channel_config_set_dreq(&config, dreq);

        channel_config_set_read_increment(&config, false);
        channel_config_set_write_increment(&config, false);

        dma_channel_set_read_addr(channel, nullptr, false);
        dma_channel_set_write_addr(channel, nullptr, false);
        dma_channel_set_trans_count(channel, 0, false);
        dma_channel_set_config(channel, &config, false);
        _enabled = true;
        return 0;
    }

    int deinit() {
        if (channel < 0) {
            return 1;
        }
        // TODO: mutex here
        dma_channel_unclaim(channel);
        _enabled = false;
        channel = -1;
        return 0;
    }

    void set_transfer_request_signal(uint dreq) {
        channel_config_set_dreq(&config, dreq);
    }

    void set_transfer_size(dma_channel_transfer_size transfer_size) {
        channel_config_set_transfer_data_size(&config, transfer_size);
    }

    void set_increment(bool on_read, bool on_write) {
        channel_config_set_read_increment(&config, on_read);
        channel_config_set_write_increment(&config, on_write);
    }

    // NOTE: Make sure dma is not busy
    void commit_config() {
        dma_channel_set_config(channel, &config, false);
    }

    void set_read(volatile void *dst, uint32_t length, bool dummy = false) {
        set_increment(false, !dummy);
        if (dummy == true) {
            dst = reinterpret_cast<volatile void *>(&DUMMY_BYTE);
        }
        commit_config();
        dma_channel_set_read_addr(channel, data_register, false);
        dma_channel_set_write_addr(channel, dst, false);
        dma_channel_set_trans_count(channel, length, false);
    }

    void set_write(const volatile void *src, uint32_t length, bool dummy = false) {
        set_increment(!dummy, false);
        if (dummy == true) {
            src = reinterpret_cast<volatile void *>(&DUMMY_BYTE);
        }
        commit_config();
        dma_channel_set_read_addr(channel, src, false);
        dma_channel_set_write_addr(channel, data_register, false);
        dma_channel_set_trans_count(channel, length, false);
    }

    int wait() {
        if (channel < 0) {
            return -1;
        }
        dma_channel_wait_for_finish_blocking(channel);
        return 0;
    }

    bool is_busy() {
        return (channel >= 0) && dma_channel_is_busy(channel);
    }

    int POST() {
        // TODO: DMA POST
        return 0;
    }
};

} // namespace DMA