#pragma once

#include <atomic>

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
        consteval Device(const char *const name, Instance instance, uint32_t baudrate, Pinout_t pinout) : Common::Device{name}, instance(instance), baudrate(baudrate), pinout(pinout) {
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
    const Device *dev;

private:
    spi_inst_t *spi = nullptr;
    spi_hw_t *spi_hw = nullptr; // spi_get_hw(spi)
    uint32_t reset_bits = 0;

    uint32_t baudrate;
    std::atomic_int transfer = 0;

    // TODO: Better HOLD and WP control
    bool HOLD = false;
    bool WP = false;

    struct {
        DMA::Memory tx;
        DMA::Memory rx;
    } dma;

    constexpr spi_inst_t *get_spi_instance(const Device *dev) const {
        // Establish what SPI instance we are using
        switch (dev->instance) {
            case Instance::SPI_0:
                return spi0; // BROKEN: error: 'reinterpret_cast' is not a constant expression
            case Instance::SPI_1:
                return spi1;
            default:
                consteval_assert(false, "Invalid spi instance chosen.");
                return spi0;
        }
    }
    constexpr spi_hw_t *get_spi_hw_instance(const Device *dev) const {
        switch (dev->instance) {
            case Instance::SPI_0:
                return spi0_hw;
            case Instance::SPI_1:
                return spi1_hw;
            default:
                consteval_assert(false, "Invalid spi instance chosen.");
                return spi0_hw;
        }
    }

    constexpr uint32_t get_spi_reset_bits(const Device *dev) const {
        switch (dev->instance) {
            case Instance::SPI_0:
                return RESETS_RESET_SPI0_BITS;
            case Instance::SPI_1:
                return RESETS_RESET_SPI1_BITS;
            default:
                consteval_assert(false, "Invalid spi instance chosen.");
                return RESETS_RESET_SPI0_BITS;
        }
    }

    constexpr uint get_spi_dreq(spi_inst_t *spi, bool is_tx) {
        static_assert(DREQ_SPI0_RX == DREQ_SPI0_TX + 1, "");
        static_assert(DREQ_SPI1_RX == DREQ_SPI1_TX + 1, "");
        static_assert(DREQ_SPI1_TX == DREQ_SPI0_TX + 2, "");
        return DREQ_SPI0_TX + spi_get_index(spi) * 2 + !is_tx;
    }

    inline void cs_select(void) const {
        asm volatile("nop \n nop \n nop");
        gpio_put(dev->pinout.CS, 0); // Active low
        asm volatile("nop \n nop \n nop");
    }

    inline void cs_deselect(void) {
        asm volatile("nop \n nop \n nop");
        gpio_put(dev->pinout.CS, 1);
        asm volatile("nop \n nop \n nop");
    }

public:
    SPI(const Device *dev) : dev(dev), spi(get_spi_instance(dev)), reset_bits(get_spi_reset_bits(dev)), dma({.tx = {&spi_get_hw(spi)->dr, spi_get_dreq(spi, true)}, .rx = {&spi_get_hw(spi)->dr, spi_get_dreq(spi, false)}}), baudrate(0) {}
    // consteval SPI(const Device *dev) : dev(dev), spi(get_spi_instance(dev)), spi_hw(get_spi_hw_instance(dev)), reset_bits(get_spi_reset_bits(dev)), dma({.tx = {&spi_hw->dr, get_spi_dreq(spi, true)}, .rx = {&spi_hw->dr, get_spi_dreq(spi, false)}}), baudrate(0) {
    // switch (dev->instance) {
    //     case Instance::SPI_0:
    //     case Instance::SPI_1:
    //         break;
    //     default:
    //         consteval_assert(false, "Invalid spi instance chosen.");
    //         break;
    // }
    // }

    int init() {
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
        if (dev->pinout.HOLD >= 0) {
            gpio_set_dir(dev->pinout.HOLD, GPIO_OUT);
            gpio_put(dev->pinout.HOLD, true);
            gpio_set_function(dev->pinout.HOLD, GPIO_FUNC_SIO);
            HOLD = true;
        }

        if (dev->pinout.WP >= 0) {
            gpio_set_dir(dev->pinout.WP, GPIO_OUT);
            gpio_put(dev->pinout.WP, false);
            gpio_set_function(dev->pinout.WP, GPIO_FUNC_SIO);
            WP = true;
        }

        // Full reset
        reset_block(reset_bits);
        unreset_block_wait(reset_bits);
        // Attempt to set baudrate
        baudrate = spi_set_baudrate(spi, dev->baudrate); // TODO: What do we do if baudrate does not match or is not similar?
        // Use TI's format, supported by the MT29F4G01ABAFD
        spi_set_format(spi, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);
        // Always enable DREQ signals -- harmless if DMA is not listening
        hw_set_bits(&spi_hw->dmacr, SPI_SSPDMACR_TXDMAE_BITS | SPI_SSPDMACR_RXDMAE_BITS);
        // Finally enable the SPI
        hw_set_bits(&spi_hw->cr1, SPI_SSPCR1_SSE_BITS);

        gpio_set_dir(dev->pinout.CS, GPIO_OUT);
        gpio_put(dev->pinout.CS, 1);
        gpio_set_function(dev->pinout.CS, GPIO_FUNC_SIO);
        gpio_set_function(dev->pinout.Rx, GPIO_FUNC_SPI);
        gpio_set_function(dev->pinout.Tx, GPIO_FUNC_SPI);
        gpio_set_function(dev->pinout.SCK, GPIO_FUNC_SPI);

        _enabled = true;
        return -(baudrate == 0);
    }

    int deinit() {
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
        HOLD = (dev->pinout.HOLD >= 0) && use_with_transfers;
        return HOLD;
    }

    bool set_write_protect(bool use_with_transfers) {
        WP = (dev->pinout.WP >= 0) && use_with_transfers;
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
                gpio_put(dev->pinout.HOLD, true);
            }
            if (WP) {
                gpio_put(dev->pinout.WP, true);
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
            gpio_put(dev->pinout.HOLD, false);
        }
        if (WP) {
            gpio_put(dev->pinout.WP, false);
        }
        transfer = 0;
        return 0;
    }

    int POST() {
        // Force loopback for testing
        // hw_set_bits(&spi_get_hw(spi_default)->cr1, SPI_SSPCR1_LBM_BITS);
        return 0;
    }
};