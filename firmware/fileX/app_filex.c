/**
 * @file    app_filex.c
 * @brief   FileX application — STM32U0, SPI1 SDHC
 *
 * After initialisation, MX_FileX_Run():
 *   1. Opens the FAT media on the SD card.
 *   2. Finds the first .txt file in the root directory.
 *   3. Reads the first 20 characters of that file.
 *   4. Stores results in g_txt_filename[] and g_txt_preview[].
 *
 * Call MX_FileX_Run() once after MX_FileX_Init() — e.g. from main().
 * Inspect g_result for the outcome.
 */

#include "app_filex.h"
#include <string.h>
#include "stdio.h"
//#include "../../Core/SRC/EPD/Display_EPD_W21.h"
//#include "../../Core/SRC/EPD/Display_EPD_W21_spi.h"
//#include "../../Core/SRC/GUI/GUI_Paint.h"
//#include "../../Core/SRC/Fonts/fonts.h"
//#define IMAGE_SIZE (EPD_WIDTH * EPD_HEIGHT / 8)
//UBYTE ImageBuffer2[IMAGE_SIZE];
/* ---------------------------------------------------------------------------
 * FileX working objects
 * --------------------------------------------------------------------------- */

/* Media object */
static FX_MEDIA  sd_media;

/* Sector cache — must be at least one sector (512 bytes).
 * Larger cache improves directory-scan performance.              */
#define MEDIA_CACHE_SIZE   (2U * 512U)
static uint8_t   media_cache[MEDIA_CACHE_SIZE] __attribute__((aligned(4)));

/* Directory entry scratch object */
static FX_LOCAL_PATH  local_path;   /* unused — FX_NO_LOCAL_PATH is set,
                                       but we keep the open/close symmetric */

/* ---------------------------------------------------------------------------
 * Public result variables (read after MX_FileX_Run() returns FX_SUCCESS)
 * --------------------------------------------------------------------------- */

/** Null-terminated name of the first .txt file found (up to FX_MAX_LONG_NAME_LEN). */
char    g_txt_filename[256];

/** First 20 characters of the file, NOT null-terminated by FileX;
 *  we add a null terminator so it can be used as a C string.    */
char    g_txt_preview[1000];

/** Overall result code — FX_SUCCESS or a FileX error code. */
UINT    g_result = FX_IO_ERROR;

/* ---------------------------------------------------------------------------
 * Helper: case-insensitive check for ".txt" extension
 * --------------------------------------------------------------------------- */
static int has_txt_extension(const char *name)
{
    size_t len = strlen(name);
    if (len < 4) return 0;
    const char *ext = name + len - 4;
    return (ext[0] == '.' &&
           (ext[1] == 't' || ext[1] == 'T') &&
           (ext[2] == 'x' || ext[2] == 'X') &&
           (ext[3] == 't' || ext[3] == 'T'));
}

/* ---------------------------------------------------------------------------
 * MX_FileX_Init  — called once at startup
 * --------------------------------------------------------------------------- */
UINT MX_FileX_Init(void)
{
    fx_system_initialize();
    return FX_SUCCESS;
}

/* ---------------------------------------------------------------------------
 * MX_FileX_Run  — open media, find first .txt, read 20 chars
 * --------------------------------------------------------------------------- */



UINT MX_FileX_GetTextFiles(char g_txt_filenames[6][256], UINT *file_count)
{
    UINT   status;
    char   entry_name[256];
    UINT   attributes;
    ULONG  size;
    UINT   year, month, day, hour, minute, second;

    *file_count = 0;

    /* ---- 1. Open media --------------------------------------------------- */
    status = fx_media_open(&sd_media,
                           "SD",
                           fx_stm32_custom_driver,
                           FX_NULL,
                           media_cache,
                           MEDIA_CACHE_SIZE);
    if (status != FX_SUCCESS)
        return status;

    /* ---- 2. Scan root directory ------------------------------------------ */
    status = fx_directory_first_full_entry_find(
                 &sd_media, entry_name, &attributes, &size,
                 &year, &month, &day, &hour, &minute, &second);

    while (status == FX_SUCCESS && *file_count < 6)
    {
        if (!(attributes & FX_DIRECTORY))
        {
            /* Check for .txt extension (case-insensitive) */
            size_t len = strlen(entry_name);
            if (has_txt_extension(entry_name))
            {
                strncpy(g_txt_filenames[*file_count], entry_name, len - 4);
                g_txt_filenames[*file_count][len-4] = '\0';
                (*file_count)++;
            }
        }

        status = fx_directory_next_full_entry_find(
                     &sd_media, entry_name, &attributes, &size,
                     &year, &month, &day, &hour, &minute, &second);
    }

    fx_media_close(&sd_media);

    if (*file_count == 0)
        return FX_NOT_FOUND;

    return FX_SUCCESS;
}
/*
UINT MX_FileX_ReadRange(const char *filename2, unsigned int start, unsigned int end)
{
    UINT   status;
    ULONG  range_len;
    ULONG  bytes_read = 0;
//    size_t len = sizeof(filename);
    char filename[250];
    snprintf(filename, sizeof(filename),
             "%s.txt", filename2);
    if (filename == NULL || filename[0] == '\0')
    {
        g_result = FX_INVALID_STATE;
        return FX_INVALID_STATE;
    }

    if (start >= end)
    {
        g_result = FX_INVALID_STATE;
        return FX_INVALID_STATE;
    }

    range_len = (ULONG)(end - start);

    if (range_len >= sizeof(g_txt_preview))
    {
        g_result = FX_BUFFER_ERROR;
        return FX_BUFFER_ERROR;
    }

    status = fx_media_open(&sd_media,
                           "SD",
                           fx_stm32_custom_driver,
                           FX_NULL,
                           media_cache,
                           MEDIA_CACHE_SIZE);
    if (status != FX_SUCCESS)
    {
        g_result = status;
        return status;
    }

    strncpy(g_txt_filename, filename, sizeof(g_txt_filename) - 1);
    g_txt_filename[sizeof(g_txt_filename) - 1] = '\0';

    FX_FILE txt_file;

    status = fx_file_open(&sd_media, &txt_file, g_txt_filename, FX_OPEN_FOR_READ);
    if (status != FX_SUCCESS)
    {
        fx_media_close(&sd_media);
        g_result = status;
        return status;
    }

    if (start > 0)
    {
        status = fx_file_seek(&txt_file, (ULONG)start);
        if (status != FX_SUCCESS)
        {
            fx_file_close(&txt_file);
            fx_media_close(&sd_media);
            g_result = status;
            return status;
        }
    }

    memset(g_txt_preview, 0, sizeof(g_txt_preview));

    status = fx_file_read(&txt_file, g_txt_preview, range_len, &bytes_read);
    if (status != FX_SUCCESS && status != FX_END_OF_FILE)
    {
        fx_file_close(&txt_file);
        fx_media_close(&sd_media);
        g_result = status;
        return status;
    }

    g_txt_preview[bytes_read] = '\0';

    ULONG file_size = txt_file.fx_file_current_file_size;
    if (range_len > 0 && file_size > 0)
    {
        ULONG current_chunk = (start / range_len) + 1;
        ULONG total_chunks  = file_size / range_len;

        char suffix[12];
        int written = snprintf(suffix, sizeof(suffix), "%lu/%lu", current_chunk, total_chunks);

        if (written > 0 && written < (int)(sizeof(suffix) - 1))
        {
            int pad = (sizeof(suffix) - 1) - written;
            memmove(suffix + pad, suffix, written + 1);
            memset(suffix, ' ', pad);
        }

        strncat(g_txt_preview, suffix, sizeof(g_txt_preview) - bytes_read - 1);
    }

    fx_file_close(&txt_file);
    fx_media_close(&sd_media);

    g_result = FX_SUCCESS;
    return FX_SUCCESS;
}

//*/

UINT MX_FileX_ReadRange(unsigned int file_index, unsigned int start, unsigned int end){

    UINT   status;
    char   entry_name[256];
    UINT   attributes;
    ULONG  size;
    UINT   year, month, day, hour, minute, second;
    char g_txt_filenames[6][256];
    UINT file_count = 0;
char filename2[256];
/*Paint_NewImage(ImageBuffer2, EPD_WIDTH, EPD_HEIGHT, ROTATE_270, WHITE);
	  Paint_SetMirroring(MIRROR_HORIZONTAL);
	  Paint_Clear(WHITE);
	char buf[100];
	snprintf(buf, sizeof(buf), " Go to page:");
	Paint_DrawString_EN(0, 10, buf, &Font16, WHITE, BLACK);
 EPD_SetRAMValue_BaseMap(ImageBuffer2);
 EPD_DeepSleep(2);
	  HAL_Delay(7000);*/
    // ---- 1. Open media ---------------------------------------------------

         status = fx_media_open(&sd_media,
                           "SD",
                           fx_stm32_custom_driver,
                           FX_NULL,
                           media_cache,
                           MEDIA_CACHE_SIZE);
    if (status != FX_SUCCESS)
        return status;


    // ---- 2. Scan root directory ------------------------------------------
    status = fx_directory_first_full_entry_find(
                 &sd_media, entry_name, &attributes, &size,
                 &year, &month, &day, &hour, &minute, &second);

    while (status == FX_SUCCESS && file_count < 6)
    {
        if (!(attributes & FX_DIRECTORY))
        {
            // Check for .txt extension (case-insensitive)
            size_t len = strlen(entry_name);
            if (has_txt_extension(entry_name))
            {
                strncpy(g_txt_filenames[file_count], entry_name, len - 4);
                g_txt_filenames[file_count][len-4] = '\0';
                file_count++;
            }
        }

        status = fx_directory_next_full_entry_find(
                     &sd_media, entry_name, &attributes, &size,
                     &year, &month, &day, &hour, &minute, &second);
    }

    //fx_media_close(&sd_media);

    if (file_count == 0)
        return FX_NOT_FOUND;
//UINT   status;
   ULONG  range_len;
   ULONG  bytes_read = 0;
  // size_t len = sizeof(filename);
   char filename[256];
   snprintf(filename, sizeof(filename),
            "%s.txt", g_txt_filenames[file_index]);
   if (filename == NULL || filename[0] == '\0')
   {
       g_result = FX_INVALID_STATE;
       return FX_INVALID_STATE;
   }

   if (start >= end)
   {
       g_result = FX_INVALID_STATE;
       return FX_INVALID_STATE;
   }

   range_len = (ULONG)(end - start);

   if (range_len >= sizeof(g_txt_preview))
   {
       g_result = FX_BUFFER_ERROR;
       return FX_BUFFER_ERROR;
   }

   /*status = fx_media_open(&sd_media,
                          "SD",
                          fx_stm32_custom_driver,
                          FX_NULL,
                          media_cache,
                          MEDIA_CACHE_SIZE);
   if (status != FX_SUCCESS)
   {
       g_result = status;
       return status;
   }*/

   strncpy(g_txt_filename, filename, sizeof(g_txt_filename) - 1);
   g_txt_filename[sizeof(g_txt_filename) - 1] = '\0';

   FX_FILE txt_file;

   status = fx_file_open(&sd_media, &txt_file, g_txt_filename, FX_OPEN_FOR_READ);
   if (status != FX_SUCCESS)
   {
       fx_media_close(&sd_media);
       g_result = status;
       return status;
   }

   if (start > 0)
   {
       status = fx_file_seek(&txt_file, (ULONG)start);
       if (status != FX_SUCCESS)
       {
           fx_file_close(&txt_file);
           fx_media_close(&sd_media);
           g_result = status;
           return status;
       }
   }

   memset(g_txt_preview, 0, sizeof(g_txt_preview));

   status = fx_file_read(&txt_file, g_txt_preview, range_len, &bytes_read);
   if (status != FX_SUCCESS && status != FX_END_OF_FILE)
   {
       fx_file_close(&txt_file);
       fx_media_close(&sd_media);
       g_result = status;
       return status;
   }

   g_txt_preview[bytes_read] = '\0';

   ULONG file_size = txt_file.fx_file_current_file_size;
   if (range_len > 0 && file_size > 0)
   {
       ULONG current_chunk = (start / range_len) + 1;
       ULONG total_chunks  = file_size / range_len;

       char suffix[12];
       int written = snprintf(suffix, sizeof(suffix), "%lu/%lu", current_chunk, total_chunks);

       if (written > 0 && written < (int)(sizeof(suffix) - 1))
       {
           int pad = (sizeof(suffix) - 1) - written;
           memmove(suffix + pad, suffix, written + 1);
           memset(suffix, ' ', pad);
       }

       strncat(g_txt_preview, suffix, sizeof(g_txt_preview) - bytes_read - 1);
   }

   fx_file_close(&txt_file);
   fx_media_close(&sd_media);

   //g_result = FX_SUCCESS;
   return FX_SUCCESS;
}

//*/
