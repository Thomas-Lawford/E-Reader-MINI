/**
 * @file    fx_stm32_custom_driver.c
 * @brief   FileX I/O driver for SPI SDHC on STM32U0
 *
 * Implements all FX_DRIVER_* requests by delegating to spi_sd.c.
 * Sector size is fixed at 512 bytes to match FAT requirements.
 */

#include "fx_stm32_custom_driver.h"
#include "spi_sd.h"
#include <string.h>

/* One 512-byte sector scratch buffer for BOOT_READ/WRITE */
static uint8_t fx_driver_sector_buf[512];
static uint32_t partition_lba_start=0;
VOID fx_stm32_custom_driver(FX_MEDIA *media_ptr)
{
    switch (media_ptr->fx_media_driver_request)
    {
        /* ------------------------------------------------------------------
         * INIT: initialise the SD card.
         * FileX calls this once when fx_media_open() is called.
         * ------------------------------------------------------------------ */
        case FX_DRIVER_INIT:
        {
            if (SD_Init() == SD_OK)
            {
                media_ptr->fx_media_driver_status = FX_SUCCESS;
            }
            else
            {
                media_ptr->fx_media_driver_status = FX_IO_ERROR;
            }
            break;
        }

        /* ------------------------------------------------------------------
         * UNINIT: nothing to do for SPI SD.
         * ------------------------------------------------------------------ */
        case FX_DRIVER_UNINIT:
        {
            media_ptr->fx_media_driver_status = FX_SUCCESS;
            break;
        }

        /* ------------------------------------------------------------------
         * BOOT READ: read sector 0 (the boot/MBR sector).
         * FileX reads this immediately after INIT to parse the FAT BPB.
         * ------------------------------------------------------------------ */
        case FX_DRIVER_BOOT_READ:
        {


            uint8_t sector0[512];
            if (SD_ReadSector(0, sector0) != SD_OK) {
                media_ptr->fx_media_driver_status = FX_IO_ERROR;

                break;
            }

            /* Check for MBR: 0x55AA signature + valid partition type in entry 1 */
            uint8_t part_type = sector0[446 + 4];
            uint16_t sig = (sector0[511] << 8) | sector0[510];

            if (sig == 0xAA55 && part_type != 0x00)
            {
                /* Extract LBA start of partition 1 (little-endian uint32 at offset 454) */
                uint32_t lba_start = (uint32_t)sector0[446 + 8]
                                   | ((uint32_t)sector0[446 + 9]  << 8)
                                   | ((uint32_t)sector0[446 + 10] << 16)
                                   | ((uint32_t)sector0[446 + 11] << 24);
                partition_lba_start=lba_start;

                /* Store so FX_DRIVER_READ can offset all subsequent sector accesses */
                media_ptr->fx_media_hidden_sectors = lba_start;

                /* Now read the actual boot sector */
                if (SD_ReadSector(lba_start, sector0) != SD_OK) {
                    media_ptr->fx_media_driver_status = FX_IO_ERROR;
                    break;

                }
                memcpy(media_ptr->fx_media_driver_buffer, sector0, 512);
            }
            else
            {
                /* No MBR — sector 0 is the BPB directly (superfloppy format) */
                memcpy(media_ptr->fx_media_driver_buffer, sector0, 512);
            }

            media_ptr->fx_media_driver_status = FX_SUCCESS;
           // HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);

            //HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
            break;
        }

        /* ------------------------------------------------------------------
         * READ: read one or more contiguous logical sectors.
         * fx_media_driver_logical_sector  = first sector to read
         * fx_media_driver_sectors         = number of sectors
         * fx_media_driver_buffer          = destination buffer
         * ------------------------------------------------------------------ */
        case FX_DRIVER_READ:
        {//HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
            uint32_t sector = media_ptr->fx_media_driver_logical_sector
                            + partition_lba_start;
            uint32_t count  = media_ptr->fx_media_driver_sectors;
            uint8_t *buf    = media_ptr->fx_media_driver_buffer;

            media_ptr->fx_media_driver_status = FX_SUCCESS;

            for (uint32_t i = 0; i < count; i++)
            {
                if (SD_ReadSector(sector + i, buf + (i * 512U)) != SD_OK)
                {
                    media_ptr->fx_media_driver_status = FX_IO_ERROR;
                    //HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
                    break;

                }
            }
            break;
        }

        /* ------------------------------------------------------------------
         * BOOT WRITE: write sector 0.
         * ------------------------------------------------------------------ */
        case FX_DRIVER_BOOT_WRITE:
        {
            if (SD_WriteSector(0, media_ptr->fx_media_driver_buffer) == SD_OK)
            {
                media_ptr->fx_media_driver_status = FX_SUCCESS;
            }
            else
            {
                media_ptr->fx_media_driver_status = FX_IO_ERROR;

            }
            break;
        }

        /* ------------------------------------------------------------------
         * WRITE: write one or more contiguous logical sectors.
         * ------------------------------------------------------------------ */
        case FX_DRIVER_WRITE:
        {
            uint32_t      sector = media_ptr->fx_media_driver_logical_sector
                                 + media_ptr->fx_media_hidden_sectors;
            uint32_t      count  = media_ptr->fx_media_driver_sectors;
            const uint8_t *buf   = media_ptr->fx_media_driver_buffer;

            media_ptr->fx_media_driver_status = FX_SUCCESS;

            for (uint32_t i = 0; i < count; i++)
            {
                if (SD_WriteSector(sector + i, buf + (i * 512U)) != SD_OK)
                {
                    media_ptr->fx_media_driver_status = FX_IO_ERROR;
                    break;
                }
            }
            break;
        }

        /* ------------------------------------------------------------------
         * FLUSH: no write cache in this driver — nothing to do.
         * ------------------------------------------------------------------ */
        case FX_DRIVER_FLUSH:
        {
            media_ptr->fx_media_driver_status = FX_SUCCESS;
            break;
        }

        /* ------------------------------------------------------------------
         * ABORT: nothing to clean up.
         * ------------------------------------------------------------------ */
        case FX_DRIVER_ABORT:
        {
            media_ptr->fx_media_driver_status = FX_SUCCESS;
            break;
        }

        default:
        {
            media_ptr->fx_media_driver_status = FX_IO_ERROR;
            break;
        }
    }
}
