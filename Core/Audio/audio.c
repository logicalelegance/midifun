/*
 * audio.c
 *
 *  Created on: Jan 29, 2020
 *      Author: Chris
 */


#include "stm32f3xx_hal.h"

extern I2S_HandleTypeDef hi2s3;

#define WAVE_SAMPLES 441

static uint16_t sinwave[WAVE_SAMPLES];

void test2(void)
{
	int i = 0;
	for (; i < WAVE_SAMPLES/2; i++) {
		sinwave[i] = i * 295;
	}

	for (; i < WAVE_SAMPLES; i++) {
		sinwave[i] = (WAVE_SAMPLES - i) * 295;
	}

	for (int i = 0; i < 1000; i++) {
		HAL_I2S_Transmit(&hi2s3, sinwave, WAVE_SAMPLES, 20);
	}
}
