#include "Display_EPD_W21.h"
#include "Display_EPD_W21_spi.h"
#include "main.h"
#include "stm32u0xx_hal.h"
//Delay Functions
void delay_xms(unsigned int xms)
{SystemClock_Config_LowPower();
	HAL_PWREx_EnableLowPowerRunMode();
	HAL_Delay(xms);
	HAL_PWREx_DisableLowPowerRunMode();
	SystemClock_Config_Normal();
}

////////////////////////////////////E-paper demo//////////////////////////////////////////////////////////
//Busy function
void Epaper_READBUSY(void)
{SystemClock_Config_LowPower();
HAL_PWREx_EnableLowPowerRunMode();
  while(1)
  {	 //=1 BUSY
     if(isEPD_W21_BUSY==0) break;
  }
  HAL_PWREx_DisableLowPowerRunMode();
  SystemClock_Config_Normal();
}
//Full screen refresh initialization
void EPD_HW_Init(unsigned int size)
{
	EPD_W21_RST_0;  // Module reset
	delay_xms(10);//At least 10ms delay
	EPD_W21_RST_1;
	delay_xms(10); //At least 10ms delay

	Epaper_READBUSY();
	EPD_W21_WriteCMD(0x12);  //SWRESET
	Epaper_READBUSY();

	EPD_W21_WriteCMD(0x01); //Driver output control
	EPD_W21_WriteDATA((EPD_HEIGHT-1)%256);
	EPD_W21_WriteDATA((EPD_HEIGHT-1)/256);
	EPD_W21_WriteDATA(0x00);

	EPD_W21_WriteCMD(0x11); //data entry mode
	EPD_W21_WriteDATA(0x01);

	EPD_W21_WriteCMD(0x44); //set Ram-X address start/end position
	EPD_W21_WriteDATA(0x00);
	EPD_W21_WriteDATA(EPD_WIDTH/8-1);

	EPD_W21_WriteCMD(0x45); //set Ram-Y address start/end position
	EPD_W21_WriteDATA((EPD_HEIGHT-1)%256);
	EPD_W21_WriteDATA((EPD_HEIGHT-1)/256);
	EPD_W21_WriteDATA(0x00);
	EPD_W21_WriteDATA(0x00);

	EPD_W21_WriteCMD(0x3C); //BorderWavefrom
	EPD_W21_WriteDATA(0x05);

  EPD_W21_WriteCMD(0x18); //Read built-in temperature sensor
	EPD_W21_WriteDATA(0x80);
	EPD_W21_WriteCMD(0x2C);   // Write VCOM register
	EPD_W21_WriteDATA(0x20);  // ~-2.0V typical; range ~0x00–0xFF
	                           // Lower value = more negative = higher contrast
	                           // Try 0x9A–0xB0 range
	EPD_W21_WriteCMD(0x04);   // Source driving voltage
	EPD_W21_WriteDATA(0x4B);  // VSH1 (positive, ~15V)
	EPD_W21_WriteDATA(0x4B);  // VSH2
	EPD_W21_WriteDATA(0x3A);  // VSL (negative, ~-15V)
	                           // Widen the gap between VSH1 & VSL for more contrast
	EPD_W21_WriteCMD(0x0C);   // Booster soft start control
	EPD_W21_WriteDATA(0xD7);  // Phase 1 (POR: 0x87)
	EPD_W21_WriteDATA(0xD6);  // Phase 2 (POR: 0x86)
	EPD_W21_WriteDATA(0xD5);  // Phase 3 (POR: 0x85)
	EPD_W21_WriteDATA(0x3F);

	EPD_W21_WriteCMD(0x1A);   // write temperature register
	//if (size<2){
	EPD_W21_WriteDATA(0x00);  //
	EPD_W21_WriteDATA(0x00);
	//}else{	EPD_W21_WriteDATA(0x0A);  //
	//EPD_W21_WriteDATA(0x10);}
	EPD_W21_WriteCMD(0x3A);   // Set dummy line period
	EPD_W21_WriteDATA(0x27);

	EPD_W21_WriteCMD(0x3B);   // Gate line width (TGate)
	EPD_W21_WriteDATA(0x10);
	//EPD_W21_WriteCMD(0x3C);

	EPD_W21_WriteCMD(0x3F);
	EPD_W21_WriteDATA(0x07);

	EPD_W21_WriteCMD(0x03);   // Gate driving voltage control
	EPD_W21_WriteDATA(0x1F);  // POR = 0x17 (~20V); range 0x10–0x1F
							/// Increase slightly (e.g. 0x19) for deeper blacks
	EPD_W21_WriteCMD(0x21);
	EPD_W21_WriteDATA(0x00);
	EPD_W21_WriteDATA(0x00);
 //* */
	EPD_W21_WriteCMD(0x4E);   // set RAM x address count to 0;
	EPD_W21_WriteDATA(0x00);
	EPD_W21_WriteCMD(0x4F);   // set RAM y address count to 0X199;
	EPD_W21_WriteDATA((EPD_HEIGHT-1)%256);
	EPD_W21_WriteDATA((EPD_HEIGHT-1)/256);
EPD_W21_WriteCMD(0x22); // Load temperature value
	EPD_W21_WriteDATA(0x91);
  EPD_W21_WriteCMD(0x20);
  Epaper_READBUSY();

}
//Fast refresh 1 initialization
void EPD_HW_Init_Fast(unsigned int size)
{
	EPD_W21_RST_0;  // Module reset
	delay_xms(10);//At least 10ms delay
	EPD_W21_RST_1;
	delay_xms(10); //At least 10ms delay

	EPD_W21_WriteCMD(0x12);  //SWRESET
	Epaper_READBUSY();

  EPD_W21_WriteCMD(0x18); //Read built-in temperature sensor
	EPD_W21_WriteDATA(0x80);

//	EPD_W21_WriteCMD(0x22); // Load temperature value
//	EPD_W21_WriteDATA(0xB1);
//  EPD_W21_WriteCMD(0x20);
 // Epaper_READBUSY();
	EPD_W21_WriteCMD(0x2C);   // Write VCOM register
	EPD_W21_WriteDATA(0x20);  // ~-2.0V typical; range ~0x00–0xFF
	                           // Lower value = more negative = higher contrast
	                           // Try 0x9A–0xB0 range
	EPD_W21_WriteCMD(0x04);   // Source driving voltage
	EPD_W21_WriteDATA(0x4B);  // VSH1 (positive, ~15V)
	EPD_W21_WriteDATA(0x4B);  // VSH2
	EPD_W21_WriteDATA(0x3A);  // VSL (negative, ~-15V)
	                           // Widen the gap between VSH1 & VSL for more contrast
	EPD_W21_WriteCMD(0x0C);   // Booster soft start control
	EPD_W21_WriteDATA(0xD7);  // Phase 1 (POR: 0x87)
	EPD_W21_WriteDATA(0xD6);  // Phase 2 (POR: 0x86)
	EPD_W21_WriteDATA(0xD5);  // Phase 3 (POR: 0x85)
	EPD_W21_WriteDATA(0x3F);

	EPD_W21_WriteCMD(0x1A);   // write temperature register
	if (size<2){
	EPD_W21_WriteDATA(0x00);  //
	EPD_W21_WriteDATA(0x00);
	}else{	EPD_W21_WriteDATA(0x0A);  //
	EPD_W21_WriteDATA(0x10);}
	EPD_W21_WriteCMD(0x3A);   // Set dummy line period
	EPD_W21_WriteDATA(0x27);

	EPD_W21_WriteCMD(0x3B);   // Gate line width (TGate)
	EPD_W21_WriteDATA(0x10);
	//EPD_W21_WriteCMD(0x3C);

	EPD_W21_WriteCMD(0x3F);
	EPD_W21_WriteDATA(0x07);

	EPD_W21_WriteCMD(0x03);   // Gate driving voltage control
	EPD_W21_WriteDATA(0x1F);  // POR = 0x17 (~20V); range 0x10–0x1F
							/// Increase slightly (e.g. 0x19) for deeper blacks
	EPD_W21_WriteCMD(0x21);
	EPD_W21_WriteDATA(0x00);
	EPD_W21_WriteDATA(0x00);
	//EPD_W21_WriteCMD(0x22); // Load temperature value
//	EPD_W21_WriteDATA(0x91);
  EPD_W21_WriteCMD(0x20);
	Epaper_READBUSY();
}


//////////////////////////////Display Update Function///////////////////////////////////////////////////////
//Full screen refresh update function
void EPD_Update(void)
{
  EPD_W21_WriteCMD(0x22); //Display Update Control
  EPD_W21_WriteDATA(0xF4);
  EPD_W21_WriteCMD(0x20); //Activate Display Update Sequence
  Epaper_READBUSY();

}
//Fast refresh 1 update function
void EPD_Update_Fast(void)
{  EPD_W21_WriteCMD(0x18); //Read built-in temperature sensor
EPD_W21_WriteDATA(0x80);

//	EPD_W21_WriteCMD(0x22); // Load temperature value
//	EPD_W21_WriteDATA(0xB1);
//  EPD_W21_WriteCMD(0x20);
// Epaper_READBUSY();


//	EPD_W21_WriteCMD(0x1A);   // write temperature register
//	EPD_W21_WriteDATA(0x00);//0a best so far 5.5 ish secs // force 25°C regardless of actual temp
//	EPD_W21_WriteDATA(0x00);
  EPD_W21_WriteCMD(0x22); //Display Update Control
  EPD_W21_WriteDATA(0xC7);
 // EPD_W21_WriteCMD(0x3F);
  //EPD_W21_WriteDATA(0x22);
//	EPD_W21_WriteCMD(0x04);//
//	EPD_W21_WriteDATA(0x4B);
//	EPD_W21_WriteDATA(0x37);//
//	EPD_W21_WriteDATA(0x3A);
//	EPD_W21_WriteCMD(0x21);
//	EPD_W21_WriteDATA(0x00);
//	EPD_W21_WriteDATA(0x00);
  EPD_W21_WriteCMD(0x20); //Activate Display Update Sequence
  Epaper_READBUSY();

}
//Partial refresh update function
void EPD_Part_Update(void)
{
	EPD_W21_WriteCMD(0x22); //Display Update Control
	EPD_W21_WriteDATA(0xC7);
	//EPD_W21_WriteCMD(0x03);
	//EPD_W21_WriteDATA(0x17);
	EPD_W21_WriteCMD(0x20); //Activate Display Update Sequence
	Epaper_READBUSY();
}
//////////////////////////////Display Data Transfer Function////////////////////////////////////////////
//Full screen refresh display function
void EPD_WhiteScreen_ALL(const unsigned char *datas)
{
   unsigned int i;
  EPD_W21_WriteCMD(0x24);   //write RAM for black(0)/white (1)
  for(i=0;i<EPD_ARRAY;i++)
   {
     EPD_W21_WriteDATA(datas[i]);
   }
   EPD_Update();
}
//Fast refresh display function
void EPD_WhiteScreen_ALL_Fast(const unsigned char *datas)
{
   unsigned int i;
  EPD_W21_WriteCMD(0x24);   //write RAM for black(0)/white (1)
   for(i=0;i<EPD_ARRAY;i++)
   {
     EPD_W21_WriteDATA(datas[i]);
   }

   EPD_Update_Fast();
}

//Clear screen display
void EPD_WhiteScreen_White(void)
{
 unsigned int i;
 EPD_W21_WriteCMD(0x24);   //write RAM for black(0)/white (1)
 for(i=0;i<EPD_ARRAY;i++)
 {
		EPD_W21_WriteDATA(0xff);
	}
	EPD_Update();
}
//Display all black
void EPD_WhiteScreen_Black(void)
{
 unsigned int i;
 EPD_W21_WriteCMD(0x24);   //write RAM for black(0)/white (1)
 for(i=0;i<EPD_ARRAY;i++)
 {
		EPD_W21_WriteDATA(0x00);
	}
	EPD_Update();
}
//Partial refresh of background display, this function is necessary, please do not delete it!!!
void EPD_SetRAMValue_BaseMap( const unsigned char * datas)
{
	unsigned int i;
  EPD_W21_WriteCMD(0x26);   //Write Black and White image to RAM
  for(i=0;i<EPD_ARRAY;i++)
   {
     EPD_W21_WriteDATA(datas[i]);
   }
  EPD_W21_WriteCMD(0x24);   //Write Black and White image to RAM
  for(i=0;i<EPD_ARRAY;i++)
   {
     EPD_W21_WriteDATA(datas[i]);
   }
   //EPD_Update();
  EPD_Update_Fast();
}
//Partial refresh display
void EPD_Dis_Part(unsigned int x_start,unsigned int y_start,const unsigned char * datas,unsigned int PART_COLUMN,unsigned int PART_LINE)
{
	unsigned int i;
	unsigned int x_end,y_end;

	x_start=x_start/8; //x address start
	x_end=x_start+PART_LINE/8-1; //x address end
	y_start=y_start; //Y address start
	y_end=y_start+PART_COLUMN-1; //Y address end

	EPD_W21_RST_0;  // Module reset
	delay_xms(10);//At least 10ms delay
	EPD_W21_RST_1;
	delay_xms(10); //At least 10ms delay
	EPD_W21_WriteCMD(0x3C); //BorderWavefrom,
	EPD_W21_WriteDATA(0x80);

	EPD_W21_WriteCMD(0x44);       // set RAM x address start/end
	EPD_W21_WriteDATA(x_start);  //x address start
	EPD_W21_WriteDATA(x_end);   //y address end
	EPD_W21_WriteCMD(0x45);    // set RAM y address start/end
	EPD_W21_WriteDATA(y_start%256);  //y address start2
	EPD_W21_WriteDATA(y_start/256); //y address start1
	EPD_W21_WriteDATA(y_end%256);  //y address end2
	EPD_W21_WriteDATA(y_end/256); //y address end1

	EPD_W21_WriteCMD(0x4E);        // set RAM x address count to 0;
	EPD_W21_WriteDATA(x_start);   //x start address
	EPD_W21_WriteCMD(0x4F);      // set RAM y address count to 0X127;
	EPD_W21_WriteDATA(y_start%256);//y address start2
	EPD_W21_WriteDATA(y_start/256);//y address start1


	 EPD_W21_WriteCMD(0x24);   //Write Black and White image to RAM
   for(i=0;i<PART_COLUMN*PART_LINE/8;i++)
   {
     EPD_W21_WriteDATA(datas[i]);
   }
	 EPD_Part_Update();

}
//Full screen partial refresh display
void EPD_Dis_PartAll(const unsigned char * datas)
{
	unsigned int i;
	unsigned int PART_COLUMN, PART_LINE;
	PART_COLUMN=EPD_HEIGHT,PART_LINE=EPD_WIDTH;

	EPD_W21_RST_0;  // Module reset
	delay_xms(10); //At least 10ms delay
	EPD_W21_RST_1;
	delay_xms(10); //At least 10ms delay
	EPD_W21_WriteCMD(0x3C); //BorderWavefrom,
	EPD_W21_WriteDATA(0x80);


	EPD_W21_WriteCMD(0x24);   //Write Black and White image to RAM
	 for(i=0;i<PART_COLUMN*PART_LINE/8;i++)
	 {
		 EPD_W21_WriteDATA(datas[i]);
	 }
	 EPD_Part_Update();

}
//Deep sleep function
void EPD_DeepSleep(unsigned int type)
{EPD_W21_WriteCMD(0x02);
Epaper_READBUSY();
  EPD_W21_WriteCMD(0x10); //Enter deep sleep
  if (type==2){
  EPD_W21_WriteDATA(0x03);}//deeeeeep sleepe
  else if (type==0){
	  EPD_W21_WriteDATA(0x00);
  }else{
	  EPD_W21_WriteDATA(0x01);
  }//twice as much deepness of sleepness do writeData(0x03)
 // delay_xms(100);
}

//Partial refresh write address and data
void EPD_Dis_Part_RAM(unsigned int x_start,unsigned int y_start,const unsigned char * datas,unsigned int PART_COLUMN,unsigned int PART_LINE)
{
	unsigned int i;
	unsigned int x_end,y_end;

	x_start=x_start/8; //x address start
	x_end=x_start+PART_LINE/8-1; //x address end

	y_start=y_start-1; //Y address start
	y_end=y_start+PART_COLUMN-1; //Y address end

	EPD_W21_RST_0;  // Module reset
	delay_xms(10);//At least 10ms delay
	EPD_W21_RST_1;
	delay_xms(10); //At least 10ms delay
	EPD_W21_WriteCMD(0x3C); //BorderWavefrom,
	EPD_W21_WriteDATA(0x80);

	EPD_W21_WriteCMD(0x44);       // set RAM x address start/end
	EPD_W21_WriteDATA(x_start);  //x address start
	EPD_W21_WriteDATA(x_end);   //y address end
	EPD_W21_WriteCMD(0x45);     // set RAM y address start/end
	EPD_W21_WriteDATA(y_start%256);  //y address start2
	EPD_W21_WriteDATA(y_start/256); //y address start1
	EPD_W21_WriteDATA(y_end%256);  //y address end2
	EPD_W21_WriteDATA(y_end/256); //y address end1

	EPD_W21_WriteCMD(0x4E);   // set RAM x address count to 0;
	EPD_W21_WriteDATA(x_start);   //x start address
	EPD_W21_WriteCMD(0x4F);   // set RAM y address count to 0X127;
	EPD_W21_WriteDATA(y_start%256); //y address start2
	EPD_W21_WriteDATA(y_start/256); //y address start1

	EPD_W21_WriteCMD(0x24);   //Write Black and White image to RAM
  for(i=0;i<PART_COLUMN*PART_LINE/8;i++)
   {
     EPD_W21_WriteDATA(datas[i]);
   }
}
//Clock display
void EPD_Dis_Part_Time(unsigned int x_startA,unsigned int y_startA,const unsigned char * datasA,
	                       unsigned int x_startB,unsigned int y_startB,const unsigned char * datasB,
												 unsigned int x_startC,unsigned int y_startC,const unsigned char * datasC,
												 unsigned int x_startD,unsigned int y_startD,const unsigned char * datasD,
											   unsigned int x_startE,unsigned int y_startE,const unsigned char * datasE,
												 unsigned int PART_COLUMN,unsigned int PART_LINE
	                      )
{
	EPD_Dis_Part_RAM(x_startA,y_startA,datasA,PART_COLUMN,PART_LINE);
	EPD_Dis_Part_RAM(x_startB,y_startB,datasB,PART_COLUMN,PART_LINE);
	EPD_Dis_Part_RAM(x_startC,y_startC,datasC,PART_COLUMN,PART_LINE);
	EPD_Dis_Part_RAM(x_startD,y_startD,datasD,PART_COLUMN,PART_LINE);
	EPD_Dis_Part_RAM(x_startE,y_startE,datasE,PART_COLUMN,PART_LINE);
	EPD_Part_Update();
}




////////////////////////////////Other newly added functions////////////////////////////////////////////
//Display rotation 180 degrees initialization
void EPD_HW_Init_180(void)
{
	EPD_W21_RST_0;  // Module reset
	delay_xms(10); //At least 10ms delay
	EPD_W21_RST_1;
	delay_xms(10); //At least 10ms delay

	Epaper_READBUSY();
	EPD_W21_WriteCMD(0x12);  //SWRESET
	Epaper_READBUSY();

	EPD_W21_WriteCMD(0x3C); //BorderWavefrom
	EPD_W21_WriteDATA(0x05);

	EPD_W21_WriteCMD(0x01); //Driver output control
	EPD_W21_WriteDATA((EPD_HEIGHT-1)%256);
	EPD_W21_WriteDATA((EPD_HEIGHT-1)/256);
	EPD_W21_WriteDATA(0x00);

	EPD_W21_WriteCMD(0x11); //data entry mode
	EPD_W21_WriteDATA(0x02);

	EPD_W21_WriteCMD(0x44); //set Ram-X address start/end position
	EPD_W21_WriteDATA(EPD_WIDTH/8-1);
	EPD_W21_WriteDATA(0x00);

	EPD_W21_WriteCMD(0x45); //set Ram-Y address start/end position
	EPD_W21_WriteDATA(0x00);
	EPD_W21_WriteDATA(0x00);
  EPD_W21_WriteDATA((EPD_HEIGHT-1)%256);
	EPD_W21_WriteDATA((EPD_HEIGHT-1)/256);


  EPD_W21_WriteCMD(0x18); //Read built-in temperature sensor
	EPD_W21_WriteDATA(0x80);

	EPD_W21_WriteCMD(0x4E);   // set RAM x address count to 0;
	EPD_W21_WriteDATA(EPD_WIDTH/8-1);
	EPD_W21_WriteCMD(0x4F);   // set RAM y address count to 0X199;
	EPD_W21_WriteDATA(0x00);
	EPD_W21_WriteDATA(0x00);
  Epaper_READBUSY();
}
// GUI initialization
void EPD_HW_Init_GUI(void)
{
	EPD_W21_RST_0;  // Module reset
	delay_xms(10); //At least 10ms delay
	EPD_W21_RST_1;
	delay_xms(10); //At least 10ms delay

	Epaper_READBUSY();
	EPD_W21_WriteCMD(0x12);  //SWRESET
	Epaper_READBUSY();

	EPD_W21_WriteCMD(0x01); //Driver output control
	EPD_W21_WriteDATA((EPD_HEIGHT+24-1)%256);
	EPD_W21_WriteDATA((EPD_HEIGHT+24-1)/256);
	EPD_W21_WriteDATA(0x01);//Show mirror

	EPD_W21_WriteCMD(0x11); //data entry mode
	EPD_W21_WriteDATA(0x01);

	EPD_W21_WriteCMD(0x44); //set Ram-X address start/end position
	EPD_W21_WriteDATA(0x00);
	EPD_W21_WriteDATA(EPD_WIDTH/8-1);

	EPD_W21_WriteCMD(0x45); //set Ram-Y address start/end position
	EPD_W21_WriteDATA((EPD_HEIGHT+24-1)%256);
	EPD_W21_WriteDATA((EPD_HEIGHT+24-1)/256);
	EPD_W21_WriteDATA(0x00);
	EPD_W21_WriteDATA(0x00);

	EPD_W21_WriteCMD(0x3C); //BorderWavefrom
	EPD_W21_WriteDATA(0x05);

  //EPD_W21_WriteCMD(0x18); //Read built-in temperature sensor
	//EPD_W21_WriteDATA(0x80);

	EPD_W21_WriteCMD(0x1A);   // write temperature register
	EPD_W21_WriteDATA(0x05);  // force 25°C regardless of actual temp
	EPD_W21_WriteDATA(0x00);

	EPD_W21_WriteCMD(0x4E);   // set RAM x address count to 0;
	EPD_W21_WriteDATA(0x00);
	EPD_W21_WriteCMD(0x4F);   // set RAM y address count to 0X199;
	EPD_W21_WriteDATA((EPD_HEIGHT+24-1)%256);
	EPD_W21_WriteDATA((EPD_HEIGHT+24-1)/256);
  Epaper_READBUSY();

}

//GUI display
void EPD_Display(unsigned char *Image)
{
	unsigned int Width, Height,i,j;
	Width = (EPD_WIDTH % 8 == 0)? (EPD_WIDTH / 8 ): (EPD_WIDTH / 8 + 1);
	Height = EPD_HEIGHT;

	EPD_W21_WriteCMD(0x24);
	for ( j = 0; j < Height; j++) {
			for ( i = 0; i < Width; i++) {
				 EPD_W21_WriteDATA(Image[i + j * Width]);
			}
	}
	EPD_Update();
}

static const uint8_t LUT_FULL_UPDATE[] = {
    // VS (voltage source) waveform data - 70 bytes (6 phases × 5 voltage pairs × 2)
    0x98, 0x50, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x98, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x64, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x98, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x64, 0x90,/*80*/ 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

	0x0A, 0x01, 0x00,//1
	0x03, 0x00, 0x03,
	0x00,
	0x1F, 0x0F,/*2 6*/ 0x00,//2
	0x00, 0x00, 0x00,
	0x00,
	0x00, 0x00, 0x00,//3
	0x00, 0x00, 0x00,
	0x00,
	0x00, 0x00, 0x00,//4
	0x00, 0x00, 0x00,
	0x00,
	0x00, 0x00, 0x00,//5
	0x00, 0x00, 0x00,
	0x00,
	0x00, 0x00, 0x00, //6
	0x00, 0x00, 0x00,
	0x00,
	0x00, 0x00, 0x00, //7
	0x00, 0x00, 0x00,
	0x00,
	0x00, 0x00, 0x00, //8
	0x00, 0x00, 0x00,
	0x00,
	0x00, 0x00, 0x00,//9
	0x00, 0x00, 0x00,
	0x00,
	0x00, 0x00, 0x00, //10
	0x00, 0x00, 0x00,
	0x00,
	0x00, 0x00, 0x00, //11
	0x00, 0x00, 0x00,
	0x00,
    0x00, 0x00, 0x00, //12
	0x00, 0x00, 0x00,
	0x00,

	0x04, 0xF0, 0x00,
    0x00, 0x00, 0x00,
};

void SSD1681_LoadCustomLUT(void)
{
    EPD_W21_WriteCMD(0x32);  // Write LUT Register
    for (uint8_t i = 0; i < sizeof(LUT_FULL_UPDATE); i++) {
        EPD_W21_WriteDATA(LUT_FULL_UPDATE[i]);
    }
}

//_Static_assert(sizeof(LUT_FULL_UPDATE) == 153);



// Add to Display_EPD_W21.c
void EPD_HW_Init_CustomLUT(void)
{
    // ----------------------------------------------------------------
    // 1. Hardware reset
    // ----------------------------------------------------------------
    EPD_W21_RST_0;
    delay_xms(10);
    EPD_W21_RST_1;
    delay_xms(10);
    Epaper_READBUSY();

    // ----------------------------------------------------------------
    // 2. SW Reset via command (recommended after HW reset)
    // ----------------------------------------------------------------
    EPD_W21_WriteCMD(0x12);  // SWRESET
    delay_xms(10);
    Epaper_READBUSY();

    // ----------------------------------------------------------------
    // 3. Driver output control
    //    MUX gate lines = 249 (0xF9) → 200 rows (0-199 = 0xC7 for 200px)
    //    Adjust Y[8:0] and TB/SM/GD bits to match your panel
    // ----------------------------------------------------------------
    EPD_W21_WriteCMD(0x01); //Driver output control
    	EPD_W21_WriteDATA((EPD_HEIGHT-1)%256);
    	EPD_W21_WriteDATA((EPD_HEIGHT-1)/256);
    	EPD_W21_WriteDATA(0x00);; // GD=0, SM=0, TB=0

    // ----------------------------------------------------------------
    // 4. Booster soft-start control
    // ----------------------------------------------------------------
    	EPD_W21_WriteCMD(0x0C);   // Booster soft start control
    	EPD_W21_WriteDATA(0xD7);  // Phase 1 (POR: 0x87)
    	EPD_W21_WriteDATA(0xD6);  // Phase 2 (POR: 0x86)
    	EPD_W21_WriteDATA(0xD5);  // Phase 3 (POR: 0x85)
    	EPD_W21_WriteDATA(0x3F);
    	EPD_W21_WriteCMD(0x21); // Display Update Control
    	EPD_W21_WriteDATA(0x00);    // Normal mode for black pixels
    	EPD_W21_WriteDATA(0x80);
    // ----------------------------------------------------------------
    // 5. Data entry mode
    //    0x03 = X increment, Y increment (left→right, top→bottom)
    // ----------------------------------------------------------------
    	EPD_W21_WriteCMD(0x04);   // Source driving voltage
    	EPD_W21_WriteDATA(0x4B);  // VSH1 (positive, ~15V)
    	EPD_W21_WriteDATA(0x4B);  // VSH2
    	EPD_W21_WriteDATA(0x3A);  // VSL (negative, ~-15V)
    // ----------------------------------------------------------------
    // 6. Set RAM X address range [0, X_END]
    //    200px wide → X bytes = ceil(200/8) - 1 = 24 (0x18)
    // ----------------------------------------------------------------
    	//EPD_W21_WriteCMD(0x44); //set Ram-X address start/end position
    	//EPD_W21_WriteDATA(0x00);
    	//EPD_W21_WriteDATA(EPD_WIDTH/8-1);
    // ----------------------------------------------------------------
    // 7. Set RAM Y address range [0, Y_END]
    //    200px tall → Y_END = 199 = 0x00C7
    // ----------------------------------------------------------------
    	//EPD_W21_WriteCMD(0x45); //set Ram-Y address start/end position
    	//EPD_W21_WriteDATA((EPD_HEIGHT-1)%256);
    	//EPD_W21_WriteDATA((EPD_HEIGHT-1)/256);
    	//EPD_W21_WriteDATA(0x00);
    	//EPD_W21_WriteDATA(0x00);

    // ----------------------------------------------------------------
    // 8. Border waveform control
    //    0x05 = VBD follows LUT1, GS transition = LUT3
    // ----------------------------------------------------------------
    EPD_W21_WriteCMD(0x3C);  // Border Waveform Control
    EPD_W21_WriteDATA(0x05);
	EPD_W21_WriteCMD(0x2C);   // Write VCOM register
	EPD_W21_WriteDATA(0x20);  // ~-2.0V typical; range ~0x00–0xFF
	                           // Lower value = more negative = higher contrast
	                           // Try 0x9A–0xB0 range
	EPD_W21_WriteCMD(0x04);   // Source driving voltage
	EPD_W21_WriteDATA(0x4B);  // VSH1 (positive, ~15V)
	EPD_W21_WriteDATA(0x4B);  // VSH2
	EPD_W21_WriteDATA(0x3A);  // VSL (negative, ~-15V)
    // ----------------------------------------------------------------
    // 9. Temperature sensor control — use internal sensor
    // ----------------------------------------------------------------
    EPD_W21_WriteCMD(0x18);  // Temperature Sensor Control
    EPD_W21_WriteDATA(0x80); // 0x80 = internal sensor
    EPD_W21_WriteCMD(0x03);   // Gate driving voltage control
    EPD_W21_WriteDATA(0x1F);
	EPD_W21_WriteCMD(0x3F);
	EPD_W21_WriteDATA(0x07);
	EPD_W21_WriteCMD(0x3A);   // Set dummy line period
	EPD_W21_WriteDATA(0x27);

	EPD_W21_WriteCMD(0x3B);   // Gate line width (TGate)
	EPD_W21_WriteDATA(0x10);
    // ----------------------------------------------------------------
    // 10. Load custom LUT from MCU (overrides OTP)
    // ----------------------------------------------------------------
    SSD1681_LoadCustomLUT();

    // ----------------------------------------------------------------
    // 11. Set RAM X & Y cursor to origin before writing pixel data
    // ----------------------------------------------------------------
    //EPD_W21_WriteCMD(0x4E);  // Set RAM X Address Counter
    //EPD_W21_WriteDATA(0x00);

    //EPD_W21_WriteCMD(0x4F);  // Set RAM Y Address Counter
    //EPD_W21_WriteDATA(0x00);
    //EPD_W21_WriteDATA(0x00);
    //EPD_W21_WriteCMD(0x22);
    //EPD_W21_WriteDATA(0xC0);
    //EPD_W21_WriteCMD(0x20);
    Epaper_READBUSY();
}
/***********************************************************
						end file
***********************************************************/

