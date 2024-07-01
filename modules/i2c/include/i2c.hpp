#pragma once

#include <stdint.h>

#include "hardware/clocks.h"
#include "hardware/dma.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/resets.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"
//
#include "test.hpp"

class Interface {
protected:
    bool enabled = false;

public:
    typedef struct {
        char name[32];
    } Device;

    virtual int init() = 0;
    virtual int deinit() = 0;

    inline int reinit() {
        if (deinit())
            return -1;
        return init();
    }
};

namespace I2C {

class I2C : public Interface {
public:
    typedef struct : Interface::Device {
        i2c_inst *i2c;
        uint32_t baudrate;
    } Device;

private:
    const Device *dev;
    uint32_t baudrate;
    int dma_tx, dma_rx;

    inline int unclaim_dma() {
        if (dma_tx >= 0)
            dma_channel_unclaim(dma_tx);
        if (dma_rx >= 0)
            dma_channel_unclaim(dma_rx);
        dma_tx = -1;
        dma_rx = -1;
        return 0;
    }

    inline int claim_dma() {
        if (dma_tx >= 0 && dma_rx >= 0)
            return 1;

        // IMPROVE: Do we need to check if only one dma was set?

        dma_tx = dma_claim_unused_channel(false);
        dma_rx = dma_claim_unused_channel(false);

        if (dma_tx < 0 || dma_rx < 0) {
            if (unclaim_dma() < 0)
                return -2;
            return -1;
        }

        return 0;
    }

public:
    constexpr I2C(const Device *dev) : dev(dev), dma_tx(-1), dma_rx(-1), baudrate(0) {}

    int init() {
        // spi_inst_t *spi = dev->spi;
        // invalid_params_if(SPI, spi != spi0 && spi != spi1); // FIXME: SPI is a #define in spi.c
        // // baudrate = spi_init(spi, dev->baudrate);
        // const uint32_t reset_bits = spi == spi0 ? RESETS_RESET_SPI0_BITS : RESETS_RESET_SPI1_BITS;

        // // Full reset
        // reset_block(reset_bits);
        // unreset_block_wait(reset_bits);

        // baudrate = spi_set_baudrate(spi, baudrate); // IMPROVE: Consider using uint32_t
        // // Use TI's format, supported by the MT29F4G01ABAFD
        // spi_set_format(spi, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);
        // // Always enable DREQ signals -- harmless if DMA is not listening
        // hw_set_bits(&spi_get_hw(spi)->dmacr, SPI_SSPDMACR_TXDMAE_BITS | SPI_SSPDMACR_RXDMAE_BITS);
        // // Finally enable the SPI
        // hw_set_bits(&spi_get_hw(spi)->cr1, SPI_SSPCR1_SSE_BITS);

        // gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
        // gpio_init(PICO_DEFAULT_SPI_CSN_PIN);
        // gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
        // gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);
        // // Make the SPI pins available to picotool
        // bi_decl(bi_3pins_with_func(PICO_DEFAULT_SPI_RX_PIN, PICO_DEFAULT_SPI_TX_PIN, PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI));
        // // Make the CS pin available to picotool
        // bi_decl(bi_1pin_with_name(PICO_DEFAULT_SPI_CSN_PIN, "SPI CS"));

        // if (claim_dma() < 0) {
        //     deinit();
        //     return -2;
        // }

        // enabled = true;
        // return -(baudrate == 0);
        return -1;
    }

    int POST() {
        // Force loopback for testing
        // hw_set_bits(&spi_get_hw(spi_default)->cr1, SPI_SSPCR1_LBM_BITS);
        return 0;
    }

    int deinit() {
        // spi_deinit(dev->spi);
        // baudrate = 0;

        // if (unclaim_dma() < 0)
        //     return -2;

        // enabled = false;
        return 0;
    }

    // inline int reinit() {
    //     if (deinit())
    //         return -1;
    //     return init();
    // }
};

} // namespace I2C
