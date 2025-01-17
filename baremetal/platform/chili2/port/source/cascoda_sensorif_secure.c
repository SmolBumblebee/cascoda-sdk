
#include <stdio.h>
/* Platform */
#include "M2351.h"
#include "i2c.h"
#include "spi.h"
#include "sys.h"
/* Cascoda */
#include "cascoda-bm/cascoda_interface.h"
#include "cascoda-bm/cascoda_sensorif.h"
#include "cascoda-bm/cascoda_types.h"
#include "cascoda_chili.h"
#include "cascoda_chili_config.h"

#ifndef CASCODA_CHILI2_CONFIG
#error CASCODA_CHILI2_CONFIG has to be defined! Please include the file "cascoda_chili_config.h"
#endif

static uint8_t SENSORIF_I2CNUM_S;
static I2C_T*  SENSORIF_I2CIF_S;

static uint8_t SENSORIF_SPINUM_S;
static SPI_T*  SENSORIF_SPIIF_S;

static uint8_t  SENSORIF_UARTNUM_S;
static UART_T*  SENSORIF_UARTIF_S;
static uint32_t SENSORIF_UART_MODULE_S;
static uint32_t SENSORIF_UART_RST_S;

static uint8_t  SENSORIF_PWM_PIN_S;
PWM_type_t      SENSORIF_PWM_TYPE_S;
static BPWM_T*  SENSORIF_PWM_BPWMIF_S;
static EPWM_T*  SENSORIF_PWM_EPWMIF_S;
static uint32_t SENSORIF_PWM_MODULE_S;
static uint32_t SENSORIF_PWM_CHANNEL_NUM_S;
static uint32_t SENSORIF_PWM_CHANNEL_MASK_S;
static uint32_t SENSORIF_PWM_RST_S;

static void SENSORIF_SECURE_PWM_Pin_Config_Helper()
{
	if (SENSORIF_PWM_PIN_S == 15)
	{
		GPIO_ENABLE_DIGITAL_PATH(PA, BIT15);
		GPIO_DISABLE_DEBOUNCE(PA, BIT15);
		GPIO_SetPullCtl(PA, BIT15, GPIO_PUSEL_DISABLE);
		SYS->GPA_MFPH = (SYS->GPA_MFPH & (~SYS_GPA_MFPH_PA15MFP_Msk)) | SYS_GPA_MFPH_PA15MFP_BPWM1_CH5;
	}
	else if (SENSORIF_PWM_PIN_S == 31)
	{
		GPIO_ENABLE_DIGITAL_PATH(PB, BIT5);
		GPIO_DISABLE_DEBOUNCE(PB, BIT5);
		GPIO_SetPullCtl(PB, BIT5, GPIO_PUSEL_DISABLE);
		SYS->GPB_MFPL = (SYS->GPB_MFPL & (~SYS_GPB_MFPL_PB5MFP_Msk)) | SYS_GPB_MFPL_PB5MFP_EPWM0_CH0;
	}
	else if (SENSORIF_PWM_PIN_S == 32)
	{
		GPIO_ENABLE_DIGITAL_PATH(PB, BIT4);
		GPIO_DISABLE_DEBOUNCE(PB, BIT4);
		GPIO_SetPullCtl(PB, BIT4, GPIO_PUSEL_DISABLE);
		SYS->GPB_MFPL = (SYS->GPB_MFPL & (~SYS_GPB_MFPL_PB4MFP_Msk)) | SYS_GPB_MFPL_PB4MFP_EPWM0_CH1;
	}
	else if (SENSORIF_PWM_PIN_S == 33)
	{
		GPIO_ENABLE_DIGITAL_PATH(PB, BIT3);
		GPIO_DISABLE_DEBOUNCE(PB, BIT3);
		GPIO_SetPullCtl(PB, BIT3, GPIO_PUSEL_DISABLE);
		SYS->GPB_MFPL = (SYS->GPB_MFPL & (~SYS_GPB_MFPL_PB3MFP_Msk)) | SYS_GPB_MFPL_PB3MFP_EPWM0_CH2;
	}
	else if (SENSORIF_PWM_PIN_S == 34)
	{
		GPIO_ENABLE_DIGITAL_PATH(PB, BIT2);
		GPIO_DISABLE_DEBOUNCE(PB, BIT2);
		GPIO_SetPullCtl(PB, BIT2, GPIO_PUSEL_DISABLE);
		SYS->GPB_MFPL = (SYS->GPB_MFPL & (~SYS_GPB_MFPL_PB2MFP_Msk)) | SYS_GPB_MFPL_PB2MFP_EPWM0_CH3;
	}
	else if (SENSORIF_PWM_PIN_S == 35)
	{
		GPIO_ENABLE_DIGITAL_PATH(PB, BIT1);
		GPIO_DISABLE_DEBOUNCE(PB, BIT1);
		GPIO_SetPullCtl(PB, BIT1, GPIO_PUSEL_DISABLE);
		SYS->GPB_MFPL = (SYS->GPB_MFPL & (~SYS_GPB_MFPL_PB1MFP_Msk)) | SYS_GPB_MFPL_PB1MFP_EPWM0_CH4;
	}
}

static ca_error SENSORIF_PWM_Config(u8_t pin)
{
	if (SENSORIF_PWM_TYPE_S != NOT_INITIALISED)
	{
		ca_log_warn("Already configured. Aborting...");
		return CA_ERROR_ALREADY;
	}

	SENSORIF_PWM_PIN_S = pin;

	if (SENSORIF_PWM_PIN_S == 15)
	{
		SENSORIF_PWM_TYPE_S         = USING_BPWM;
		SENSORIF_PWM_BPWMIF_S       = BPWM1;
		SENSORIF_PWM_MODULE_S       = BPWM1_MODULE;
		SENSORIF_PWM_CHANNEL_NUM_S  = 5;
		SENSORIF_PWM_CHANNEL_MASK_S = BPWM_CH_5_MASK;
		SENSORIF_PWM_RST_S          = BPWM1_RST;
	}
	else if (SENSORIF_PWM_PIN_S == 31)
	{
		SENSORIF_PWM_TYPE_S         = USING_EPWM;
		SENSORIF_PWM_EPWMIF_S       = EPWM0;
		SENSORIF_PWM_MODULE_S       = EPWM0_MODULE;
		SENSORIF_PWM_CHANNEL_NUM_S  = 0;
		SENSORIF_PWM_CHANNEL_MASK_S = EPWM_CH_0_MASK;
		SENSORIF_PWM_RST_S          = EPWM0_RST;
	}
	else if (SENSORIF_PWM_PIN_S == 32)
	{
		SENSORIF_PWM_TYPE_S         = USING_EPWM;
		SENSORIF_PWM_EPWMIF_S       = EPWM0;
		SENSORIF_PWM_MODULE_S       = EPWM0_MODULE;
		SENSORIF_PWM_CHANNEL_NUM_S  = 1;
		SENSORIF_PWM_CHANNEL_MASK_S = EPWM_CH_1_MASK;
		SENSORIF_PWM_RST_S          = EPWM0_RST;
	}
	else if (SENSORIF_PWM_PIN_S == 33)
	{
		SENSORIF_PWM_TYPE_S         = USING_EPWM;
		SENSORIF_PWM_EPWMIF_S       = EPWM0;
		SENSORIF_PWM_MODULE_S       = EPWM0_MODULE;
		SENSORIF_PWM_CHANNEL_NUM_S  = 2;
		SENSORIF_PWM_CHANNEL_MASK_S = EPWM_CH_2_MASK;
		SENSORIF_PWM_RST_S          = EPWM0_RST;
	}
	else if (SENSORIF_PWM_PIN_S == 34)
	{
		SENSORIF_PWM_TYPE_S         = USING_EPWM;
		SENSORIF_PWM_EPWMIF_S       = EPWM0;
		SENSORIF_PWM_MODULE_S       = EPWM0_MODULE;
		SENSORIF_PWM_CHANNEL_NUM_S  = 3;
		SENSORIF_PWM_CHANNEL_MASK_S = EPWM_CH_3_MASK;
		SENSORIF_PWM_RST_S          = EPWM0_RST;
	}
	else if (SENSORIF_PWM_PIN_S == 35)
	{
		SENSORIF_PWM_TYPE_S         = USING_EPWM;
		SENSORIF_PWM_EPWMIF_S       = EPWM0;
		SENSORIF_PWM_MODULE_S       = EPWM0_MODULE;
		SENSORIF_PWM_CHANNEL_NUM_S  = 4;
		SENSORIF_PWM_CHANNEL_MASK_S = EPWM_CH_4_MASK;
		SENSORIF_PWM_RST_S          = EPWM0_RST;
	}
	else
	{
		ca_log_warn("sensorif PWM not supported on pin %d", pin);
		SENSORIF_PWM_TYPE_S = NOT_INITIALISED;
		return CA_ERROR_NOT_HANDLED;
	}

	return CA_ERROR_SUCCESS;
}

__NONSECURE_ENTRY void SENSORIF_SECURE_I2C_Config(u32_t portnum)
{
	SENSORIF_I2CNUM_S = portnum;
	if (SENSORIF_I2CNUM_S == 0)
		SENSORIF_I2CIF_S = I2C0;
	else if (SENSORIF_I2CNUM_S == 1)
		SENSORIF_I2CIF_S = I2C1;
	else if (SENSORIF_I2CNUM_S == 2)
		SENSORIF_I2CIF_S = I2C2;
	else
		ca_log_warn("sensorif I2C module not valid");
}

__NONSECURE_ENTRY void SENSORIF_SECURE_SPI_Config(u32_t portnum)
{
	SENSORIF_SPINUM_S = portnum;
	/* SPI module */
	if (SENSORIF_SPINUM_S == 1)
		SENSORIF_SPIIF_S = SPI1;
#if ((CASCODA_CHILI2_CONFIG != 2) && (CASCODA_CHILI2_CONFIG != 3)) /* not Chili2 devboard */
	else if (SENSORIF_SPINUM_S == 2)
		SENSORIF_SPIIF_S = SPI2;
#endif
	else
		ca_log_warn("sensorif SPI module not valid");
}

__NONSECURE_ENTRY void SENSORIF_SECURE_UART_Config(u32_t portnum)
{
	SENSORIF_UARTNUM_S = portnum;
	/* UART module */
	if (SENSORIF_UARTNUM_S == 5)
	{
		SENSORIF_UARTIF_S      = UART5;
		SENSORIF_UART_MODULE_S = UART5_MODULE;
		SENSORIF_UART_RST_S    = UART5_RST;
	}
	else
		ca_log_warn("sensorif UART module not valid");
}

__NONSECURE_ENTRY void SENSORIF_I2C_Init(void)
{
	/* enable I2C peripheral clock */
	if (SENSORIF_I2CNUM_S == 0)
		CLK_EnableModuleClock(I2C0_MODULE);
	else if (SENSORIF_I2CNUM_S == 1)
		CLK_EnableModuleClock(I2C1_MODULE);
	else
		CLK_EnableModuleClock(I2C2_MODULE);

	/* SDA/SCL port configurations */
	if (SENSORIF_I2CNUM_S == 0)
	{
		/* re-config PB.5 and PB.4 */
		GPIO_ENABLE_DIGITAL_PATH(PB, BIT5);
		GPIO_ENABLE_DIGITAL_PATH(PB, BIT4);
		GPIO_DISABLE_DEBOUNCE(PB, BIT5);
		GPIO_DISABLE_DEBOUNCE(PB, BIT4);
#if (SENSORIF_INT_PULLUPS)
		GPIO_SetPullCtl(PB, BIT5, GPIO_PUSEL_PULL_UP);
		GPIO_SetPullCtl(PB, BIT4, GPIO_PUSEL_PULL_UP);
#else
		GPIO_SetPullCtl(PB, BIT5, GPIO_PUSEL_DISABLE);
		GPIO_SetPullCtl(PB, BIT4, GPIO_PUSEL_DISABLE);
#endif
		/* initialise PB MFP for I2C0 SDA and SCL */
		/* PB.5 = I2C0 SCL */
		/* PB.4 = I2C0 SDA */
		SYS->GPB_MFPL &= ~(SYS_GPB_MFPL_PB4MFP_Msk | SYS_GPB_MFPL_PB5MFP_Msk);
		SYS->GPB_MFPL |= (SYS_GPB_MFPL_PB4MFP_I2C0_SDA | SYS_GPB_MFPL_PB5MFP_I2C0_SCL);
	}

	else if (SENSORIF_I2CNUM_S == 1)
	{
#if (CASCODA_CHILI2_CONFIG == 2) /* Chili2 devboard */
		/* re-config PF.0 and PF.1 */
		GPIO_ENABLE_DIGITAL_PATH(PF, BIT0);
		GPIO_ENABLE_DIGITAL_PATH(PF, BIT1);
		GPIO_DISABLE_DEBOUNCE(PF, BIT0);
		GPIO_DISABLE_DEBOUNCE(PF, BIT1);
#if (SENSORIF_INT_PULLUPS)
		GPIO_SetPullCtl(PF, BIT0, GPIO_PUSEL_PULL_UP);
		GPIO_SetPullCtl(PF, BIT1, GPIO_PUSEL_PULL_UP);
#else
		GPIO_SetPullCtl(PF, BIT0, GPIO_PUSEL_DISABLE);
		GPIO_SetPullCtl(PF, BIT1, GPIO_PUSEL_DISABLE);
#endif
		/* initialise PF MFP for I2C1 SDA and SCL */
		/* PF.0 = I2C1 SCL */
		/* PF.1 = I2C1 SDA */
		SYS->GPF_MFPL &= ~(SYS_GPF_MFPL_PF1MFP_Msk | SYS_GPF_MFPL_PF0MFP_Msk);
		SYS->GPF_MFPL |= (SYS_GPF_MFPL_PF1MFP_I2C1_SDA | SYS_GPF_MFPL_PF0MFP_I2C1_SCL);
#else
		/* re-config PB.1 and PB.0 */
		GPIO_ENABLE_DIGITAL_PATH(PB, BIT1);
		GPIO_ENABLE_DIGITAL_PATH(PB, BIT0);
		GPIO_DISABLE_DEBOUNCE(PB, BIT1);
		GPIO_DISABLE_DEBOUNCE(PB, BIT0);
#if (SENSORIF_INT_PULLUPS)
		GPIO_SetPullCtl(PB, BIT1, GPIO_PUSEL_PULL_UP);
		GPIO_SetPullCtl(PB, BIT0, GPIO_PUSEL_PULL_UP);
#else
		GPIO_SetPullCtl(PB, BIT1, GPIO_PUSEL_DISABLE);
		GPIO_SetPullCtl(PB, BIT0, GPIO_PUSEL_DISABLE);
#endif
		/* initialise PB MFP for I2C1 SDA and SCL */
		/* PB.1 = I2C1 SCL */
		/* PB.0 = I2C1 SDA */
		SYS->GPB_MFPL &= ~(SYS_GPB_MFPL_PB0MFP_Msk | SYS_GPB_MFPL_PB1MFP_Msk);
		SYS->GPB_MFPL |= (SYS_GPB_MFPL_PB0MFP_I2C1_SDA | SYS_GPB_MFPL_PB1MFP_I2C1_SCL);
#endif // CASCODA_CHILI2_CONFIG
	}
	else
	{
		/* re-config PB.13 and PB.12 */
		GPIO_ENABLE_DIGITAL_PATH(PB, BIT13);
		GPIO_ENABLE_DIGITAL_PATH(PB, BIT12);
		GPIO_DISABLE_DEBOUNCE(PB, BIT13);
		GPIO_DISABLE_DEBOUNCE(PB, BIT12);
#if (SENSORIF_INT_PULLUPS)
		GPIO_SetPullCtl(PB, BIT13, GPIO_PUSEL_PULL_UP);
		GPIO_SetPullCtl(PB, BIT12, GPIO_PUSEL_PULL_UP);
#else
		GPIO_SetPullCtl(PB, BIT13, GPIO_PUSEL_DISABLE);
		GPIO_SetPullCtl(PB, BIT12, GPIO_PUSEL_DISABLE);
#endif
		/* initialise PB MFP for I2C2 SDA and SCL */
		/* PB.13 = I2C2 SCL */
		/* PB.12 = I2C2 SDA */
		SYS->GPB_MFPL &= ~(SYS_GPB_MFPH_PB12MFP_Msk | SYS_GPB_MFPH_PB13MFP_Msk);
		SYS->GPB_MFPL |= (SYS_GPB_MFPH_PB12MFP_I2C2_SDA | SYS_GPB_MFPH_PB13MFP_I2C2_SCL);
	}

	/* reset I2C module */
	if (SENSORIF_I2CNUM_S == 0)
		SYS_ResetModule(I2C0_RST);
	else if (SENSORIF_I2CNUM_S == 1)
		SYS_ResetModule(I2C1_RST);
	else
		SYS_ResetModule(I2C2_RST);

	/* enable I2C */
	I2C_Open(SENSORIF_I2CIF_S, SENSORIF_I2C_CLK_FREQUENCY);
}

__NONSECURE_ENTRY void SENSORIF_I2C_Deinit(void)
{
	/* wait (max. 100 us) until bus access has ended */
	for (u8_t i = 0; i < 10; ++i)
	{
		BSP_WaitUs(10);
		if (!(I2C_SMBusGetStatus(SENSORIF_I2CIF_S) & I2C_BUSSTS_BUSY_Msk))
			break;
	}
	I2C_DisableInt(SENSORIF_I2CIF_S);
	I2C_Close(SENSORIF_I2CIF_S);
	if (SENSORIF_I2CNUM_S == 0)
		CLK_DisableModuleClock(I2C0_MODULE);
	else if (SENSORIF_I2CNUM_S == 1)
		CLK_DisableModuleClock(I2C1_MODULE);
	else
		CLK_DisableModuleClock(I2C2_MODULE);
}

__NONSECURE_ENTRY void SENSORIF_SPI_Init(void)
{
	/* enable SPI peripheral clock */
	if (SENSORIF_SPINUM_S == 1)
		CLK_EnableModuleClock(SPI1_MODULE);
	else if (SENSORIF_SPINUM_S == 2)
		CLK_EnableModuleClock(SPI2_MODULE);

	/* MOSI/MISO/CLK/SS port configurations*/
	if (SENSORIF_SPINUM_S == 1)
	{
#if (!CASCODA_EINK_DISPLAY_PRESENT)
		/* re-config PB.5 for MISO pin for full duplex mode*/
		GPIO_ENABLE_DIGITAL_PATH(PB, BIT5);
		GPIO_DISABLE_DEBOUNCE(PB, BIT5);
		GPIO_SetPullCtl(PB, BIT5, GPIO_PUSEL_DISABLE);
		/* initialise PB MFP for MISO */
		SYS->GPB_MFPL = (SYS->GPB_MFPL & (~SYS_GPB_MFPL_PB5MFP_Msk)) | SYS_GPB_MFPL_PB5MFP_SPI1_MISO;
#endif
		/* re-config PB.4, PB.3 and PB.2 */
		GPIO_ENABLE_DIGITAL_PATH(PB, BIT4);
		GPIO_ENABLE_DIGITAL_PATH(PB, BIT3);
		// GPIO_ENABLE_DIGITAL_PATH(PB, BIT2); /* Uncomment if using SS */
		GPIO_DISABLE_DEBOUNCE(PB, BIT4);
		GPIO_DISABLE_DEBOUNCE(PB, BIT3);
		// GPIO_DISABLE_DEBOUNCE(PB, BIT2); /* Uncomment if using SS */
		GPIO_SetPullCtl(PB, BIT4, GPIO_PUSEL_DISABLE);
		GPIO_SetPullCtl(PB, BIT3, GPIO_PUSEL_DISABLE);
		// GPIO_SetPullCtl(PB, BIT2, GPIO_PUSEL_DISABLE);
		/* initialise PB MFP for SPI1 MOSI, CLK, and SS */
		/* PB.4 = SPI1 MOSI */
		/* PB.3 = SPI1 CLK */
		SYS->GPB_MFPL = (SYS->GPB_MFPL & (~SYS_GPB_MFPL_PB4MFP_Msk)) | SYS_GPB_MFPL_PB4MFP_SPI1_MOSI;
		SYS->GPB_MFPL = (SYS->GPB_MFPL & (~SYS_GPB_MFPL_PB3MFP_Msk)) | SYS_GPB_MFPL_PB3MFP_SPI1_CLK;
		/* PB.2 = SPI1 SS */
		// SYS->GPB_MFPL = (SYS->GPB_MFPL & (~SYS_GPB_MFPL_PB2MFP_Msk)) | SYS_GPB_MFPL_PB2MFP_GPIO;
	}
	else if (SENSORIF_SPINUM_S == 2)
	{
		/* re-config PA.15, PA.13 and PA.12 */
		GPIO_ENABLE_DIGITAL_PATH(PA, BIT15);
		GPIO_ENABLE_DIGITAL_PATH(PA, BIT13);
		// GPIO_ENABLE_DIGITAL_PATH(PA, BIT12); /* Uncomment if using SS */
		GPIO_DISABLE_DEBOUNCE(PA, BIT15);
		GPIO_DISABLE_DEBOUNCE(PA, BIT13);
		// GPIO_DISABLE_DEBOUNCE(PA, BIT12); /* Uncomment if using SS */
		GPIO_SetPullCtl(PA, BIT15, GPIO_PUSEL_DISABLE);
		GPIO_SetPullCtl(PA, BIT13, GPIO_PUSEL_DISABLE);
		// GPIO_SetPullCtl(PA, BIT12, GPIO_PUSEL_DISABLE); /* Uncomment if using SS */
		/* initialise PA MFP for SPI2 MOSI, CLK, and SS */
		/* PA.15 = SPI2 MOSI */
		/* PA.13 = SPI2 CLK */
		SYS->GPA_MFPH = (SYS->GPA_MFPH & (~SYS_GPA_MFPH_PA15MFP_Msk)) | SYS_GPA_MFPH_PA15MFP_SPI2_MOSI;
		SYS->GPA_MFPH = (SYS->GPA_MFPH & (~SYS_GPA_MFPH_PA13MFP_Msk)) | SYS_GPA_MFPH_PA13MFP_SPI2_CLK;
		/* PA.12 = SPI2 SS */
		//SYS->GPA_MFPH = (SYS->GPA_MFPH & (~SYS_GPA_MFPH_PA12MFP_Msk)) | SYS_GPA_MFPH_PA12MFP_SPI2_SS;
	}

	/* reset SPI module */
	if (SENSORIF_SPINUM_S == 1)
		SYS_ResetModule(SPI1_RST);
	else if (SENSORIF_SPINUM_S == 2)
		SYS_ResetModule(SPI2_RST);

	/* clear transmit fifo but only clear receive fifo for full-duplex mode*/
	SPI_ClearTxFIFO(SENSORIF_SPIIF_S);
#if (!CASCODA_EINK_DISPLAY_PRESENT)
	SPI_ClearRxFIFO(SENSORIF_SPIIF_S);
#endif

	/* enable SPI */
	SPI_Open(SENSORIF_SPIIF_S, SPI_MASTER, SPI_MODE_0, SENSORIF_SPI_DATA_WIDTH, SENSORIF_SPI_CLK_FREQUENCY);

#if (CASCODA_EINK_DISPLAY_PRESENT)
	/* Set SPI commmunication to half-duplex mode with output data direction */
	SENSORIF_SPIIF_S->CTL |= (SPI_CTL_HALFDPX_Msk | SPI_CTL_DATDIR_Msk);
#endif
}

__NONSECURE_ENTRY void SENSORIF_SPI_Deinit(void)
{
	SPI_DisableInt(SENSORIF_SPIIF_S, 0x3FF);
	SPI_Close(SENSORIF_SPIIF_S);
	if (SENSORIF_SPINUM_S == 1)
		CLK_DisableModuleClock(SPI1_MODULE);
	else if (SENSORIF_SPINUM_S == 2)
		CLK_DisableModuleClock(SPI2_MODULE);
}

#if !defined(USE_UART)
__NONSECURE_ENTRY void SENSORIF_UART_Init(void)
{
	CLK_EnableModuleClock(SENSORIF_UART_MODULE_S);

	if (SENSORIF_UARTNUM_S == 5)
	{
		/* re-config PB.5 and PB.4 */
		GPIO_ENABLE_DIGITAL_PATH(PB, BIT5);
		GPIO_ENABLE_DIGITAL_PATH(PB, BIT4);
		GPIO_DISABLE_DEBOUNCE(PB, BIT5);
		GPIO_DISABLE_DEBOUNCE(PB, BIT4);
		GPIO_SetPullCtl(PB, BIT5, GPIO_PUSEL_DISABLE);
		GPIO_SetPullCtl(PB, BIT4, GPIO_PUSEL_DISABLE);

		/* initialise PB MFP for UART TX and RX */
		/* PB.4 = UART5 RX */
		/* PB.5 = UART5 TX */
		SYS->GPB_MFPL = (SYS->GPB_MFPL & (~SYS_GPB_MFPL_PB4MFP_Msk)) | SYS_GPB_MFPL_PB4MFP_UART5_RXD;
		SYS->GPB_MFPL = (SYS->GPB_MFPL & (~SYS_GPB_MFPL_PB5MFP_Msk)) | SYS_GPB_MFPL_PB5MFP_UART5_TXD;
	}
	SYS_ResetModule(SENSORIF_UART_RST_S);
	UART_SetLineConfig(SENSORIF_UARTIF_S, UART_SENSORIF_BAUDRATE, UART_WORD_LEN_8, UART_PARITY_NONE, UART_STOP_BIT_1);
	UART_Open(SENSORIF_UARTIF_S, UART_SENSORIF_BAUDRATE);
}

__NONSECURE_ENTRY void SENSORIF_UART_Deinit(void)
{
	CLK_DisableModuleClock(SENSORIF_UART_MODULE_S);
	UART_Close(SENSORIF_UARTIF_S);
}
#endif // !defined(USE_UART)

__NONSECURE_ENTRY ca_error SENSORIF_PWM_Init(u8_t pin, u32_t u32Frequency, u32_t u32DutyCycle)
{
#define NOT_USED 0

	ca_log_debg("In SENSORIF_PWM_Init(), pin: %d, freq: %d, duty cycle: %d", pin, u32Frequency, u32DutyCycle);

	ca_error status;

	// Safeguards against Init function being called twice without a deinit in between
	if (SENSORIF_PWM_TYPE_S != NOT_INITIALISED)
	{
		ca_log_warn("Already initialised. If you want to initialise again, deinit first. Aborting...");
		return CA_ERROR_ALREADY;
	}

	// Configures which PWM interface will be used based on pin number
	if ((status = SENSORIF_PWM_Config(pin)))
		return status;

	// Clock configuration
	CLK_EnableModuleClock(SENSORIF_PWM_MODULE_S);
	if (SENSORIF_PWM_TYPE_S == USING_BPWM)
		BPWM_SetClockSource(SENSORIF_PWM_BPWMIF_S, NOT_USED, BPWM_CLKSRC_BPWM_CLK);
	else if (SENSORIF_PWM_TYPE_S == USING_EPWM)
		EPWM_SetClockSource(SENSORIF_PWM_EPWMIF_S, SENSORIF_PWM_CHANNEL_NUM_S, EPWM_CLKSRC_EPWM_CLK);

	// Output port/pin configuration
	SENSORIF_SECURE_PWM_Pin_Config_Helper();

	// Reset PWM module
	SYS_ResetModule(SENSORIF_PWM_RST_S);

	// Configure PWM settings (Module, channel, freq and duty cycle)
	if (SENSORIF_PWM_TYPE_S == USING_BPWM)
		BPWM_ConfigOutputChannel(
		    SENSORIF_PWM_BPWMIF_S, SENSORIF_PWM_CHANNEL_NUM_S, u32Frequency, (u32DutyCycle > 100) ? 100 : u32DutyCycle);
	else if (SENSORIF_PWM_TYPE_S == USING_EPWM)
		EPWM_ConfigOutputChannel(
		    SENSORIF_PWM_EPWMIF_S, SENSORIF_PWM_CHANNEL_NUM_S, u32Frequency, (u32DutyCycle > 100) ? 100 : u32DutyCycle);

	// Start
	if (SENSORIF_PWM_TYPE_S == USING_BPWM)
	{
		BPWM_Start(SENSORIF_PWM_BPWMIF_S, NOT_USED);
		BPWM_EnableOutput(SENSORIF_PWM_BPWMIF_S, SENSORIF_PWM_CHANNEL_MASK_S);
	}
	else if (SENSORIF_PWM_TYPE_S == USING_EPWM)
	{
		EPWM_Start(SENSORIF_PWM_EPWMIF_S, SENSORIF_PWM_CHANNEL_MASK_S);
		EPWM_EnableOutput(SENSORIF_PWM_EPWMIF_S, SENSORIF_PWM_CHANNEL_MASK_S);
	}

	return status;
}

__NONSECURE_ENTRY void SENSORIF_PWM_Deinit(void)
{
	ca_log_debg("In SENSORIF_PWM_Deinit()");
	if (SENSORIF_PWM_TYPE_S == NOT_INITIALISED)
	{
		ca_log_warn("PWM not initialised... Nothing to deinitialise");
		return;
	}

	if (SENSORIF_PWM_TYPE_S == USING_BPWM)
	{
		BPWM_DisableOutput(SENSORIF_PWM_BPWMIF_S, SENSORIF_PWM_CHANNEL_MASK_S);
		BPWM_Stop(SENSORIF_PWM_BPWMIF_S, SENSORIF_PWM_CHANNEL_MASK_S);
	}
	else if (SENSORIF_PWM_TYPE_S == USING_EPWM)
	{
		EPWM_DisableOutput(SENSORIF_PWM_EPWMIF_S, SENSORIF_PWM_CHANNEL_MASK_S);
		EPWM_Stop(SENSORIF_PWM_EPWMIF_S, SENSORIF_PWM_CHANNEL_MASK_S);
	}

	CLK_DisableModuleClock(SENSORIF_PWM_MODULE_S);

	// Resets state to allow new initialisation to occur.
	SENSORIF_PWM_TYPE_S = NOT_INITIALISED;
}

void SENSORIF_PWM_SetDutyCycle(u32_t u32DutyCycle)
{
	ca_log_debg("In SENSORIF_PWM_SetDutyCycle(), duty cycle: %d", u32DutyCycle);

	if (SENSORIF_PWM_TYPE_S == NOT_INITIALISED)
	{
		ca_log_warn("PWM not initialised. Aborting...");
		return;
	}

	u32_t u32CNR;
	u32_t u32CMR;

	// Get the current period value
	if (SENSORIF_PWM_TYPE_S == USING_BPWM)
		u32CNR = BPWM_GET_CNR(SENSORIF_PWM_BPWMIF_S, NOT_USED);
	else
		u32CNR = EPWM_GET_CNR(SENSORIF_PWM_EPWMIF_S, SENSORIF_PWM_CHANNEL_NUM_S);

	// Calculate the cmp value that is required to achieve the specified duty cycle
	u32CMR = ((u32DutyCycle > 100) ? 100 : u32DutyCycle) * (u32CNR + 1UL) / 100UL;

	// Set that new cmp value
	if (SENSORIF_PWM_TYPE_S == USING_BPWM)
		BPWM_SET_CMR(SENSORIF_PWM_BPWMIF_S, SENSORIF_PWM_CHANNEL_NUM_S, u32CMR);
	else
		EPWM_SET_CMR(SENSORIF_PWM_EPWMIF_S, SENSORIF_PWM_CHANNEL_NUM_S, u32CMR);
}
