// Copyright (c) 2020-2022 Cesanta Software Limited
// All rights reserved

#include <FreeRTOS.h>
#include <lwip/ip4_addr.h>
#include <pico/cyw43_arch.h>
#include <pico/stdlib.h>

extern "C" {
#include "net.h"
}
#include "mongoose.h"
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
        auto ip = mgr.conns->loc.ip;
        printf("Connected: %u.%u.%u.%u\n", ipv4_digit(ip, 0), ipv4_digit(ip, 1), ipv4_digit(ip, 2), ipv4_digit(ip, 3));
    }

    while (true) {
        mg_mgr_poll(&mgr, 10);
    }

    cyw43_arch_deinit();
}

void print_task(__unused void *params) {
    while (true) {
        vTaskDelay(500 / portTICK_PERIOD_MS);
        printf("Heyo!\n");
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