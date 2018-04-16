/*
 * Copyright (c) 2017, NXP Semiconductor, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of NXP Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
 
/**
 * @file    Practica4.c
 * @brief   Application entry point.
 */
#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MK64F12.h"
#include "fsl_adc16.h"
#include "fsl_dac.h"

#define ADC12_BASE				ADC0
#define ADC12_CHANNEL_GROUP		0U
#define ADC12_USER_CHANNEL		12U

#define AMPLITUDE 				(0.7f)
#define ONE_MINUS_A_SQUARE 		(0.51f)
#define HALF_OFFSET				0x7FF


int main(void)
{

	adc16_config_t adc12_config;
	adc16_channel_config_t adc12_channel_config;
	dac_config_t dac_config;

	static int16_t echo[5000] = {0};
	uint16_t n = 0;
	float x;
	float y;

  	/* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
  	/* Init FSL debug console. */
    BOARD_InitDebugConsole();

    /*******************************************************************************
	 * ADC CONFIG
	 ******************************************************************************/

	ADC16_GetDefaultConfig(&adc12_config);
	adc12_config.clockDivider = kADC16_ClockDivider8;
	ADC16_Init(ADC12_BASE, &adc12_config);

	/* Make sure the software trigger is used. */
	ADC16_EnableHardwareTrigger(ADC12_BASE, false);

#if defined(FSL_FEATURE_ADC16_HAS_CALIBRATION) && FSL_FEATURE_ADC16_HAS_CALIBRATION

#endif /* FSL_FEATURE_ADC16_HAS_CALIBRATION */

	adc12_channel_config.channelNumber = ADC12_USER_CHANNEL;
	adc12_channel_config.enableInterruptOnConversionCompleted = false;

#if defined(FSL_FEATURE_ADC16_HAS_DIFF_MODE) && FSL_FEATURE_ADC16_HAS_DIFF_MODE
	adc12_channel_config.enableDifferentialConversion = false;
#endif /* FSL_FEATURE_ADC16_HAS_DIFF_MODE */

	/*******************************************************************************
	 * SIMPLE DAC CONFIG
	 ******************************************************************************/

	DAC_GetDefaultConfig(&dac_config);
	/* DAC Initialization */
	DAC_Init(DAC0, &dac_config);
	/* Enable output. */
	DAC_Enable(DAC0, true);
	/* Make sure the read pointer to the start. */
	DAC_SetBufferReadPointer(DAC0, 0U);

	/*******************************************************************************
	 * MAIN PROGRAM
	 ******************************************************************************/

	for(;;)
	{

		ADC16_SetChannelConfig(ADC12_BASE, ADC12_CHANNEL_GROUP,
				&adc12_channel_config);

		while (0U
				== (kADC16_ChannelConversionDoneFlag
						& ADC16_GetChannelStatusFlags(ADC12_BASE,
						ADC12_CHANNEL_GROUP)))
		{

		}

		echo[n] = ADC16_GetChannelConversionValue(ADC12_BASE,
		ADC12_CHANNEL_GROUP) - HALF_OFFSET + echo[n];

		x = echo[n] * AMPLITUDE;

		y = (ONE_MINUS_A_SQUARE * echo[n]) - x;

		echo[n] = x;

		n = (n < 5000) ? n + 1 : 0;

		y += HALF_OFFSET;

		/* DAC value to be send */
		DAC_SetBufferValue(DAC0, 0U, (uint16_t) y);
	}

    return 0 ;
}
