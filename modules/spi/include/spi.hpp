#pragma once

#include <atomic>
#include <bit>
#include <cstdint>

#include <hardware/gpio.h>
#include <hardware/resets.h>
#include <hardware/spi.h>
#include <pico/stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "common/gpio.hpp"
#include "common/macro.hpp"
#include "common/peripheral.hpp"
#include "dma/memory.hpp"

class SPI : public Common::Interface {
public:
    enum class Instance {
        SPI_0,
        SPI_1,
    };

private:
    static constexpr uintptr_t get_spi_instance(const Instance instance) {
        // Establish what SPI instance we are using
        switch (instance) {
            case Instance::SPI_0:
                return SPI0_BASE;
            case Instance::SPI_1:
                return SPI1_BASE;
            default:
                consteval_assert(false, "Invalid spi instance chosen.");
                return SPI0_BASE;
        }
    }

    static constexpr uint32_t get_spi_reset_bits(const Instance instance) {
        switch (instance) {
            case Instance::SPI_0:
                return RESETS_RESET_SPI0_BITS;
            case Instance::SPI_1:
                return RESETS_RESET_SPI1_BITS;
            default:
                consteval_assert(false, "Invalid spi instance chosen.");
                return RESETS_RESET_SPI0_BITS;
        }
    }

public:
    struct Device : Common::Device {
        const Instance instance;
        const uint32_t baudrate;
        const struct Pinout_t {
            const int8_t Tx;   // MOSI IO0
            const int8_t Rx;   // MISO IO1
            const int8_t HOLD; // Hold IO2
            const int8_t WP;   // WP IO3
            const int8_t SCK;  // CLK
            const int8_t CS;   // CSn
        } pinout;

        consteval Device(const char *const name, const Instance instance, const uint32_t baudrate, const Pinout_t pinout) : Common::Device{name}, instance(instance), baudrate(baudrate), pinout(pinout) {
            switch (instance) {
                case Instance::SPI_0:
                case Instance::SPI_1:
                    break;
                default:
                    consteval_assert(false, "Invalid spi instance chosen.");
                    break;
            }
        };
    };

    // TODO: x4 Read and Writes, will probably require pio
    const Device &dev;

private:
    uint32_t baudrate = 0;
    std::atomic_int transfer = 0;

    // TODO: Better HOLD and WP control
    bool HOLD = false;
    bool WP = false;

    const uint32_t reset_bits;
    const uintptr_t spi_ptr;

    struct {
        DMA::Memory tx;
        DMA::Memory rx;
    } dma;
    spi_inst_t *spi;
    spi_hw_t *spi_hw;

    inline void cs_select(void) const {
        asm volatile("nop \n nop \n nop");
        gpio_put(dev.pinout.CS, 0); // Active low
        asm volatile("nop \n nop \n nop");
    }

    inline void cs_deselect(void) {
        asm volatile("nop \n nop \n nop");
        gpio_put(dev.pinout.CS, 1);
        asm volatile("nop \n nop \n nop");
    }

public:
    SPI(const Device &dev) : dev(dev), reset_bits(get_spi_reset_bits(dev.instance)), spi_ptr(get_spi_instance(dev.instance)), spi(reinterpret_cast<spi_inst_t *>(spi_ptr)), spi_hw(reinterpret_cast<spi_hw_t *>(spi_ptr)), dma{{&spi_hw->dr, spi_get_dreq(spi, true)}, {&spi_hw->dr, spi_get_dreq(spi, false)}} {
    }

    int32_t init() {
        // To begin with, claim DMAs
        if (dma.tx.init() < 0 || dma.rx.init() < 0) {
            int tx_unclaim = dma.tx.deinit();
            int rx_unclaim = dma.rx.deinit();
            if (tx_unclaim < 0 || rx_unclaim < 0) {
                return -3;
            }
            return -2;
        }

        // Setup Hold and Write protect pins if not negative
        if (dev.pinout.HOLD >= 0) {
            gpio_set_dir(dev.pinout.HOLD, GPIO_OUT);
            gpio_put(dev.pinout.HOLD, true);
            gpio_set_function(dev.pinout.HOLD, GPIO_FUNC_SIO);
            HOLD = true;
        }

        if (dev.pinout.WP >= 0) {
            gpio_set_dir(dev.pinout.WP, GPIO_OUT);
            gpio_put(dev.pinout.WP, false);
            gpio_set_function(dev.pinout.WP, GPIO_FUNC_SIO);
            WP = true;
        }

        // Full reset
        reset_block(reset_bits);
        unreset_block_wait(reset_bits);
        // Attempt to set baudrate
        baudrate = spi_set_baudrate(spi, dev.baudrate); // TODO: What do we do if baudrate does not match or is not similar?
        // Use TI's format, supported by the MT29F4G01ABAFD
        spi_set_format(spi, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);
        // Always enable DREQ signals -- harmless if DMA is not listening
        hw_set_bits(&spi_hw->dmacr, SPI_SSPDMACR_TXDMAE_BITS | SPI_SSPDMACR_RXDMAE_BITS);
        // Finally enable the SPI
        hw_set_bits(&spi_hw->cr1, SPI_SSPCR1_SSE_BITS);

        gpio_set_dir(dev.pinout.CS, GPIO_OUT);
        gpio_put(dev.pinout.CS, 1);
        gpio_set_function(dev.pinout.CS, GPIO_FUNC_SIO);
        gpio_set_function(dev.pinout.Rx, GPIO_FUNC_SPI);
        gpio_set_function(dev.pinout.Tx, GPIO_FUNC_SPI);
        gpio_set_function(dev.pinout.SCK, GPIO_FUNC_SPI);

        _enabled = true;
        return -(baudrate == 0);
    }

    int32_t deinit() {
        spi_deinit(spi);
        baudrate = 0;
        _enabled = false;

        int tx_unclaim = dma.tx.deinit();
        int rx_unclaim = dma.rx.deinit();

        if (tx_unclaim < 0 || rx_unclaim < 0) {
            return -3;
        }
        return 0;
    }

    bool set_hold(bool use_with_transfers) {
        HOLD = (dev.pinout.HOLD >= 0) && use_with_transfers;
        return HOLD;
    }

    bool set_write_protect(bool use_with_transfers) {
        WP = (dev.pinout.WP >= 0) && use_with_transfers;
        return WP;
    }

    int start_transfer(const uint8_t *tx, uint8_t *rx, size_t length, bool blocking) {
        if (blocking) {
            wait();
        } else if (dma.rx.is_busy() || dma.tx.is_busy()) {
            return -2;
        }
        dma.tx.set_write(tx, length, (tx == nullptr));
        dma.rx.set_read(rx, length, (rx == nullptr));

        // If we are in the middle of a transfer, just trigger dma
        if (transfer == 0) {
            transfer = 1;
            if (HOLD) {
                gpio_put(dev.pinout.HOLD, true);
            }
            if (WP) {
                gpio_put(dev.pinout.WP, true);
            }
            cs_select();
        }
        if (transfer == 1) {
            DMA_Trigger(dma.tx, dma.rx);
        }
        // TODO: Handle spi dma error
        return -!(transfer == 1);
    }

    int start_read(uint8_t *rx, size_t length) {
        return start_transfer(nullptr, rx, length, true);
    }

    int start_write(const uint8_t *tx, size_t length) {
        return start_transfer(tx, nullptr, length, true);
    }

    int wait() {
        // TODO: use dma interrupt instead, especially for cs_deselect
        dma.tx.wait();
        dma.rx.wait();
        // TODO: should we be concerned if one dma finishes before the other?
        return 0;
    }

    int finish(bool blocking = true) {
        int status = transfer;
        if (status != 1) {
            // TODO: will transfer ever be zero at this point? Besides an inital call?
            return status;
        }
        if (blocking) {
            wait();
        }
        cs_deselect();
        if (HOLD) {
            gpio_put(dev.pinout.HOLD, false);
        }
        if (WP) {
            gpio_put(dev.pinout.WP, false);
        }
        transfer = 0;
        return 0;
    }

    int32_t POST() {
        // Force loopback for testing
        // hw_set_bits(&spi_get_hw(spi_default)->cr1, SPI_SSPCR1_LBM_BITS);
        return 0;
    }
};