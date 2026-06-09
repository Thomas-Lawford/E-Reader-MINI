/**
 * @file    spi_sd.c
 * @brief   Low-level SPI SD/SDHC card driver for STM32U0 (SPI1)
 *
 * SD SPI mode protocol:
 *   1. Pull CS low, send command, read response, pull CS high.
 *   2. All data is MSB-first, 8-bit frames.
 *   3. Card must be clocked at ≤400 kHz during init, then up to 25 MHz.
 *
 * SPI1 must already be initialised by CubeMX (CPOL=0, CPHA=0 — SPI Mode 0).
 * Reconfigure to 400 kHz before SD_Init(), then to full speed after.
 */

#include "spi_sd.h"
#include <string.h>

/* -----------------------------------------------------------------------
 * Internal helpers
 * ----------------------------------------------------------------------- */

static void     CS_Low(void)  { HAL_GPIO_WritePin(SD_CS_GPIO_PORT, SD_CS_GPIO_PIN, GPIO_PIN_RESET); }
static void     CS_High(void) { HAL_GPIO_WritePin(SD_CS_GPIO_PORT, SD_CS_GPIO_PIN, GPIO_PIN_SET);   }

/** Send one byte, return the received byte. */
static uint8_t SPI_TxRx(uint8_t tx)
{
    uint8_t rx = 0xFF;
    HAL_SPI_TransmitReceive(&SD_SPI_HANDLE, &tx, &rx, 1, 10);
    return rx;
}

/** Clock out 0xFF bytes until the card de-asserts busy (returns non-0x00). */
static uint8_t SPI_WaitReady(uint32_t timeout_ms)
{
    uint32_t start = HAL_GetTick();
    uint8_t  r;
    do {
        r = SPI_TxRx(0xFF);
        if (r != 0x00) return SD_OK;
    } while ((HAL_GetTick() - start) < timeout_ms);
    return SD_TIMEOUT;
}

/* -----------------------------------------------------------------------
 * SD command helpers
 * ----------------------------------------------------------------------- */

/**
 * Send a 6-byte SD command frame.
 * @param cmd   Command index (0–63)
 * @param arg   32-bit argument
 * @param crc   Pre-computed CRC byte (only required for CMD0 and CMD8)
 * @return      R1 response byte (0x00 = idle/ok, 0xFF = no response)
 */
static uint8_t SD_SendCmd(uint8_t cmd, uint32_t arg, uint8_t crc)
{
    uint8_t r1;
    uint8_t retry;

    /* Wait for card to be ready before issuing next command */
    SPI_WaitReady(500);

    SPI_TxRx(0x40 | cmd);
    SPI_TxRx((uint8_t)(arg >> 24));
    SPI_TxRx((uint8_t)(arg >> 16));
    SPI_TxRx((uint8_t)(arg >> 8));
    SPI_TxRx((uint8_t)(arg));
    SPI_TxRx(crc);

    /* Read R1: card may send up to 8 0xFF bytes before responding */
    for (retry = 0; retry < 8; retry++) {
        r1 = SPI_TxRx(0xFF);
        if (!(r1 & 0x80)) break;  /* MSB clear = valid R1 */
    }
    return r1;
}

/* SD command indices */
#define CMD0    0   /* GO_IDLE_STATE       */
#define CMD8    8   /* SEND_IF_COND        */
#define CMD17   17  /* READ_SINGLE_BLOCK   */
#define CMD24   24  /* WRITE_BLOCK         */
#define CMD55   55  /* APP_CMD             */
#define ACMD41  41  /* SD_APP_OP_COND      */
#define CMD58   58  /* READ_OCR            */

/* R1 status bits */
#define R1_IDLE         0x01
#define R1_ILLEGAL_CMD  0x04

/* -----------------------------------------------------------------------
 * Public API
 * ----------------------------------------------------------------------- */

uint8_t SD_Init(void)
{
    uint8_t  r1;
    uint32_t retry;
    uint8_t  resp[4];

    CS_High();

    /* ---- Step 1: Send ≥74 clock pulses with CS high ---- */
    for (uint8_t i = 0; i < 10; i++) {
        SPI_TxRx(0xFF);
    }

    /* ---- Step 2: CMD0 — reset card to SPI idle mode ---- */
    CS_Low();
    r1 = SD_SendCmd(CMD0, 0, 0x95);   /* CRC for CMD0 with arg=0 */
    CS_High();
    SPI_TxRx(0xFF);

    if (r1 != R1_IDLE) {

    	return SD_ERROR;

    }

    /* ---- Step 3: CMD8 — check voltage range (required for SDHC) ---- */
    CS_Low();
    r1 = SD_SendCmd(CMD8, 0x000001AA, 0x87);  /* VHS=1 (2.7-3.6V), check pattern=0xAA */
    if (r1 == R1_IDLE) {
        /* Read 4-byte R7 response */
        for (uint8_t i = 0; i < 4; i++) resp[i] = SPI_TxRx(0xFF);
        CS_High();
        SPI_TxRx(0xFF);

        /* Verify echo-back: lower byte must be 0xAA, voltage nibble must be 1 */
        if (resp[3] != 0xAA || (resp[2] & 0x0F) != 0x01) {
            return SD_ERROR;
        }
    } else if (r1 & R1_ILLEGAL_CMD) {
        /* SDv1 card — not SDHC, not supported */
        CS_High();
        return SD_ERROR;
    } else {
        CS_High();
        return SD_ERROR;
    }

    /* ---- Step 4: ACMD41 — init loop (up to 1 second) ---- */
    uint32_t start = HAL_GetTick();
    do {
        /* CMD55 must precede every ACMD */
        CS_Low();
        SD_SendCmd(CMD55, 0, 0x65);
        CS_High();
        SPI_TxRx(0xFF);

        CS_Low();
        r1 = SD_SendCmd(ACMD41, 0x40000000, 0x77); /* HCS=1 for SDHC */
        CS_High();
        SPI_TxRx(0xFF);

        if (r1 == 0x00) break;  /* Card left idle state — init done */
    } while ((HAL_GetTick() - start) < 1000);

    if (r1 != 0x00) {
        return SD_TIMEOUT;
    }

    /* ---- Step 5: CMD58 — read OCR, confirm SDHC bit ---- */
    CS_Low();
    r1 = SD_SendCmd(CMD58, 0, 0xFD);
    for (uint8_t i = 0; i < 4; i++) resp[i] = SPI_TxRx(0xFF);
    CS_High();
    SPI_TxRx(0xFF);

    if (r1 != 0x00) return SD_ERROR;
    if (!(resp[0] & 0x40)) return SD_ERROR;  /* CCS bit must be set for SDHC */

    return SD_OK;
}

uint8_t SD_ReadSector(uint32_t sector, uint8_t *buf)
{
    uint8_t  r1;
    uint8_t  token;
    assert_param(hspi1.Instance->CR2 & SPI_CR2_FRXTH);
    uint8_t  dummy_tx[SD_SECTOR_SIZE];
    memset(dummy_tx, 0xFF, SD_SECTOR_SIZE);
   // uint32_t retry;
    if (!(hspi1.Instance->CR1 & SPI_CR1_SPE)) {
        HAL_SPI_Init(&hspi1);  // re-enable if it got disabled
    }
    CS_Low();
    r1 = SD_SendCmd(CMD17, sector, 0xFF);  /* SDHC: address = sector number */
    if (r1 != 0x00) {
        CS_High();
        SPI_TxRx(0xFF);

        return SD_ERROR;
    }

    /* Wait for data token 0xFE (up to 200 ms) */
    uint32_t start = HAL_GetTick();
    do {
        token = SPI_TxRx(0xFF);
        if (token == 0xFE) break;
    } while ((HAL_GetTick() - start) < 200);

    if (token != 0xFE) {
        CS_High();
        SPI_TxRx(0xFF);
        return SD_TIMEOUT;
    }

    /* Read 512 data bytes */
    while (__HAL_SPI_GET_FLAG(&hspi1, SPI_FLAG_BSY));


    memset(buf, 0xAB, SD_SECTOR_SIZE);   // poison pattern

    HAL_StatusTypeDef hal = HAL_SPI_TransmitReceive(
        &hspi1, dummy_tx, buf, SD_SECTOR_SIZE, 500);
    int leading_zeros = 0;
    for (int i = 0; i < 32; i++) {
        if (buf[i] == 0x00) leading_zeros++;
        else break;
    }
    /* Discard 2 CRC bytes */
    SPI_TxRx(0xFF);
    SPI_TxRx(0xFF);

    CS_High();
    SPI_TxRx(0xFF);
    //buf[11] = 0x00;  // bytes per sector low  = 512
    //buf[12] = 0x02;  // bytes per sector high
    return (hal == HAL_OK) ? SD_OK : SD_ERROR;
}

uint8_t SD_WriteSector(uint32_t sector, const uint8_t *buf)
{
    uint8_t r1, resp;

    CS_Low();
    r1 = SD_SendCmd(CMD24, sector, 0xFF);
    if (r1 != 0x00) {
        CS_High();
        SPI_TxRx(0xFF);
        return SD_ERROR;
    }

    /* Send start token */
    SPI_TxRx(0xFE);

    /* Send 512 data bytes */
    for (uint16_t i = 0; i < SD_SECTOR_SIZE; i++) {
        SPI_TxRx(buf[i]);
    }

    /* Send dummy CRC */
    SPI_TxRx(0xFF);
    SPI_TxRx(0xFF);

    /* Read data response token */
    resp = SPI_TxRx(0xFF) & 0x1F;
    if (resp != 0x05) {  /* 0x05 = data accepted */
        CS_High();
        SPI_TxRx(0xFF);
        return SD_ERROR;
    }

    /* Wait for card to finish programming */
    if (SPI_WaitReady(500) != SD_OK) {
        CS_High();
        return SD_TIMEOUT;
    }

    CS_High();
    SPI_TxRx(0xFF);

    return SD_OK;
}
