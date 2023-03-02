/**
 * @file
 *//*
 *  Copyright (c) 2019, Cascoda Ltd.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * Application for E-ink display of images.
*/
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cascoda-bm/cascoda_evbme.h"
#include "cascoda-bm/cascoda_sensorif.h"
#include "cascoda-bm/cascoda_serial.h"
#include "cascoda-bm/cascoda_spi.h"
#include "cascoda-bm/cascoda_types.h"
#include "cascoda-bm/cascoda_wait.h"
#include "cascoda-util/cascoda_time.h"
#include "ca821x_api.h"

/* Insert Application-Specific Includes here */
#include "cascoda-bm/test15_4_evbme.h"
#include "sif_ssd1681.h"
#include "sif_ssd1681_image.h"

void partial_display_cycle(void)
{
	uint8_t num_of_cycles = 2;

	SIF_SSD1681_Initialise();
	SIF_SSD1681_DisplayPartBaseImageWhite();

	for (uint8_t i = 0; i < num_of_cycles; ++i)
	{
		SIF_SSD1681_overlay_qr_code("https://www.cascoda.com", waveshare_example_img, 1, 30, 20);
		SIF_SSD1681_SetFrameMemoryPartial(waveshare_example_img);
		SIF_SSD1681_DisplayPartFrame();

		SIF_SSD1681_overlay_qr_code("https://www.somethingelse.com", waveshare_example_img, 1, 30, 20);
		SIF_SSD1681_SetFrameMemoryPartial(waveshare_example_img);
		SIF_SSD1681_DisplayPartFrame();

		SIF_SSD1681_overlay_qr_code("https://www.differentQRCODEE.com", waveshare_example_img, 1, 30, 20);
		SIF_SSD1681_SetFrameMemoryPartial(waveshare_example_img);
		SIF_SSD1681_DisplayPartFrame();

		SIF_SSD1681_overlay_qr_code("https://www.difflol.com", waveshare_example_img, 2, 80, 40);
		SIF_SSD1681_SetFrameMemoryPartial(waveshare_example_img);
		SIF_SSD1681_DisplayPartFrame();

		SIF_SSD1681_overlay_qr_code("https://www.anothre.com", waveshare_example_img, 2, 80, 40);
		SIF_SSD1681_SetFrameMemoryPartial(waveshare_example_img);
		SIF_SSD1681_DisplayPartFrame();
	}

	SIF_SSD1681_DeepSleep();
}

/******************************************************************************/
/***************************************************************************/ /**
 * \brief Main Program Endless Loop
 *******************************************************************************
 * \return Does not return
 *******************************************************************************
 ******************************************************************************/
int main(void)
{
#define IGNORED 1
	struct ca821x_dev dev;
	ca821x_api_init(&dev);
	SENSORIF_SPI_Config(1);

	/* Initialisation of Chip and EVBME */
	/* Returns a Status of CA_ERROR_SUCCESS/CA_ERROR_FAIL for further Action */
	/* in case there is no UpStream Communications Channel available */
	EVBMEInitialise(CA_TARGET_NAME, &dev);

	/* Application-Specific Initialisation Routines */

	// Full display
	SIF_SSD1681_Initialise();
	SIF_SSD1681_ClearDisplay();
	SIF_SSD1681_SetFrameMemory(waveshare_example_img, IGNORED);
	SIF_SSD1681_DisplayFrame();
	SIF_SSD1681_DeepSleep();

	// Partial update
	partial_display_cycle();

	// Full display again
	SIF_SSD1681_Initialise();
	SIF_SSD1681_ClearDisplay();
	SIF_SSD1681_SetFrameMemory(waveshare_example_img, IGNORED);
	SIF_SSD1681_DisplayFrame();
	SIF_SSD1681_DeepSleep();

	// Clear
	WAIT_ms(3000);
	SIF_SSD1681_Initialise();
	SIF_SSD1681_StrongClearDisplay();
	SIF_SSD1681_DeepSleep();

	/* Endless Polling Loop */
	while (1)
	{
		cascoda_io_handler(&dev);
	} /* while(1) */
}
