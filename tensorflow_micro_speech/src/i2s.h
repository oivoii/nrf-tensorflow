/*
 * Copyright (c) 2015 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef NRFX_I2S_H_
#define NRFX_I2S_H_

#include <zephyr/types.h>

#define I2S_WS_PIN 4
#define I2S_SD_PIN 5
#define I2S_SCK_PIN 6

#define I2S_DATA_BLOCK_WORDS 2048

#ifdef __cplusplus
extern "C" {
#endif

void get_sound_init(void (*handler)() );

void get_sound(void* buffer, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* NRFX_I2S_H_ */
