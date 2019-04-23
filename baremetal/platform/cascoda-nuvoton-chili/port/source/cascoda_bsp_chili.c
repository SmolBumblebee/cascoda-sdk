/*
 * Copyright (C) 2019  Cascoda, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/*
 * Cascoda Interface to Vendor BSP/Library Support Package.
 * MCU:    Nuvoton Nano120
 * MODULE: Chili 1 (1.2, 1.3)
 * Interface Functions, see cascoda-bm/cascoda_interface.h for descriptions
*/
/* System */
#include <stdio.h>
/* Platform */
#include "Nano100Series.h"
#include "adc.h"
#include "gpio.h"
#include "spi.h"
#include "sys.h"
#include "timer.h"
#include "uart.h"
#include "usbd.h"
/* Cascoda */
#include "cascoda-bm/cascoda_evbme.h"
#include "cascoda-bm/cascoda_interface.h"
#include "cascoda-bm/cascoda_spi.h"
#include "cascoda-bm/cascoda_time.h"
#include "cascoda-bm/cascoda_types.h"
#include "ca821x_api.h"
#include "cascoda_chili.h"
#ifdef USE_USB
#include "cascoda-bm/cascoda_usbhid.h"
#include "cascoda_chili_usb.h"
#endif /* USE_USB */
#ifdef USE_DEBUG
#include "cascoda_debug_chili.h"
#endif /* USE_DEBUG */

/******************************************************************************/
/****** Global Variables                                                 ******/
/******************************************************************************/
volatile u8_t WDTimeout        = 0;
volatile u8_t USBPresent       = 1;
volatile u8_t UseExternalClock = 0;

volatile u8_t asleep = 0;

struct device_link device_list[NUM_DEVICES];

/******************************************************************************/
/***************************************************************************/ /**
 * \brief See cascoda-bm/cascoda_interface.h
 *******************************************************************************
 ******************************************************************************/
void BSP_WaitUs(u32_t us)
{
	volatile u32_t t1;
	volatile u32_t t2;

	/* Note this is dependent on the TIMER0 prescaling in CHILI_TimersInit() */
	/* as it is pre-scaled to 1 us and counting to 1 ms, max. wait time is 1000 us */
	if (us >= 1000)
		return;
	t1 = TIMER0->DR;
	do
	{
		t2 = TIMER0->DR;
		if (t1 > t2)
			t2 += 1000;
	} while ((t2 - t1) < us);
}

/******************************************************************************/
/***************************************************************************/ /**
 * \brief See cascoda-bm/cascoda_interface.h
 *******************************************************************************
 ******************************************************************************/
void BSP_ResetRF(u8_t ms)
{
	u32_t ticks;

	ticks = (u32_t)ms;

	/* reset board with RSTB */
	PA11 = 0; /* RSTB is LOW */
	if (ticks == 0)
		TIME_WaitTicks(10);
	else
		TIME_WaitTicks(ticks);
	PA11 = 1;
	TIME_WaitTicks(50);
}

/******************************************************************************/
/***************************************************************************/ /**
 * \brief See cascoda-bm/cascoda_interface.h
 *******************************************************************************
 ******************************************************************************/
u8_t BSP_SenseRFIRQ(void)
{
	return PA10;
}

/******************************************************************************/
/***************************************************************************/ /**
 * \brief See cascoda-bm/cascoda_interface.h
 *******************************************************************************
 ******************************************************************************/
void BSP_DisableRFIRQ()
{
	/* note that this is temporarily disabling all GPIO A/B/C ports */
	NVIC_DisableIRQ(GPABC_IRQn);
	__ASM volatile("" : : : "memory");
}

/******************************************************************************/
/***************************************************************************/ /**
 * \brief See cascoda-bm/cascoda_interface.h
 *******************************************************************************
 ******************************************************************************/
void BSP_EnableRFIRQ()
{
	/* note that this is re-enabling all GPIO A/B/C ports */
	NVIC_EnableIRQ(GPABC_IRQn);
	__ASM volatile("" : : : "memory");
}

/******************************************************************************/
/***************************************************************************/ /**
 * \brief See cascoda-bm/cascoda_interface.h
 *******************************************************************************
 ******************************************************************************/
void BSP_SetRFSSBHigh(void)
{
	PB3 = 1;
}

/******************************************************************************/
/***************************************************************************/ /**
 * \brief See cascoda-bm/cascoda_interface.h
 *******************************************************************************
 ******************************************************************************/
void BSP_SetRFSSBLow(void)
{
	PB3 = 0;
}

/******************************************************************************/
/***************************************************************************/ /**
 * \brief See cascoda-bm/cascoda_interface.h
 *******************************************************************************
 ******************************************************************************/
void BSP_EnableSerialIRQ(void)
{
#if defined(USE_USB)
	NVIC_EnableIRQ(USBD_IRQn);
#elif defined(USE_UART)
	NVIC_EnableIRQ(UART1_IRQn);
#endif
	__ASM volatile("" : : : "memory");
}

/******************************************************************************/
/***************************************************************************/ /**
 * \brief See cascoda-bm/cascoda_interface.h
 *******************************************************************************
 ******************************************************************************/
void BSP_DisableSerialIRQ(void)
{
#if defined(USE_USB)
	NVIC_DisableIRQ(USBD_IRQn);
#elif defined(USE_UART)
	NVIC_DisableIRQ(UART1_IRQn);
#endif
	__ASM volatile("" : : : "memory");
}

#if defined(USE_USB)

/******************************************************************************/
/***************************************************************************/ /**
 * \brief See cascoda-bm/cascoda_interface.h
 *******************************************************************************
 ******************************************************************************/
void BSP_USBSerialWrite(u8_t *pBuffer)
{
	int i = 0;
	if (BSP_IsUSBPresent())
	{
		while (!USB_Transmit(pBuffer))
		{
			TIME_WaitTicks(2);
			if (i++ > USB_TX_RETRIES)
			{
				USB_SetConnectedFlag(0);
				break;
			}
		}
	}
}

#endif /* USE_USB */

#if defined(USE_UART)

/******************************************************************************/
/***************************************************************************/ /**
 * \brief See cascoda-bm/cascoda_interface.h
 *******************************************************************************
 ******************************************************************************/
void BSP_SerialWriteAll(u8_t *pBuffer, u32_t BufferSize)
{
	UART_Write(UART1, pBuffer, BufferSize);
}

/******************************************************************************/
/***************************************************************************/ /**
 * \brief See cascoda-bm/cascoda_interface.h
 *******************************************************************************
 ******************************************************************************/
u32_t BSP_SerialRead(u8_t *pBuffer, u32_t BufferSize)
{
	u32_t numBytes = 0;

	/* Note that this is basically redefining the vendor BSP function UART_Read() */
	/* which cannot read a previously unknown number of bytes. */

	while ((!(UART1->FSR & UART_FSR_RX_EMPTY_F_Msk)) && (numBytes < BufferSize)) /* Rx FIFO empty ? */
	{
		pBuffer[numBytes] = UART1->RBR;
		++numBytes;
	}

	return (numBytes);
}

#endif /* USE_UART */

/******************************************************************************/
/***************************************************************************/ /**
 * \brief See cascoda-bm/cascoda_interface.h
 *******************************************************************************
 ******************************************************************************/
u64_t BSP_GetUniqueId(void)
{
	u64_t rval = 0;
	u32_t extra_byte;

	SYS_UnlockReg();
	FMC_Open();
	extra_byte = FMC_ReadUID(2);
	rval       = FMC_ReadUID(0) ^ extra_byte;
	rval       = rval << 32ULL;
	rval |= FMC_ReadUID(1) ^ extra_byte;
	FMC_Close();
	SYS_LockReg();

	return rval;
}

/******************************************************************************/
/***************************************************************************/ /**
 * \brief See cascoda-bm/cascoda_interface.h
 *******************************************************************************
 ******************************************************************************/
u8_t BSP_GetChargeStat(void)
{
	u8_t charging;

	/* CHARGE_STAT Input, MCP73831 STAT Output */
	/* High (Hi-Z):  Charging Complete, Shutdown or no Battery present */
	/* Low:          Charging / Preconditioning */

	if (PA12 == 0)
		charging = 1;
	else
		charging = 0;

	return (charging);
}

/******************************************************************************/
/***************************************************************************/ /**
 * \brief See cascoda-bm/cascoda_interface.h
 *******************************************************************************
 ******************************************************************************/
i32_t BSP_GetTemperature(void)
{
	u32_t adcval;
	i32_t tempval;

	/* enable temperature sensor */
	SYS_UnlockReg();
	SYS->TEMPCTL = 1;
	SYS_LockReg();

	/* ADC value for channel = 14 (temperature sensor), reference = internal 1.8V */
	adcval = CHILI_ADCConversion(14, ADC_REFSEL_INT_VREF);

	/* disable temperature sensor */
	SYS_UnlockReg();
	SYS->TEMPCTL = 0;
	SYS_LockReg();

	/* Tenths of a degree celcius */
	/* Vadc         = adcval / 4096 * 1.8 = -1.73e-3 * T ['C] + 0.74 */
	/* => T ['C]    = (0.74 - N*1.8/4096)/1.73e-3 */
	/* => T ['C/10] = (0.74 - N*1.8/4096)/1.73e-4 */
	/*              = (671561 - N*400)/157 */
	tempval = (671561 - (adcval * 400)) / 157;

	return tempval;
}

/******************************************************************************/
/***************************************************************************/ /**
 * \brief See cascoda-bm/cascoda_interface.h
 *******************************************************************************
 ******************************************************************************/
u32_t BSP_ADCGetVolts(void)
{
	u32_t voltsval;

	PB12 = 1; /* VOLTS_TEST high */

	/* ADC value for channel = 0, reference = AVDD 3.3V */
	voltsval = CHILI_ADCConversion(0, ADC_REFSEL_POWER);

	PB12 = 0; /* VOLTS_TEST low */

	return (voltsval);
}

/******************************************************************************/
/***************************************************************************/ /**
 * \brief See cascoda-bm/cascoda_interface.h
 *******************************************************************************
 ******************************************************************************/
void BSP_LEDSigMode(u8_t mode)
{
	/* Mode   LEDs     BlinkIntervall  Comments */
	/* 0        (G)    On              Clear R; G as is */
	/* 1       R       On              Set R, Clear G */
	/* 2      (R)G     On              R as is, Set G */
	/* 3      (R)G     50/50           R as is, Set G Blinking 50/50 */
	/* 4       R G     1/1000          Set R, Set G, Blinking 1/1000 */
	/* 5         G     1/1000          Clear R, Set G, Blinking 1/1000 */
	/* 6               On              Clear all (R, G) */
	/* 7        RG     100             R and G blinking alternatively 100/100 */
	/* 8        RG     35              R and G blinking alternatively 35/35 */
	/* 9        RG     On              All on */
	/* Note that On/Off times are in [10ms] */

	if (mode == 0)
	{
		CHILI_LED_ClrRED();
	}
	else if (mode == 1)
	{
		CHILI_LED_SetRED(1, 0);
		CHILI_LED_ClrGREEN();
	}
	else if (mode == 2)
	{
		CHILI_LED_SetGREEN(1, 0);
	}
	else if (mode == 3)
	{
		CHILI_LED_SetGREEN(50, 50);
	}
	else if (mode == 4)
	{
		CHILI_LED_SetRED(1, 1000);
		CHILI_LED_SetGREEN(1, 1000);
	}
	else if (mode == 5)
	{
		CHILI_LED_ClrRED();
		CHILI_LED_SetGREEN(1, 1000);
	}
	else if (mode == 6)
	{
		CHILI_LED_ClrRED();
		CHILI_LED_ClrGREEN();
	}
	else if (mode == 7)
	{
		CHILI_LED_SetRED(100, 100);
		TIME_WaitTicks(1000);
		CHILI_LED_SetGREEN(100, 100);
	}
	else if (mode == 8)
	{
		CHILI_LED_SetRED(35, 35);
		TIME_WaitTicks(350);
		CHILI_LED_SetGREEN(35, 35);
	}
	else
	{
		CHILI_LED_SetRED(1, 0);
		CHILI_LED_SetGREEN(1, 0);
	}
}

/******************************************************************************/
/***************************************************************************/ /**
 * \brief See cascoda-bm/cascoda_interface.h
 *******************************************************************************
 ******************************************************************************/
void BSP_SPIInit(void)
{
	/* Unlock protected registers */
	SYS_UnlockReg();
	CLK_EnableModuleClock(SPI1_MODULE);
	/* Lock protected registers */
	SYS_LockReg();

	/* SPI clock rate */
	SPI_Open(SPI1, SPI_MASTER, SPI_MODE_2, 8, FCLK_SPI);

	/* Disable Auto-Slave-Select, SSB controlled by GPIO */
	SPI_DisableAutoSS(SPI1);
	BSP_SetRFSSBHigh();

	SPI_EnableFIFO(SPI1, 7, 7);
}

/******************************************************************************/
/***************************************************************************/ /**
 * \brief See cascoda-bm/cascoda_interface.h
 *******************************************************************************
 ******************************************************************************/
u8_t BSP_SPIExchangeByte(u8_t OutByte)
{
	u8_t InByte;
	while (!BSP_SPIPushByte(OutByte))
		;
	while (!BSP_SPIPopByte(&InByte))
		;
	return InByte;
}

/******************************************************************************/
/***************************************************************************/ /**
 * \brief See cascoda-bm/cascoda_interface.h
 *******************************************************************************
 ******************************************************************************/
u8_t BSP_SPIPushByte(u8_t OutByte)
{
	if (!SPI_GET_TX_FIFO_FULL_FLAG(SPI1) && (SPI_GET_RX_FIFO_COUNT(SPI1) < 7))
	{
		SPI_WRITE_TX0(SPI1, OutByte);
		return 1;
	}
	return 0;
}

/******************************************************************************/
/***************************************************************************/ /**
 * \brief See cascoda-bm/cascoda_interface.h
 *******************************************************************************
 ******************************************************************************/
u8_t BSP_SPIPopByte(u8_t *InByte)
{
	if (!SPI_GET_RX_FIFO_EMPTY_FLAG(SPI1))
	{
		*InByte = SPI_READ_RX0(SPI1) & 0xFF;
		return 1;
	}
	return 0;
}

/******************************************************************************/
/***************************************************************************/ /**
 * \brief See cascoda-bm/cascoda_interface.h
 *******************************************************************************
 ******************************************************************************/
void BSP_PowerDown(u32_t sleeptime_ms, u8_t use_timer0)
{
	u8_t lxt_connected = 0;
	u8_t use_watchdog  = 0;

	/* Note: GPIO wake-up only on falling edge, rising edge (IIE) does not perform wake-up contrary to datasheet */
	/*       This means that USBPresent cannot currently wake-up Nano120 as it is active high */
	/* Wake-Up Conditions: */
	/* - Timeout Timer (Timer0 or CAX Sleep Timer) (if set) */
	/* - Falling Edge USBPresent            (PB.15) (Rev. 1.2 or lower) */
	/*                                      (PC.7)  (Rev. 1.3 or higher) */
	/* - Falling Edge SW2                   (PA.9) */
	/* - CAX Wakeup / Falling Edge SPI IRQ  (PA.10) */

	BSP_DisableUSB();

	/* Disable GPIO - tristate the CAx if it is powered off */
	CHILI_GPIOPowerDown(!use_timer0);

	/* Disable HIRC calibration */
	CHILI_DisableIntOscCal();

	if (USE_WATCHDOG_POWEROFF && !use_timer0)
		use_watchdog = 1;

	/* use LF 32.768 kHz Crystal LXT for timer if present, switch off if not */
	if ((CLK->CLKSTATUS & CLK_CLKSTATUS_LXT_STB) && use_timer0)
	{
		lxt_connected = 1;
		/* Note: LIRC cannot be powered down as used for GPIOs */
	}
	else
	{
		lxt_connected = 0;
		SYS_UnlockReg();
		CLK->PWRCTL &= ~(CLK_PWRCTL_LXT_EN);
		SYS_LockReg();
	}

	/* reconfig GPIO debounce clock to LIRC */
	GPIO_SET_DEBOUNCE_TIME(GPIO_DBCLKSRC_IRC10K, GPIO_DBCLKSEL_1);

	/* enable wake-up for USBPresent PB.15/PC.7 */
	GPIO_DisableInt(USBP_PORT, USBP_PIN);
	GPIO_EnableInt(USBP_PORT, USBP_PIN, GPIO_INT_RISING);
	/* enable wake-up for SW2 PA.9 */
	GPIO_DisableInt(PA, 9);
	GPIO_EnableInt(PA, 9, GPIO_INT_FALLING);
	/* enable wake-up for SPI IRQ PA.10 */
	GPIO_DisableInt(PA, 10);
	GPIO_EnableInt(PA, 10, GPIO_INT_FALLING);

	TIMER_Stop(TIMER2);
	TIMER2->CTL |= TIMER_CTL_SW_RST_Msk;
	TIMER_DisableInt(TIMER2);
	NVIC_DisableIRQ(TMR2_IRQn);

	TIMER_Stop(TIMER0);
	TIMER0->CTL |= TIMER_CTL_SW_RST_Msk;
	while (TIMER0->CTL & TIMER_CTL_SW_RST_Msk)
		;

	if (use_timer0 || use_watchdog)
	{
		/*  1s period - 1Hz */
		TIMER_Open(TIMER0, TIMER_ONESHOT_MODE, 1);
		TIMER_EnableInt(TIMER0);
		TIMER_EnableWakeup(TIMER0);
		if (use_watchdog)
		{
			/* always uses 10 kHz LIRC clock, 10 kHz Clock: prescaler 9 gives 1 ms units */
			WDTimeout = 0;
			TIMER_SET_PRESCALE_VALUE(TIMER0, 9);
			TIMER_SET_CMP_VALUE(TIMER0, 2 * sleeptime_ms);
		}
		else if (lxt_connected)
		{
			if (sleeptime_ms < 1000)
			{
				/* 32.768 kHz Clock: prescaler 32 gives roughly 1 ms units */
				TIMER_SET_PRESCALE_VALUE(TIMER0, 32);
				TIMER_SET_CMP_VALUE(TIMER0, sleeptime_ms);
			}
			else
			{
				/* 32.768 kHz Clock: prescaler 255 gives 7.8125 ms units, so 128 is 1 sec */
				TIMER_SET_PRESCALE_VALUE(TIMER0, 255);
				TIMER_SET_CMP_VALUE(TIMER0, (128 * sleeptime_ms) / 1000);
			}
		}
		else
		{
			/*  10 kHz Clock: prescaler 9 gives 1 ms units */
			TIMER_SET_PRESCALE_VALUE(TIMER0, 9);
			TIMER_SET_CMP_VALUE(TIMER0, sleeptime_ms);
		}
		TIMER0->CTL = TIMER_ONESHOT_MODE;
		NVIC_EnableIRQ(TMR0_IRQn);
		TIMER_EnableInt(TIMER0);
		TIMER_EnableWakeup(TIMER0);
		if (use_watchdog)
			CLK->CLKSEL1 = (CLK->CLKSEL1 & ~CLK_CLKSEL1_TMR0_S_Msk) | CLK_CLKSEL1_TMR0_S_LIRC;
		else if (lxt_connected)
			CLK->CLKSEL1 = (CLK->CLKSEL1 & ~CLK_CLKSEL1_TMR0_S_Msk) | CLK_CLKSEL1_TMR0_S_LXT;
		else
			CLK->CLKSEL1 = (CLK->CLKSEL1 & ~CLK_CLKSEL1_TMR0_S_Msk) | CLK_CLKSEL1_TMR0_S_LIRC;
		TIMER_Start(TIMER0);
	}

	asleep = 1;
	SYS_UnlockReg();

	CLK->PWRCTL |= (CLK_PWRCTL_PWRDOWN_EN | CLK_PWRCTL_DELY_EN); /* Set power down bit */
	SCB->SCR |= 0x04;                                            /* Sleep Deep */

	__WFI(); /* really enter power down here !!! */
	__DSB();
	__ISB();

	SYS_LockReg();

	TIMER_Stop(TIMER0);
	TIMER0->CTL |= TIMER_CTL_SW_RST_Msk;

	GPIO_SET_DEBOUNCE_TIME(GPIO_DBCLKSRC_HCLK, GPIO_DBCLKSEL_8);

	asleep = 0;

	/* SWITCH PA.9 */
	GPIO_DisableInt(PA, 9);
	GPIO_EnableInt(PA, 9, GPIO_INT_FALLING);
	/* USB PRESENT */
	GPIO_DisableInt(USBP_PORT, USBP_PIN);
	GPIO_EnableInt(USBP_PORT, USBP_PIN, GPIO_INT_BOTH_EDGE);

	CHILI_TimersInit();
	CHILI_GPIOPowerUp();
	CHILI_SystemReInit();
}

/******************************************************************************/
/***************************************************************************/ /**
 * \brief See cascoda-bm/cascoda_interface.h
 *******************************************************************************
 ******************************************************************************/
u8_t BSP_GPIOSenseSwitch(void)
{
	return PA9;
}

/******************************************************************************/
/***************************************************************************/ /**
 * \brief See cascoda-bm/cascoda_interface.h
 *******************************************************************************
 ******************************************************************************/
void BSP_UseExternalClock(u8_t useExternalClock)
{
	if (UseExternalClock == useExternalClock)
		return;
	UseExternalClock = useExternalClock;

	if (useExternalClock)
	{
		/* swap then disable  */
		CHILI_SystemReInit();
		CHILI_EnableIntOscCal();
	}
	else
	{
		/* disable then swap */
		CHILI_DisableIntOscCal();
		CHILI_SystemReInit();
	}
}

/******************************************************************************/
/***************************************************************************/ /**
 * \brief See cascoda-bm/cascoda_interface.h
 *******************************************************************************
 ******************************************************************************/
void BSP_Waiting(void)
{
}

/******************************************************************************/
/***************************************************************************/ /**
 * \brief See cascoda-bm/cascoda_interface.h
 *******************************************************************************
 ******************************************************************************/
void BSP_Initialise(struct ca821x_dev *pDeviceRef)
{
	u32_t               i;
	struct device_link *device;

	/* start up is on internal HIRC Oscillator, 12MHz, PLL disabled */

	/* initialise GPIOs */
	CHILI_GPIOInit();

	/* wait until system is stable after potential usb plug-in */
	while (!(CLK->CLKSTATUS & CLK_CLKSTATUS_HIRC_STB))
		;
	for (i = 0; i < 500000; ++i)
		;

	/* system  initialisation (clocks, timers ..) */
	CHILI_SystemReInit();

	/* enable GPIO interrupts */
	CHILI_GPIOEnableInterrupts();

	/* Initialise flash config */
	CHILI_FlashInit();

	device                       = &device_list[0];
	device->chip_select_gpio     = &PB3;
	device->irq_gpio             = &PA10;
	device->dev                  = pDeviceRef;
	pDeviceRef->exchange_context = device;
}

/******************************************************************************/
/***************************************************************************/ /**
 * \brief See cascoda-bm/cascoda_interface.h
 *******************************************************************************
 ******************************************************************************/
void BSP_WatchdogEnable(u32_t timeout_ms)
{
	/* uses TIMER3 instead of WDT for longer timeout intervals */
	if (timeout_ms < 0x00FFFFFF)
	{
		/* 10 kHz Clock: prescaler 9 gives 1 ms units */
		TIMER_SET_PRESCALE_VALUE(TIMER3, 9);
		TIMER_SET_CMP_VALUE(TIMER3, timeout_ms);
		TIMER3->CTL |= TIMER_PERIODIC_MODE;
		TIMER_EnableInt(TIMER3);
		TIMER_EnableWakeup(TIMER3);
		NVIC_EnableIRQ(TMR3_IRQn);
		/* always uses 10 kHz LIRC clock */
		CLK_SetModuleClock(TMR3_MODULE, CLK_CLKSEL2_TMR3_S_LIRC, 0);
		CLK_EnableModuleClock(TMR3_MODULE);
		TIME_WaitTicks(2);
		TIMER_Start(TIMER3);
		while (!TIMER_IS_ACTIVE(TIMER3))
			;
		WDTimeout = 0;
	}
}

/******************************************************************************/
/***************************************************************************/ /**
 * \brief See cascoda-bm/cascoda_interface.h
 *******************************************************************************
 ******************************************************************************/
void BSP_WatchdogReset(u32_t timeout_ms)
{
	/* uses TIMER3 instead of WDT for longer timeout intervals */
	if (timeout_ms < 0x00FFFFFF)
	{
		TIMER3->CTL |= TIMER_CTL_SW_RST_Msk;
		while (TIMER3->CTL & TIMER_CTL_SW_RST_Msk)
			;
		BSP_WaitUs(500); /* wait additional 5 clock cycles to be sure */
	}
}

/******************************************************************************/
/***************************************************************************/ /**
 * \brief See cascoda-bm/cascoda_interface.h
 *******************************************************************************
 ******************************************************************************/
u8_t BSP_IsWatchdogTriggered(void)
{
	u8_t retval = WDTimeout;
	if (retval)
		WDTimeout = 0;
	return retval;
}

/******************************************************************************/
/***************************************************************************/ /**
 * \brief See cascoda-bm/cascoda_interface.h
 *******************************************************************************
 ******************************************************************************/
void BSP_WatchdogDisable(void)
{
	/* uses TIMER3 instead of WDT for longer timeout intervals */
	if (TIMER_IS_ACTIVE(TIMER3))
	{
		NVIC_DisableIRQ(TMR3_IRQn);
		TIMER3->ISR = TIMER_ISR_TMR_IS_Msk | TIMER_ISR_TCAP_IS_Msk | TIMER_ISR_TMR_WAKE_STS_Msk;
		TIMER_Close(TIMER3);
		while (TIMER_IS_ACTIVE(TIMER3))
			;
		CLK_DisableModuleClock(TMR3_MODULE);
	}
	WDTimeout = 0;
}

/******************************************************************************/
/***************************************************************************/ /**
 * \brief See cascoda-bm/cascoda_interface.h
 *******************************************************************************
 ******************************************************************************/
void BSP_EnableUSB(void)
{
#if defined(USE_USB)
	if (USBPresent || !USBP_GPIO)
		return;
	USBPresent = 1;
	CHILI_SystemReInit();
#endif /* USE_USB */
}

/******************************************************************************/
/***************************************************************************/ /**
 * \brief See cascoda-bm/cascoda_interface.h
 *******************************************************************************
 ******************************************************************************/
void BSP_DisableUSB(void)
{
#if defined(USE_USB)
	if (!USBPresent)
		return;
	USBPresent = 0;
	CHILI_SystemReInit();
#endif /* USE_USB */
}

/******************************************************************************/
/***************************************************************************/ /**
 * \brief See cascoda-bm/cascoda_interface.h
 *******************************************************************************
 ******************************************************************************/
u8_t BSP_IsUSBPresent(void)
{
	return USBPresent;
}
