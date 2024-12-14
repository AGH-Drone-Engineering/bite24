#ifndef __GUPIDETEKTOR_H__
#define __GUPIDETEKTOR_H__

#ifdef __cplusplus
#include <cstdint>
extern "C" {
#else
#include <stdint.h>
#endif

void gupidetektor_init();

void gupidetektor_detect(const uint8_t *image, int width, int height, int *x);

#ifdef __cplusplus
}
#endif

#endif
