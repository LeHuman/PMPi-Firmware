#include <bit>
#include <initializer_list>

#include <stdio.h>

#include "hardware/i2c.h"
#include "pico/binary_info.h"
#include "pico/error.h"
#include "pico/stdlib.h"

namespace I2C {
// I2C reserves some addresses for special purposes. We exclude these from the scan.
// These are any addresses of the form 000 0xxx or 111 1xxx
bool reserved_addr(uint8_t addr) {
    return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

#define CHK_ERR(x)               \
    if (x == PICO_ERROR_GENERIC) \
    puts("FAIL")

void set_data_cmd(io_rw_32 data_cmd) {
    data_cmd = std::byteswap(data_cmd);
    while (!i2c_get_write_available(i2c1))
        tight_loop_contents();
    i2c_get_hw(i2c1)->data_cmd = data_cmd;
}

int test() {
    // Enable UART so we can print status output
    // stdio_init_all();

    // for (size_t i = 0; i < 29; i++) {
    //     gpio_set_function(i, GPIO_FUNC_SIO);
    // }

    // gpio_set_dir_out_masked(0x3fffffffU);
    // gpio_set_mask(0x3fffffffU);

    // This example will use I2C0 on the default SDA and SCL pins (GP4, GP5 on a Pico)
    i2c_init(i2c1, 100 * 1000);
    gpio_set_function(27, GPIO_FUNC_I2C);
    gpio_set_function(26, GPIO_FUNC_I2C);
    // gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    // gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
    // Make the I2C pins available to picotool
    // bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));

    // printf("\nI2C Bus Scan\n");
    // printf("   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");

    int ret;
    uint32_t rxdata;
    int c = 0;
    bool set = false;
    float last = 0;

    while (1) {
        rxdata = 0;
        ret = i2c_write_blocking(i2c1, 0b1001000, reinterpret_cast<uint8_t *>(&rxdata), 1, false);
        CHK_ERR(ret);
        rxdata = 0;
        ret = i2c_read_blocking(i2c1, 0b1001000, reinterpret_cast<uint8_t *>(&rxdata), 2, false);
        CHK_ERR(ret);
        rxdata = std::byteswap(rxdata) >> 4 >> 16;
        float temp = float(rxdata) / 16.0f;
        if (last != temp) {
            printf("%f\n", (last = temp));
        }
        // sleep_ms(250);

        // union {
        //     io_rw_32 raw;
        //     [[gnu::packed]] struct {
        //         uint8_t data : 8;
        //         bool to_read : 1;
        //         bool stop : 1;
        //         bool restart : 1;
        //         bool start : 1;
        //         uint8_t _ : 4;
        //     };
        // } i2c_data_cmd;
        // sizeof(i2c_data_cmd);
        // i2c_data_cmd.raw = 0;

        // i2c1->hw->enable = 0;
        // i2c1->hw->tar = 0b1011000;
        // i2c1->hw->enable = 1;

        // rxdata = 0xFFFFFFFF;
        // i2c_data_cmd.start = true;
        // i2c_data_cmd.to_read = false;
        // i2c_data_cmd.data = 0b11111111;
        // set_data_cmd(i2c_data_cmd.raw);
        // printf("%u %u %u\n", rxdata, i2c1->hw->tx_abrt_source, i2c_data_cmd.raw);
        // i2c_data_cmd.start = false;
        // i2c_data_cmd.data = 0b11111111;
        // set_data_cmd(i2c_data_cmd.raw);
        // printf("%u %u %u\n", rxdata, i2c1->hw->tx_abrt_source, i2c_data_cmd.raw);
        // i2c_data_cmd.start = true;
        // // i2c_data_cmd.stop = true;
        // i2c_data_cmd.to_read = true;
        // i2c_data_cmd.data = 0;
        // set_data_cmd(i2c_data_cmd.raw);
        // sleep_ms(100);
        // printf("%u %u %u\n", rxdata, i2c1->hw->tx_abrt_source, i2c_data_cmd.raw);
        // while (!i2c_get_read_available(i2c1))
        //     tight_loop_contents();
        // rxdata = i2c_get_hw(i2c1)->data_cmd;

        // rxdata = 0xFFFFFFFF;
        // ret = i2c_write_blocking(i2c1, 0b1011000, reinterpret_cast<uint8_t *>(&rxdata), 2, true);
        // CHK_ERR(ret);
        // sleep_ms(12);
        // rxdata = 0;
        // ret = i2c_read_blocking(i2c1, 0b1011000, reinterpret_cast<uint8_t *>(&rxdata), 1, false);
        // CHK_ERR(ret);
        // printf("%u %u %u\n", rxdata, i2c1->hw->tx_abrt_source, i2c_data_cmd.raw);
        // sleep_ms(250);

        // rxdata = 0x02000004U;
        // rxdata = std::byteswap(rxdata);
        // ret = i2c_write_blocking(i2c1, 0b1100000, reinterpret_cast<uint8_t *>(&rxdata), 4, false);
        // CHK_ERR(ret);
        // rxdata = 0;
        // ret = i2c_read_blocking(i2c1, 0b1100000, reinterpret_cast<uint8_t *>(&rxdata), 1, false);
        // CHK_ERR(ret);
        // printf("%u\n", rxdata);
        // sleep_ms(250);

        // if (!(++c % 8)) {
        //     if (set = !set) {
        //         gpio_set_mask(0x3fffffffU & ~(1 << 27 | 1 << 26));
        //     } else {
        //         gpio_clr_mask(0x3fffffffU & ~(1 << 27 | 1 << 26));
        //     }
        // }
    }

    // for (int addr = 0; addr < (1 << 7); ++addr) {
    //     if (addr % 16 == 0) {
    //         printf("%02x ", addr);
    //     }

    //     // Perform a 1-byte dummy read from the probe address. If a slave
    //     // acknowledges this address, the function returns the number of bytes
    //     // transferred. If the address byte is ignored, the function returns
    //     // -1.

    //     // Skip over any reserved addresses.
    //     int ret;
    //     uint8_t rxdata;
    //     if (reserved_addr(addr))
    //         ret = PICO_ERROR_GENERIC;
    //     else
    //         ret = i2c_read_blocking(i2c_default, addr, &rxdata, 1, false);

    //     printf(ret < 0 ? "." : "@");
    //     printf(addr % 16 == 15 ? "\n" : "  ");
    // }
    // printf("Done.\n");
    return 0;
    // #endif
}
} // namespace I2C