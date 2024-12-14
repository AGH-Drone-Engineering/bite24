#ifndef __KURWIK_UART_H__
#define __KURWIK_UART_H__

#ifdef __cplusplus
extern "C" {
#endif

void kurwik_uart_init(void);

void kurwik_uart_send(int is_detected, int x, int y, int area);

#ifdef __cplusplus
}
#endif

#endif
