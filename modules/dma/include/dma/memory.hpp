#pragma once

#include <bit>
#include <cstdint>

#include <hardware/dma.h>
#include <pico/types.h>

#include "dma.hpp"

namespace DMA {

static volatile uint8_t DUMMY_BYTE = 0xFFU; // TODO: should we fix the dummy byte used?

class Memory : public DMA {
private:
    volatile void *data_register;
    uint dreq;

public:
    dma_channel_config config;

    constexpr Memory(volatile void *data_register, const uint dreq) : data_register(data_register), dreq(dreq) {}

    int32_t init() final {
        channel = dma_claim_unused_channel(false);
        if (channel < 0) {
            return channel;
        }
        const auto uchannel = static_cast<uint32_t>(channel);

        // Configure General DMA to defaults
        config = dma_channel_get_default_config(uchannel);
        channel_config_set_transfer_data_size(&config, DMA_SIZE_8);
        channel_config_set_dreq(&config, dreq);

        channel_config_set_read_increment(&config, false);
        channel_config_set_write_increment(&config, false);

        dma_channel_set_read_addr(uchannel, nullptr, false);
        dma_channel_set_write_addr(uchannel, nullptr, false);
        dma_channel_set_trans_count(uchannel, 0, false);
        dma_channel_set_config(uchannel, &config, false);
        _enabled = true;
        return 0;
    }

    int32_t deinit() final {
        if (channel < 0) {
            return 1;
        }
        // TODO: mutex here
        dma_channel_unclaim(static_cast<uint32_t>(channel));
        _enabled = false;
        channel = -1;
        return 0;
    }

    void set_transfer_request_signal(const uint source) {
        channel_config_set_dreq(&config, source);
    }

    void set_transfer_size(const dma_channel_transfer_size transfer_size) {
        channel_config_set_transfer_data_size(&config, transfer_size);
    }

    void set_increment(const bool on_read, const bool on_write) {
        channel_config_set_read_increment(&config, on_read);
        channel_config_set_write_increment(&config, on_write);
    }

    // NOTE: Make sure dma is not busy
    void commit_config() const {
        dma_channel_set_config(static_cast<uint32_t>(channel), &config, false);
    }

    void set_read(volatile void *dst, const uint32_t length, const bool dummy = false) {
        set_increment(false, !dummy);
        if (dummy) {
            dst = std::bit_cast<volatile void *>(&DUMMY_BYTE);
        }
        commit_config();
        const auto uchannel = static_cast<uint32_t>(channel);
        dma_channel_set_read_addr(uchannel, data_register, false);
        dma_channel_set_write_addr(uchannel, dst, false);
        dma_channel_set_trans_count(uchannel, length, false);
    }

    void set_write(const volatile void *src, const uint32_t length, const bool dummy = false) {
        set_increment(!dummy, false);
        if (dummy) {
            src = std::bit_cast<volatile void *>(&DUMMY_BYTE);
        }
        commit_config();
        const auto uchannel = static_cast<uint32_t>(channel);
        dma_channel_set_read_addr(uchannel, src, false);
        dma_channel_set_write_addr(uchannel, data_register, false);
        dma_channel_set_trans_count(uchannel, length, false);
    }

    int32_t wait() const {
        if (channel < 0) {
            return -1;
        }
        dma_channel_wait_for_finish_blocking(static_cast<uint32_t>(channel));
        return 0;
    }

    bool is_busy() const {
        return (channel >= 0) && dma_channel_is_busy(static_cast<uint32_t>(channel));
    }

    int32_t POST() final {
        // TODO: DMA POST
        return 0;
    }
};

} // namespace DMA
