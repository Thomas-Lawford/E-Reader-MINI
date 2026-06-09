/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "app_filex.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "EPD/Display_EPD_W21.h"
#include "EPD/Display_EPD_W21_spi.h"
#include "GUI/GUI_Paint.h"
#include "Fonts/fonts.h"
//#include "fatfs.h"
//#include "spi.h"
//#include "sd_filereader.h"
#include "stdio.h"
//#include "sd_spi.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define IMAGE_SIZE (EPD_WIDTH * EPD_HEIGHT / 8)
UBYTE ImageBuffer[IMAGE_SIZE];
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define NDEBUG

// If using HAL, disable assert_param checks
#define USE_FULL_ASSERT 0
#define PVD_THRESHOLD_LEVEL  PWR_PVDLEVEL_4//4
#define COUNTER_BKP_REG     0   /* RTC->BKP0R = counter */
//#define STRING_BKP_START    1
#define BACKUP_STR_START   0   // BKP0R
#define BACKUP_STR_WORDS   16  // 64 bytes
//#define BACKUP_COUNTER     16  // BKP16R
#define BACKUP_page       17
#define BACKUP_menu       18

#define RETAINED_MAGIC 1
#define BACKUP_MAGIC     20
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

RTC_HandleTypeDef hrtc;

SPI_HandleTypeDef hspi1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_RTC_Init(void);
/* USER CODE BEGIN PFP */
/* Configure Wakeup pins */
/* FWD_Pin = PWR_WAKEUP_PIN1 (PA0) - falling edge (high to low) */
static void PVD_Config(void);
static void enter_standby(uint8_t high);
static void Configure_RTC_WakeupTimer(uint32_t seconds);
//static void SystemClock_Config_LowPower(void);
//static void SystemClock_Config_Normal(void);
//static void enable_backup_access(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
	//__HAL_RCC_SYS_DBGMCU_CLK_ENABLE();
	DBGMCU->CR = 0x00; // Disable all debug features

	/* Disable debug MCU in low-power modes (very impactful!) */
	//DBGMCU->CR &= ~DBGMCU_CR_DBG_SLEEP;
	DBGMCU->CR &= ~DBGMCU_CR_DBG_STOP;
	DBGMCU->CR &= ~DBGMCU_CR_DBG_STANDBY;

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_FileX_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_RTC_WAKEUPTIMER_CLEAR_FLAG(&hrtc, RTC_FLAG_WUTF);
  unsigned int retained_magic;
  char     retained_string[28];
  unsigned int menu;
  unsigned int page;
  unsigned int num;
  unsigned int settings;
  unsigned int fontsize;
  unsigned int exits=0;
  //HAL_PWREx_EnableSRAMContentRetention();
  HAL_PWR_EnableBkUpAccess();
  //HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN1);
  retained_magic = READ_REG(TAMP->BKP8R);
  settings=((READ_REG(TAMP->BKP8R)&4)==4);
  num=(READ_REG(TAMP->BKP8R)&120)>>3;
  page=READ_REG(TAMP->BKP7R);
  menu=READ_REG(TAMP->BKP8R)&2;
  fontsize=READ_REG(TAMP->BKP6R);
  unsigned int temp=(READ_REG(TAMP->BKP8R)&896)>>7;
  //unsigned int EPDreset=
  unsigned int stuff =READ_REG(TAMP->BKP8R);
  uint8_t first_boot = ((retained_magic & 1) != 1);
  char    filenames[6][256];
  UINT    file_count = 0;
  while (__HAL_PWR_GET_FLAG(PWR_FLAG_VOSF)) { }
  PVD_Config();
  if (__HAL_PWR_GET_FLAG(PWR_FLAG_PVDO)){
	  EPD_HW_Init_CustomLUT();
	  Paint_NewImage(ImageBuffer, EPD_WIDTH, EPD_HEIGHT, ROTATE_270, WHITE);
	  Paint_SetMirroring(MIRROR_HORIZONTAL);
	  Paint_Clear(WHITE);
      Paint_DrawString_EN(10, 50, "LOW BATT", &Font16, WHITE, BLACK);
      EPD_SetRAMValue_BaseMap(ImageBuffer);
      EPD_DeepSleep(2);
      CLEAR_BIT(PWR->PDCRB, PWR_PDCRB_PD7);
      WRITE_REG(PWR->SCR, PWR_SCR_CWUF1 | PWR_SCR_CWUF3 | PWR_SCR_CSBF|RTC_SCR_CWUTF);
      HAL_PWR_EnterSHUTDOWNMode();   // never returns
  }
  if (first_boot){
	 /* EPD_GPIO_Init();
	/* uncomment only when ready
	  //EPD_HW_Init();
	  //EPD_HW_Init_Fast(0);
	  EPD_HW_Init_CustomLUT();
	  Paint_NewImage(ImageBuffer, EPD_WIDTH, EPD_HEIGHT, ROTATE_270, WHITE);
	  Paint_SetMirroring(MIRROR_HORIZONTAL);
	  Paint_Clear(WHITE);
  	char buf[100];
  	snprintf(buf, sizeof(buf), "HHIlo");
  	Paint_DrawString_EN(0, 10, buf, &Font24, WHITE, BLACK);
    EPD_SetRAMValue_BaseMap(ImageBuffer);
    EPD_DeepSleep(2);
    //SystemClock_Config_LowPower();
      //HAL_PWREx_EnableLowPowerRunMode();
      HAL_Delay(2000);
      //HAL_PWREx_DisableLowPowerRunMode();
      //SystemClock_Config_Normal();//*/
	  fontsize=2;
      menu=2;
	  page=0;
	  temp=0;
      //memset((void *)retained_string, 0, sizeof(retained_string));
      WRITE_REG(TAMP->BKP8R, 3);
      g_result=MX_FileX_GetTextFiles(filenames, &file_count);
      HAL_GPIO_WritePin(GPIOB, SD_ON_Pin, GPIO_PIN_SET);
   }
   uint32_t sr1 = READ_REG(PWR->SR1);  /* read before clearing */
   WRITE_REG(PWR->SCR, PWR_SCR_CWUF1 | PWR_SCR_CWUF3 | PWR_SCR_CSBF);

   if (!first_boot){

	   //int negtive=0;
       if ((sr1 & PWR_SR1_WUF1)==PWR_SR1_WUF1){//if fwd increment stuff   //HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
    	  if(!settings){
    		  if(menu==2){
    			  temp=temp+1;
    		  }else{
        		  page=page+1;
    		  }
    	  }else{// if in settings increment number selector
    		  num=num+1;
    	  }

       }else if ((sr1 & PWR_SR1_WUF3)==PWR_SR1_WUF3){//back pin, decrement relevant value
       if(!settings){
    	  if(menu!=2){
    		  if (page!=0){
    	      page=page-1;}
    	  }else{
    		  temp=temp-1;
    	  }
       }else{
    		  num=num-1;
       }
       }else{//woken up by timer, release timer and sd card and long sleep
    	   //EPD_DeepSleep(2);
    	  HAL_RTCEx_DeactivateWakeUpTimer(&hrtc);
    	  enter_standby(1);}
       if(menu==2||settings){ //if menu
          if(menu==2){g_result=MX_FileX_GetTextFiles(filenames, &file_count);}else{SystemClock_Config_LowPower();
          HAL_PWREx_EnableLowPowerRunMode();
          HAL_Delay(70);
          HAL_PWREx_DisableLowPowerRunMode();
          SystemClock_Config_Normal();
          g_result=FX_SUCCESS;}
          //HAL_GPIO_WritePin(GPIOB, SD_ON_Pin, GPIO_PIN_SET);
          //SystemClock_Config_LowPower();
          //HAL_PWREx_EnableLowPowerRunMode();
          //HAL_Delay(300);
          //HAL_PWREx_DisableLowPowerRunMode();
          //SystemClock_Config_Normal();
        	if(HAL_GPIO_ReadPin(FWD_GPIO_Port, FWD_Pin) == GPIO_PIN_RESET){
        		menu=0;
        		if(!settings){//enter book
        			//HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
        			  if (temp>file_count){///
        					temp=file_count;
        			  }
        			strncpy(filenames[0], filenames[temp-1], sizeof(filenames[temp-1]) - 1);
        			filenames[0][sizeof(filenames[temp-1]) - 2] = '\0';
        			temp=temp-1;
        			WRITE_REG(TAMP->BKP8R, 1|(temp<<7));
        		    page=0;
        		    fontsize=fontsize+1;
        		}else{//add page digit
        			num=num-1;
        			page=page*10+num;
        		}
        	}//fwd double press
        	if(HAL_GPIO_ReadPin(BCK_GPIO_Port, BCK_Pin) == GPIO_PIN_RESET){
        		menu=0;
        		if(!settings){//enter settings first time
	      	      	temp=temp+1;
	      	      	WRITE_REG(TAMP->BKP8R, (5|(temp<<7)));
        		    settings=1;
        		    page=0;
        		}else{//enter book and exit settings
      			  //if (temp>=file_count){///
      				//	temp=file_count-1;
      			  //}
      			g_result=MX_FileX_GetTextFiles(filenames, &file_count);
        			settings=0;//hangs here
        			WRITE_REG(TAMP->BKP8R, (1|(temp<<7)));
            		strncpy(filenames[0], filenames[temp], sizeof(filenames[temp]) - 1);
            		filenames[0][sizeof(filenames[temp]) - 1] = '\0';
            		fontsize=fontsize+2;
            		exits=1;
        		}
        	}//if double press back


       }// if menu


  }//not first boot
   if (settings){
		menu=0;//make sure settings gets drawn if in settings and not menu
	}
  WRITE_REG(TAMP->BKP7R, page);
  if(num>9){num=0;}

  //num=;
  WRITE_REG(TAMP->BKP8R, (num<<3)|(READ_REG(TAMP->BKP8R)&7)|(temp<<7));

   // paintsection no logic allowed below
  EPD_GPIO_Init();
  if(menu||settings){

	  //EPD_DeepSleep(0);
	  EPD_HW_Init_CustomLUT();
	  //EPD_HW_Init_Fast(5);
  }else{
	  //EPD_DeepSleep(0);
	  EPD_HW_Init_CustomLUT();
  //EPD_HW_Init_Fast(fontsize);
  }
// uncomment only when ready
  //EPD_HW_Init_Fast();
  Paint_NewImage(ImageBuffer, EPD_WIDTH, EPD_HEIGHT, ROTATE_270, WHITE);
  Paint_SetMirroring(MIRROR_HORIZONTAL);
  Paint_Clear(WHITE);

  if(menu==2){//draw menu
	  if (g_result == FX_SUCCESS){
		  for (int i =0; i<file_count; i++){
      		  char buf[256];
      	      snprintf(buf, sizeof(buf)," %s\r\n", filenames[i]);
      	      if (i==temp){Paint_DrawString_EN(0, 32*i, buf, &Font16, BLACK, WHITE);}else{
      	    	  Paint_DrawString_EN(0, 32*i, buf, &Font16, WHITE, BLACK);
      	      }
		  }
	  }else{
		  char buf[32];
      	  snprintf(buf, sizeof(buf), " SD error (%d)\r\n", g_result);
   		  Paint_DrawString_EN(10, 10, buf, &Font16, WHITE, BLACK);
   	      (void)g_result;
      }
  }else{//draw pages
		if(!settings){
			unsigned int up=0;
			unsigned int down=0;
		    //g_result=MX_FileX_GetTextFiles(filenames, &file_count);
	        SystemClock_Config_LowPower();
	          HAL_PWREx_EnableLowPowerRunMode();
	          HAL_Delay(70);
	          HAL_PWREx_DisableLowPowerRunMode();
	          SystemClock_Config_Normal();
	          g_result=FX_SUCCESS;
        	if(HAL_GPIO_ReadPin(FWD_GPIO_Port, FWD_Pin) == GPIO_PIN_RESET){

				fontsize=fontsize+1;
				up=1;
				if (fontsize>4){
					fontsize=4;
        	}
        	}
        	if(HAL_GPIO_ReadPin(BCK_GPIO_Port, BCK_Pin) == GPIO_PIN_RESET){
        		if(!exits){
        	    if (fontsize>0){fontsize=fontsize-1;}
        	    down=1;}
        	}
			WRITE_REG(TAMP->BKP6R, fontsize);

    		strncpy(filenames[0], filenames[temp], sizeof(filenames[temp]) - 1);
    		filenames[0][sizeof(filenames[temp]) - 1] = '\0';//*/
			if (g_result == FX_SUCCESS){//

				char buf[1000];
			    if (fontsize==0){
			    	if (down){
			    		page=((page+1)*389/829);
			    	}
				//MX_FileX_ReadRange(filenames[0],page*829, (page+1)*829);
			    	MX_FileX_ReadRange(temp,page*829, (page+1)*829);
				snprintf(buf, sizeof(buf)," %s\r\n", g_txt_preview);
				Paint_DrawString_EN(0, 0, buf, &Font8, WHITE, BLACK);
				Paint_DrawString_EN(0, 0, buf, &Font8, WHITE, BLACK);
				}else if (fontsize==1){
			    	if (down){
			    		page=((page+1)*181/389);
			    	}if (up){
						if (page!=0){page=page-1;}
						page=((page)*829/389);
					}
			    	//MX_FileX_ReadRange(filenames[0],page*389, (page+1)*389);
					MX_FileX_ReadRange(temp,page*389, (page+1)*389);
				snprintf(buf, sizeof(buf)," %s\r\n", g_txt_preview);
				Paint_DrawString_EN(0, 0, buf, &Font12, WHITE, BLACK);
				Paint_DrawString_EN(0, 0, buf, &Font12, WHITE, BLACK);

			    }else if (fontsize==2){
			    	if (down){
			    		page=((page+1)*97/181);
			    	}if (up){
						if (page!=0){page=page-1;}
			    		page=((page)*389/181);

					}
			    //	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
			    	//MX_FileX_ReadRange(filenames[0],page*181, (page+1)*181);
			    	MX_FileX_ReadRange(temp,page*181, (page+1)*181);
			    	snprintf(buf, sizeof(buf)," %s\r\n", g_txt_preview);

			    	Paint_DrawString_EN(0, 0, buf, &Font16, WHITE, BLACK);
				Paint_DrawString_EN(0, 0, buf, &Font16, WHITE, BLACK);
				}else if(fontsize==3){
					if (down){
						page=((page+1)*97/181);
					}if (up){
						if (page!=0){page=page-1;}
						page=((page)*181/97);
					}
					//HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
					//MX_FileX_ReadRange(filenames[0],page*97, (page+1)*97);
					MX_FileX_ReadRange(temp,page*97, (page+1)*97);
					snprintf(buf, sizeof(buf)," %s\r\n", g_txt_preview);
				Paint_DrawString_EN(0, 0, buf, &Font20, WHITE, BLACK);
				Paint_DrawString_EN(0, 0, buf, &Font20, WHITE, BLACK);
				}else{
					if (up){
						if (page!=0){page=page-1;}
						page=((page)*97/69);
					}
					//MX_FileX_ReadRange(filenames[0],page*69, (page+1)*69);
					MX_FileX_ReadRange(temp,page*69, (page+1)*69);
				snprintf(buf, sizeof(buf)," %s\r\n", g_txt_preview);
				Paint_DrawString_EN(0, 0, buf, &Font24, WHITE, BLACK);
				Paint_DrawString_EN(0, 0, buf, &Font24, WHITE, BLACK);
			    }
				WRITE_REG(TAMP->BKP7R, page);

				(void)g_txt_filename;
				(void)g_txt_preview;
				if (g_result != FX_SUCCESS){
					char buf[32];
					snprintf(buf, sizeof(buf), "  SD error (%d)\r\n", g_result);
					Paint_DrawString_EN(10, 10, buf, &Font16, WHITE, BLACK);
					(void)g_result;
					WRITE_REG(TAMP->BKP8R, 0);
				}
			}else{//pretty sure it never gets here
			char buf[32];
			snprintf(buf, sizeof(buf), "  SD error (%d)\r\n", g_result);
				Paint_DrawString_EN(10, 10, buf, &Font16, WHITE, BLACK);
				(void)g_result;
				WRITE_REG(TAMP->BKP8R, 0);
			}
		}else{//draw settings
        	char buf[400];
        	snprintf(buf, sizeof(buf), " Go to page:(%d)\r\n", page);
        	Paint_DrawString_EN(0, 0, buf, &Font16, WHITE, BLACK);
        	for(int i =0; i<10; i++){
        		snprintf(buf, sizeof(buf), " %d\r\n", i);
        		if(num==i){Paint_DrawString_EN((16*i)+10, 30,buf, &Font16, BLACK, WHITE);
        	    }else{
        	    	Paint_DrawString_EN((16*i)+10, 30, buf, &Font16, WHITE, BLACK);
        	    }
        	}
        	snprintf(buf, sizeof(buf), " XS = 0.2182      S = 0.4653      M = 1.0000      L = 1.8660     XL = 2.6231");
        	Paint_DrawString_EN(0, 60, buf, &Font16, WHITE, BLACK);
        }
  }
  //EPD_Dis_PartAll(ImageBuffer);
  //EPD_WhiteScreen_ALL_Fast(ImageBuffer);
  EPD_SetRAMValue_BaseMap(ImageBuffer);
  EPD_DeepSleep(2);
  __HAL_RTC_WAKEUPTIMER_CLEAR_FLAG(&hrtc, RTC_FLAG_WUTF);
  Configure_RTC_WakeupTimer(40);
  enter_standby(0);


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1){
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_7;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  hrtc.Init.OutPutPullUp = RTC_OUTPUT_PULLUP_NONE;
  hrtc.Init.BinMode = RTC_BINARY_NONE;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0x0;
  sTime.Minutes = 0x0;
  sTime.Seconds = 0x0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 0x1;
  sDate.Year = 0x0;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable the WakeUp
  */
  if (HAL_RTCEx_SetWakeUpTimer(&hrtc, 0, RTC_WAKEUPCLOCK_RTCCLK_DIV16) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, DEBUG_LED_Pin|EPD_DC_Pin|EPD_RES_Pin|SD_ON_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : SD_CS_Pin */
  GPIO_InitStruct.Pin = SD_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SD_CS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : DEBUG_LED_Pin EPD_DC_Pin EPD_RES_Pin SD_ON_Pin */
  GPIO_InitStruct.Pin = DEBUG_LED_Pin|EPD_DC_Pin|EPD_RES_Pin|SD_ON_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : EPD_BUSY_Pin */
  GPIO_InitStruct.Pin = EPD_BUSY_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(EPD_BUSY_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SD_CD_Pin */
  GPIO_InitStruct.Pin = SD_CD_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(SD_CD_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : EPD_CS_Pin */
  GPIO_InitStruct.Pin = EPD_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
  HAL_GPIO_Init(EPD_CS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : EPD_SCK_Pin EPD_MOSI_Pin */
  GPIO_InitStruct.Pin = EPD_SCK_Pin|EPD_MOSI_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  /* MISO — pull-up to prevent floating bus during SD init \/
  GPIO_InitStruct.Pin = SD_MISO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);*/
  GPIO_InitStruct.Pin  = FWD_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;  // plain input, no EXTI
  GPIO_InitStruct.Pull = GPIO_NOPULL;      // you have external pull-ups
  HAL_GPIO_Init(FWD_GPIO_Port, &GPIO_InitStruct);
  GPIO_InitStruct.Pin  = BCK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;  // plain input, no EXTI
  GPIO_InitStruct.Pull = GPIO_NOPULL;      // you have external pull-ups
  HAL_GPIO_Init(FWD_GPIO_Port, &GPIO_InitStruct);
  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
static void PVD_Config(void)
{
    PWR_PVDTypeDef pvdConfig = {0};

    pvdConfig.PVDLevel = PWR_PVDLEVEL_4; //4  = ~2.6V threshold
    pvdConfig.Mode    = PWR_PVD_MODE_NORMAL; // polling mode, no interrupt/EXTI

    HAL_PWR_ConfigPVD(&pvdConfig);
    HAL_PWR_EnablePVD();
}
void SystemClock_Config_LowPower(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Drop to voltage scale 2 (already set in your normal config, but be explicit) */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Switch MSI to 1 MHz (RCC_MSIRANGE_4)
   *  Valid LPR ranges: _0=100kHz, _1=200kHz, _2=400kHz, _3=800kHz, _4=1MHz, _5=2MHz
   *  LSI stays on if you need RTC/IWDG/LPTIM wakeup */
  RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSIState            = RCC_LSI_ON;
  RCC_OscInitStruct.MSIState            = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSIClockRange       = RCC_MSIRANGE_0;  // 1 MHz
  RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_NONE;    // PLL must be off for LPR
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** AHB/APB dividers: keep at /1 so HCLK = 1 MHz */
  RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                   | RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  /** At 1 MHz, zero flash wait states are needed */
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }

  /** NOW it's safe to enable Low Power Run mode */
  HAL_PWREx_EnableLowPowerRunMode();
}
void SystemClock_Config_Normal(void)
{
  /** Must disable LPR before raising the clock back up */
  if (HAL_PWREx_DisableLowPowerRunMode() != HAL_OK)
  {
    Error_Handler();
  }

  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSIState            = RCC_LSI_ON;
  RCC_OscInitStruct.MSIState            = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSIClockRange       = RCC_MSIRANGE_7;  // back to 16 MHz
  RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                   | RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  /** Back to 1 wait state for 16 MHz */
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}
/** Call this to return to full 16 MHz operation */
static void Configure_RTC_WakeupTimer(uint32_t seconds)
{
    HAL_RTCEx_DeactivateWakeUpTimer(&hrtc);
HAL_Delay(1);//taiting for WUTWF=1

    // * RTC_WAKEUPCLOCK_CK_SPRE_16BITS selects the 1 Hz ck_spre source
    // * and gives a 16-bit counter → up to 65535 s range.
    // * Counter value = desired seconds - 1.
    //HAL_RTCEx_SetWakeUpTimer_IT()
    if (HAL_RTCEx_SetWakeUpTimer_IT(&hrtc,
                                    seconds - 1,
                                    RTC_WAKEUPCLOCK_CK_SPRE_16BITS, 1) != HAL_OK)
    {
        Error_Handler();
    }//*/
}
/* ------------------------------------------------------------------ */
static void enter_standby(uint8_t high)
{
    /* Clear any stale wakeup flags before arming */
    WRITE_REG(PWR->SCR, PWR_SCR_CWUF1 | PWR_SCR_CWUF3 | PWR_SCR_CSBF|RTC_SCR_CWUTF);

    SET_BIT(PWR->CR3, PWR_CR3_APC);
    if (high){//should be a
    	SET_BIT(PWR->PUCRA, PWR_PUCRA_PU15);
    	SET_BIT(PWR->PUCRB, PWR_PUCRB_PU6);
    	CLEAR_BIT(PWR->PDCRB, PWR_PDCRB_PD7);
    	//CLEAR_BIT(PWR->PUCRB, PWR_PUCRB_PU0);
    }else{
    	//SET_BIT(PWR->PUCRB, PWR_PUCRB_PU0);
    	//CLEAR_BIT(PWR->PUCRB, PWR_PUCRB_PB7);
    	SET_BIT(PWR->PUCRB, PWR_PUCRB_PU6);
    	SET_BIT(PWR->PUCRA, PWR_PUCRA_PU15);
    	SET_BIT(PWR->PDCRB, PWR_PDCRB_PD7);

        __HAL_RCC_GPIOB_CLK_ENABLE();
       // GPIOB->MODER = 0xFFFFFFFFU;
    }

    SET_BIT(PWR->CR3, PWR_CR3_EWUP1 | PWR_CR3_EWUP3);
    SET_BIT(PWR->CR4, PWR_CR4_WP1   | PWR_CR4_WP3);


    SET_BIT(PWR->CR3, PWR_CR3_RRS);

    /* Enter Standby — execution stops here; wakeup causes a system reset */
    HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);
    HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN3);

    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
    HAL_SuspendTick();
    HAL_PWR_EnterSTANDBYMode();
}

void Backup_WriteString(const char *s) {
    uint32_t w;

        memcpy(&w, s + 0*4, 4);
        WRITE_REG(TAMP->BKP0R, w);
        memcpy(&w, s + 1*4, 4);
            WRITE_REG(TAMP->BKP1R, w);
            memcpy(&w, s + 2*4, 4);
                        WRITE_REG(TAMP->BKP2R, w);
                        memcpy(&w, s + 3*4, 4);
                                    WRITE_REG(TAMP->BKP3R, w);
                                    memcpy(&w, s + 4*4, 4);
                                                WRITE_REG(TAMP->BKP4R, w);
                                                memcpy(&w, s + 5*4, 4);
                                                            WRITE_REG(TAMP->BKP5R, w);//'\0' add at end?
                                                            memcpy(&w, s + 6*4, 4);
                                                                        WRITE_REG(TAMP->BKP6R, w);
}

void Backup_ReadString(char *s) {
    uint32_t w;
    for (int i = 0; i < 7; i++) {
        w = READ_REG(TAMP->BKP0R+i);
        memcpy(s + i*4, &w, 4);
    }

}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
