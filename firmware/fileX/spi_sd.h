/**
 * @file    spi_sd.h
 * @brief   Low-level SPI SD/SDHC card driver for STM32U0 (SPI1)
 *
 * Supports SDHC (block-addressed) cards only.
 * CS pin: configure SPI1_CS_PIN / SPI1_CS_PORT below to match your hardware.
 */

#ifndef SPI_SD_H
#define SPI_SD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32u0xx_hal.h"
#include <stdint.h>

/* -----------------------------------------------------------------------
 * Hardware configuration — adjust to your board
 * ----------------------------------------------------------------------- */
extern SPI_HandleTypeDef hspi1;          /* defined in main.c / spi.c     */

#define SD_SPI_HANDLE       hspi1
#define SD_CS_GPIO_PORT     GPIOA        /* chip-select GPIO port          */
#define SD_CS_GPIO_PIN      GPIO_PIN_4   /* chip-select GPIO pin           */

/* SD card sector size (fixed for FAT) */
#define SD_SECTOR_SIZE      512U

/* -----------------------------------------------------------------------
 * Status codes
 * ----------------------------------------------------------------------- */
#define SD_OK               0x00U
#define SD_ERROR            0x01U
#define SD_TIMEOUT          0x02U

/* -----------------------------------------------------------------------
 * Public API
 * ----------------------------------------------------------------------- */

/**
 * @brief  Initialize the SD card over SPI.
 * @retval SD_OK on success, SD_ERROR or SD_TIMEOUT on failure.
 */
uint8_t SD_Init(void);

/**
 * @brief  Read one 512-byte sector.
 * @param  sector  Logical sector number (byte address = sector * 512 for SDSC,
 *                 or just sector number for SDHC — this driver handles SDHC).
 * @param  buf     Output buffer (must be at least 512 bytes).
 * @retval SD_OK / SD_ERROR / SD_TIMEOUT
 */
uint8_t SD_ReadSector(uint32_t sector, uint8_t *buf);

/**
 * @brief  Write one 512-byte sector.
 * @param  sector  Logical sector number.
 * @param  buf     Input buffer (512 bytes).
 * @retval SD_OK / SD_ERROR / SD_TIMEOUT
 */
uint8_t SD_WriteSector(uint32_t sector, const uint8_t *buf);

#ifdef __cplusplus
}
#endif
#endif /* SPI_SD_H */
