/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/**************************************************************************/

/**
 * @file  fx_user.h
 * @brief FileX user configuration for STM32U0 standalone (no ThreadX).
 *
 * No changes from the original generated file.
 * Key active defines:
 *   FX_STANDALONE_ENABLE  — FileX runs without ThreadX
 *   FX_SINGLE_THREAD      — disables mutex protection (safe for bare-metal)
 *   FX_NO_LOCAL_PATH      — saves RAM (local path feature not needed)
 *   FX_NO_TIMER           — disables timestamp updates (saves resources)
 */

#ifndef FX_USER_H
#define FX_USER_H

/* #define FX_DISABLE_DIRECT_DATA_READ_CACHE_FILL */
/* #define FX_DISABLE_ERROR_CHECKING */
/* #define FX_DONT_UPDATE_OPEN_FILES */
/* #define FX_DRIVER_USE_64BIT_LBA */
/* #define FX_ENABLE_EXFAT */
/* #define FX_ENABLE_FAULT_TOLERANT */
/* #define FX_FAT_MAP_SIZE         128 */
/* #define FX_FAULT_TOLERANT */
/* #define FX_FAULT_TOLERANT_BOOT_INDEX         116 */
/* #define FX_FAULT_TOLERANT_DATA */
/* #define FX_MAX_FAT_CACHE         16 */
/* #define FX_MAX_LAST_NAME_LEN         256 */
/* #define FX_MAX_LONG_NAME_LEN         256 */
/* #define FX_MAX_SECTOR_CACHE         256 */
/* #define FX_MEDIA_DISABLE_SEARCH_CACHE */
/* #define FX_MEDIA_STATISTICS_DISABLE */

#define FX_NO_LOCAL_PATH

#define FX_NO_TIMER

/* #define FX_RENAME_PATH_INHERIT */
/* #define FX_SINGLE_OPEN_LEGACY */

#define FX_SINGLE_THREAD

#define FX_STANDALONE_ENABLE

/* #define FX_UPDATE_RATE_IN_SECONDS         10 */
/* #define FX_UPDATE_RATE_IN_TICKS         1000 */
/* #define FX_DISABLE_CACHE */
/* #define FX_DISABLE_FILE_CLOSE */
/* #define FX_DISABLE_FAST_OPEN */
/* #define FX_DISABLE_FORCE_MEMORY_OPERATION */
/* #define FX_DISABLE_BUILD_OPTIONS */
/* #define FX_DISABLE_ONE_LINE_FUNCTION */
/* #define FX_DISABLE_FAT_ENTRY_REFRESH */
/* #define FX_DISABLE_CONSECUTIVE_DETECT */
/* #define FX_EXFAT_MAX_CACHE_SIZE            512 */
/* #define FX_FAULT_TOLERANT_CACHE_SIZE            1024 */

#endif /* FX_USER_H */
