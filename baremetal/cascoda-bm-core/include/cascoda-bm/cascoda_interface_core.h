/*
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
/**
 * @file
 * Declarations for core platform abstraction functions.
 */
/**
 * @ingroup bm-core
 * @defgroup bm-interface-core Baremetal Core platform abstraction
 * @brief  Platform abstraction for core SDK functionality, see \ref bm-interface for extended functionality.
 *
 * @{
 */

#ifndef CASCODA_INTERFACE_CORE_H
#define CASCODA_INTERFACE_CORE_H

#include "cascoda_types.h"
#include "mac_messages.h"

/**
 * @brief Send an unhandled message upstream.
 *
 * This function must be implemented by a lower layer (such as EVBME). If you
 * want to catch unhandled messages at the application level, implement the
 * `generic_dispatch` member of `ca821x_api_callbacks.
 *
 * @param msg Message to send upstream
 */
void DISPATCH_NotHandled(struct MAC_Message *msg);

/**
 * \brief Wait for specified Time in Microseconds (max. 1000)
 *
 * \param us - Time in microseconds
 *
 */
void BSP_WaitUs(u32_t us);

/**
 * \brief This function will be called repeatedly when the Baremetal drivers
 * are blocking & waiting (eg. WAIT_ms or WAIT_Callback), in case the
 * BSP needs to do any system maintenance or wants to reduce power consumption.
 *
 */
void BSP_Waiting(void);

/**
 * \brief Get the number of milliseconds since program start.
 *
 * \returns The number of milliseconds since program start
 *
 */
u32_t BSP_ReadAbsoluteTime(void);

/**
 * \brief Wait for specified Time in Milliseconds
 *
 * \param ticks - Time in milliseconds
 *
 */
void BSP_WaitTicks(u32_t ticks);

/**
 * \brief Reset CAX RF Chip
 *
 * \param ms - Reset Low Time in [ms]
 *
 */
void BSP_ResetRF(u8_t ms);

/**
 * \brief Sense whether SPI IRQ is high or low
 *
 * \return State of IRQ Pin  PA.10
 *
 */
u8_t BSP_SenseRFIRQ(void);

/**
 * \brief Inhibit SPI IRQ, supresses interrupt but still latches it
 *
 */
void BSP_DisableRFIRQ(void);

/**
 * \brief Allow SPI IRQ, re-enabling interrupt after BSP_DisableRFIRQ()
 *
 */
void BSP_EnableRFIRQ(void);

/**
 * \brief Put SPI Select (SSB) Pin high
 *
 */
void BSP_SetRFSSBHigh(void);

/**
 * \brief Put SPI Select (SSB) Pin low
 *
 */
void BSP_SetRFSSBLow(void);

/**
 * \brief Initialise GPIO and SPI Pins for Comms with CA-821X
 *
 */
void BSP_SPIInit(void);

/**
 * \brief Transmit Byte to SPI and Receive Byte at same Time
 *
 * \param OutByte - Character to transmit
 *
 * \return Character received
 *
 * It is required that the Exchange & push/pop methods can work together,
 * so it is recommended that one is implemented with the other. The FIFO
 * behaviour is optimal, as it allows maximal usage of the SPI bus. Then
 * SPIExchangeByte can be implemented like:
 *
 * u8_t BSP_SPIExchangeByte( u8_t OutByte )
 * {
 *    u8_t InByte;
 *    while(!BSP_SPIPushByte(OutByte));
 *    while(!BSP_SPIPopByte(&InByte));
 *    return InByte;
 * }
 *
 * If the given board doesn't support FIFO behaviour, then the push/pop methods
 * can be implemented by using a virtual receive FIFO of length 1, and actually
 * doing the SPI byte exchange when PushByte is called.
 *
 */
u8_t BSP_SPIExchangeByte(u8_t OutByte);

/**
 * \brief Push byte to FIFO for transmitting over SPI
 *
 * \param OutByte - Character to transmit
 *
 * \return 1 if successful, 0 if the FIFO is full
 *
 */
u8_t BSP_SPIPushByte(u8_t OutByte);

/**
 * \brief Get Byte from SPI receive FIFO
 *
 * \param InByte - output, filled with the received byte if retval is 1
 *
 * \return 1 if successful, 0 if the FIFO is full
 *
 */
u8_t BSP_SPIPopByte(u8_t *InByte);

/**
 * \brief Is the code running in an interrupt context?
 * @return true if in interrupt context
 */
bool BSP_IsInsideInterrupt(void);

#endif // CASCODA_INTERFACE_CORE_H

/**
 * @}
 */
