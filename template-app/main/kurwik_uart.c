#include "kurwik_uart.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "driver/uart.h"

#define CLAMP(x, min, max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))

// static const char TAG[] = "kurwik_uart";

void kurwik_uart_init(void)
{
    const uart_port_t uart_num = UART_NUM_2;
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
    };
    ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));

    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_2, 4, 3, -1, -1));

    const int uart_buffer_size = (1024 * 2);
    QueueHandle_t uart_queue;
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_2, uart_buffer_size, uart_buffer_size, 10, &uart_queue, 0));
}

void kurwik_uart_send(int is_detected, int x, int y, int area)
{
    char buf[64];
    int sum = is_detected + x + y + area;
    snprintf(buf, sizeof(buf), "%d %04d %04d %06d %06d\n", is_detected, x, y, area, sum);
    uart_write_bytes(UART_NUM_2, buf, strlen(buf));
}
