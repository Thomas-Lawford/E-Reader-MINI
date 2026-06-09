/**
  ******************************************************************************
  * @file    app_filex.h
  * @brief   FileX applicative header — STM32U0 SPI1 SDHC
  ******************************************************************************
  */

#ifndef __APP_FILEX_H__
#define __APP_FILEX_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "fx_api.h"
#include "fx_stm32_custom_driver.h"

/* Exported function prototypes ----------------------------------------------*/

/** Initialise the FileX subsystem. Call once before MX_FileX_Run(). */
UINT MX_FileX_Init(void);

/**
 * Open the SD card, find the first .txt file in the root directory,
 * read its name and first 20 characters.
 *
 * On return inspect:
 *   g_result        — FX_SUCCESS or a FileX error code
 *   g_txt_filename  — null-terminated filename (valid when g_result == FX_SUCCESS)
 *   g_txt_preview   — first ≤20 chars, null-terminated (valid when g_result == FX_SUCCESS)
 */
UINT MX_FileX_GetTextFiles(char g_txt_filenames[6][256], UINT *file_count);
//UINT MX_FileX_Run(void);
UINT MX_FileX_ReadRange(unsigned int file_index,unsigned int start, unsigned int end);
//UINT MX_FileX_ReadRange(const char *filename, unsigned int start, unsigned int end);

/* Exported result variables -------------------------------------------------*/
extern char g_txt_filename[256];  /**< Name of the first .txt file found   */
extern char g_txt_preview[1000];    /**< First 20 characters of that file    */
extern UINT g_result;             /**< FX_SUCCESS or FileX error code      */

#ifdef __cplusplus
}
#endif
#endif /* __APP_FILEX_H__ */
