#pragma once

#include <spi.hpp>
#include <stdint.h>
#include <stdio.h>

namespace Flash {

class SPI {
private:
    ::SPI *spi;

    int read_address(uint8_t addr, uint8_t *buf, uint16_t len) {
        uint8_t msg[2] = {addr, 0};
        int res = spi->start_write(msg, 2);
        res += spi->start_read(buf, len);
        spi->finish();
        return res;
    }

    int read_registers(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len) {
        uint8_t msg[2] = {addr, reg};
        int res = spi->start_write(msg, 2);
        res += spi->start_read(buf, len);
        spi->finish();
        return res;
    }

public:
    constexpr SPI(::SPI *interface) : spi(interface) {}

    int test() {
        spi->set_hold(false);
        spi->set_write_protect(false);
        uint8_t regs[8] = {0};
        this->read_address(0x9F, regs, 2);
        printf("%u &u\n", regs[0], regs[1]);
        this->read_registers(0x0F, 0xC0, regs, 1);
        printf("%u &u\n", regs[0], regs[1]);
        return 0;
    }
};

} // namespace Flash

// int read_address(uint8_t addr, uint8_t *buf, uint16_t len) {
//     uint8_t msg[2] = {addr, 0};
//     spi->set_transfer_write(msg, 2);
//     int res = spi_write_blocking(spi_default, msg, 2);
//     res += spi_read_blocking(spi_default, 0, buf, len);
//     cs_deselect();
//     return res;
// }

// int read_registers(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len) {
//     // uint8_t msg[2] = {addr, reg};
//     // cs_select();
//     // int res = spi_write_blocking(spi_default, msg, 2);
//     // res += spi_read_blocking(spi_default, 0, buf, len);
//     // cs_deselect();
//     // return res;
// }

// int unclaim_dma() {
//     if (dma.tx >= 0) {
//         dma_channel_unclaim(dma.tx);
//     }
//     if (dma.rx >= 0) {
//         dma_channel_unclaim(dma.rx);
//     }
//     dma.tx = -1;
//     dma.rx = -1;
//     return 0;
// }

// int claim_dma() {
//     if (spi == nullptr)
//         return -3;

//     if (dma.tx >= 0 && dma.rx >= 0)
//         return 1;

//     if (dma.tx < 0) {
//         dma.tx = dma_claim_unused_channel(false);
//     }

//     if (dma.rx < 0) {
//         dma.rx = dma_claim_unused_channel(false);
//     }

//     if (dma.tx < 0 || dma.rx < 0) {
//         if (unclaim_dma() < 0)
//             return -2;
//         return -1;
//     }

//     // Configure TX DMA
//     dma_channel_config c = dma_channel_get_default_config(dma.tx);
//     channel_config_set_transfer_data_size(&c, DMA_SIZE_8); // TODO: Are we okay with always doing size 8 for DMA?
//     channel_config_set_dreq(&c, spi_get_dreq(spi, true));

//     channel_config_set_read_increment(&c, true);
//     channel_config_set_write_increment(&c, false);

//     dma_channel_set_read_addr(dma.tx, nullptr, false);               // Read from null, not set here
//     dma_channel_set_write_addr(dma.tx, &spi_get_hw(spi)->dr, false); // Write to SPI hardware
//     dma_channel_set_trans_count(dma.tx, 0, false);                   // Set length to zero, not set here
//     dma_channel_set_config(dma.tx, &c, false);

//     // Configure RX DMA
//     c = dma_channel_get_default_config(dma.rx);
//     channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
//     channel_config_set_dreq(&c, spi_get_dreq(spi, false));

//     channel_config_set_read_increment(&c, false);
//     channel_config_set_write_increment(&c, true);

//     dma_channel_set_read_addr(dma.rx, &spi_get_hw(spi)->dr, false); // Read from SPI hardware
//     dma_channel_set_write_addr(dma.rx, nullptr, false);             // Write to null, not set here
//     dma_channel_set_trans_count(dma.rx, 0, false);                  // Set length to zero, not set here
//     dma_channel_set_config(dma.rx, &c, false);

//     return 0;
// }