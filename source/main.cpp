// Copyright (c) 2020-2022 Cesanta Software Limited
// All rights reserved

#include <cstddef>
#include <cstdint>

#include <FreeRTOS.h>
#include <hardware/watchdog.h>
#include <lwip/ip4_addr.h>
#include <pico/cyw43_arch.h>
#include <pico/stdlib.h>
#include <portability.h>
#include <test.hpp>

#include "flash/spi.hpp"
#include "gpib.hpp"
#include "log.hpp"
#include "mongoose.h"
#include "net.h"
#include "spi.hpp"
#include "task.h"
//
#include "ff_headers.h"
#include "ff_sddisk.h"
#include "ff_stdio.h"
#include "ff_utils.h"
#include "hw_config.h"

#define TEST_TASK_PRIORITY (tskIDLE_PRIORITY + 1UL)
#define TEST_TASK_STACK_SIZE ((configSTACK_DEPTH_TYPE)2048)

using namespace Log;

static inline void stop() {
    fflush(stdout);
    __breakpoint();
}

// See https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_FAT/Standard_File_System_API.html

static void SimpleTask(void *arg) {
    (void)arg;

    logme(Level::NOTICE, pcTaskGetName(NULL), "Hello, world!");

    FF_Disk_t *pxDisk = FF_SDDiskInit("sd0");
    configASSERT(pxDisk);
    FF_Error_t xError = FF_SDDiskMount(pxDisk);
    if (FF_isERR(xError) != pdFALSE) {
        FF_PRINTF("FF_SDDiskMount: %s\n",
                  (const char *)FF_GetErrMessage(xError));
        stop();
    }
    FF_FS_Add("/sd0", pxDisk);

    FF_FILE *pxFile = ff_fopen("/sd0/filename.txt", "a");
    if (!pxFile) {
        FF_PRINTF("ff_fopen failed: %s (%d)\n", strerror(stdioGET_ERRNO()), stdioGET_ERRNO());
        stop();
    }
    if (ff_fprintf(pxFile, "Hello, world!\n") < 0) {
        FF_PRINTF("ff_fprintf failed: %s (%d)\n", strerror(stdioGET_ERRNO()), stdioGET_ERRNO());
        stop();
    }
    if (-1 == ff_fclose(pxFile)) {
        FF_PRINTF("ff_fclose failed: %s (%d)\n", strerror(stdioGET_ERRNO()), stdioGET_ERRNO());
        stop();
    }
    FF_FS_Remove("/sd0");
    FF_Unmount(pxDisk);
    FF_SDDiskDelete(pxDisk);
    puts("Goodbye, world!");

    vTaskDelete(NULL);
}

// static struct mg_mgr mgr;

constexpr uint8_t ipv4_digit(uint32_t &ip, int pos) {
    return (ip >> (8 * pos)) & 0xFF;
}

void main_task(__unused void *params) {

    while (!stdio_usb_connected()) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    vTaskDelay(pdMS_TO_TICKS(250));

    puts("Hello");

    int32_t test_results = Test::baseline_test();

    logme(Level::INFO, pcTaskGetName(NULL), "Test Results: %i", test_results);

    //     if (cyw43_arch_init()) {
    //         printf("failed to initialise\n");
    //         return;
    //     }
    //     cyw43_arch_enable_sta_mode();
    //     printf("Connecting to WiFi...\n");
    //     if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
    //         printf("failed to connect.\n");
    //         exit(1);
    //     }

    //     mg_mgr_init(&mgr);
    //     web_init(&mgr);

    //     {
    //         uint32_t ip = *((uint32_t *)mgr.conns->loc.ip);
    //         printf("Connected: %u.%u.%u.%u\n", ipv4_digit(ip, 0), ipv4_digit(ip, 1), ipv4_digit(ip, 2), ipv4_digit(ip, 3));
    //     }

    //     while (true) {
    //         mg_mgr_poll(&mgr, 10);
    //     }

    //     cyw43_arch_deinit();
    vTaskDelete(NULL);
}

void print_task(__unused void *params) {
    uint64_t count = 0;
    uint16_t delay = 500;
    const char *const me = pcTaskGetName(NULL);

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(rand() % delay));
        switch (rand() % 5) {
            case 0:
                logme(Level::ERROR, me, "UGH: %llu", count);
                break;
            case 1:
                logme(Level::WARNING, me, "WOOAH: %llu", count);
                break;
            case 2:
                logme(Level::INFO, me, "Wait, what?! 💀 : %llu", count);
                logme(Level::FATAL, me, "oops ;) : %llu", count);
                break;
            case 3:
                logme(Level::INFO, me, "Look!!!: %llu", count);
                break;
            default:
                if (delay && !(count % (505 - delay)))
                    delay -= 50;
                logme(Level::TRACE, me, "NADA Here: %llu\tYesssir", count);
                break;
        }
        // printf("MASK: %ul\n", ctrl.mask_dio);
        count++;
        if (stdio_usb_connected()) {
            static char buf[6];
            int c, i = 0;
            while (((c = getchar_timeout_us(0)) != PICO_ERROR_TIMEOUT) && i < 6) {
                buf[i] = c;
                i++;
                if (!strncmp(buf, "DEBUG", 5)) {
                    logme(Level::WARNING, me, "SWITCHING TO BOOTLOADER");
                    stdio_flush();

                    watchdog_hw->scratch[0] = 1;
                    watchdog_reboot(0, 0, 1);
                    while (1) {
                    }
                }
            }
        }
    }
}

void vLaunch(void) {
    TaskHandle_t task;
    xTaskCreate(main_task, "TestMainThread", TEST_TASK_STACK_SIZE, NULL, TEST_TASK_PRIORITY, &task);
    xTaskCreate(print_task, "PrintThread0", TEST_TASK_STACK_SIZE / 2, NULL, tskIDLE_PRIORITY, &task);
    xTaskCreate(print_task, "PrintThread1", TEST_TASK_STACK_SIZE / 2, NULL, tskIDLE_PRIORITY, &task);
    vTaskStartScheduler();
}

constexpr GPIB::PinOut gpib_pinout{
    {
        .DIO1 = 0,
        .DIO2 = 1,
        .DIO3 = 2,
        .DIO4 = 3,
        .DIO5 = 4,
        .DIO6 = 5,
        .DIO7 = 6,
        .DIO8 = 7,

        .REN = 8,
        .EOI = 9,
        .IFC = 13,
        .SRQ = 14,
        .ATN = 15,

        .DAV = 10,
        .NRFD = 11,
        .NDAC = 12,
    },
};

GPIB::Controller ctrl(gpib_pinout, GPIB::default_config);
// constexpr GPIB::Controller ctrl();

static constinit SPI::Device spi_flash_device{
    "Main Flash",
    SPI::Instance::SPI_0,
    133 * 1000 * 1000,
    {
        .Tx = 19,
        .Rx = 16,
        .HOLD = 20,
        .WP = 21,
        .SCK = 18,
        .CS = 17,
    },
};
static SPI spi_flash_interface(spi_flash_device);
static Flash::SPI main_flash(&spi_flash_interface);

int main() {
    timer_hw->dbgpause = 0;
    bool status = stdio_init_all();

    logit(Level::NOTICE, "PMPi Booting!");

    // spi_flash_interface.init();
    // main_flash.test();

    vLaunch();

    while (1) {
    }

    return 0;
}
