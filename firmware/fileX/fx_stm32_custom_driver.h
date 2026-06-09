/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/**************************************************************************/

/**
 * @file    fx_stm32_custom_driver.h
 * @brief   FileX SPI-SD driver for STM32U0
 *
 * Depends on spi_sd.h / spi_sd.c for low-level SPI SD access.
 */

#ifndef FX_STM32_CUSTOM_DRIVER_H
#define FX_STM32_CUSTOM_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "fx_api.h"
#include "spi_sd.h"     /* low-level SPI SD driver */

/* Exported functions prototypes ---------------------------------------------*/
VOID fx_stm32_custom_driver(FX_MEDIA *media_ptr);

#ifdef __cplusplus
}
#endif
#endif /* FX_STM32_CUSTOM_DRIVER_H */
