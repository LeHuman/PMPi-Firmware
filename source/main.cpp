// Copyright (c) 2020-2022 Cesanta Software Limited
// All rights reserved

#include <FreeRTOS.h>
#include <hardware/watchdog.h>
#include <lwip/ip4_addr.h>
#include <pico/cyw43_arch.h>
#include <pico/stdlib.h>

#include "mongoose.h"
#include "net.h"
#include "task.h"

#define TEST_TASK_PRIORITY (tskIDLE_PRIORITY + 1UL)
#define TEST_TASK_STACK_SIZE ((configSTACK_DEPTH_TYPE)2048)

static struct mg_mgr mgr;

constexpr uint8_t ipv4_digit(uint32_t &ip, int pos) {
    return (ip >> (8 * pos)) & 0xFF;
}

void main_task(__unused void *params) {
    if (cyw43_arch_init()) {
        printf("failed to initialise\n");
        return;
    }
    cyw43_arch_enable_sta_mode();
    printf("Connecting to WiFi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("failed to connect.\n");
        exit(1);
    }

    mg_mgr_init(&mgr);
    web_init(&mgr);

    {
        uint32_t ip = *((uint32_t *)mgr.conns->loc.ip);
        printf("Connected: %u.%u.%u.%u\n", ipv4_digit(ip, 0), ipv4_digit(ip, 1), ipv4_digit(ip, 2), ipv4_digit(ip, 3));
    }

    while (true) {
        mg_mgr_poll(&mgr, 10);
    }

    cyw43_arch_deinit();
}

void print_task(__unused void *params) {
    uint64_t count = 0;
    uint16_t delay = 500;
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(rand() % delay));
        switch (rand() % 5) {
            case 0:
                printf("[ERROR] UGH: %llu\n", count);
                break;
            case 1:
                printf("[WARN] WOOAH: %llu\n", count);
                break;
            case 2:
                printf("Wait, what?! 💀 : %llu\n", count);
                printf("[FATAL] oops ;) : %llu\n", count);
                break;
            case 3:
                printf("[INFO] Look!!!: %llu\n", count);
                break;
            default:
                if (delay && !(count % (505 - delay)))
                    delay -= 50;
                printf("NADA Here: %llu\nYesssir\n", count);
                break;
        }
        count++;
        if (stdio_usb_connected()) {
            static char buf[6];
            int c, i = 0;
            while (((c = getchar_timeout_us(0)) != PICO_ERROR_TIMEOUT) && i < 6) {
                buf[i] = c;
                i++;
                if (!strncmp(buf, "DEBUG", 5)) {
                    printf("SWITCHING TO BOOTLOADER\n");
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
    xTaskCreate(print_task, "PrintThread", TEST_TASK_STACK_SIZE / 2, NULL, tskIDLE_PRIORITY, &task);
    vTaskStartScheduler();
}

int main(void) {
    stdio_init_all();
    vLaunch();

    return 0;
}