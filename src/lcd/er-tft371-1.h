/* er-tft458-1.h
 *
 * ER-TFT3.71-1 LCD panel (ST7701S Controller)
 *
 */


#ifndef _ER_TFT371_1_H_
#define _ER_TFT371_1_H_

#define LCD_WIDTH   240
#define LCD_HEIGHT  960

#define LCD_VBPD		 11
#define LCD_VFPD	 	 5
#define LCD_VSPW		 5
#define LCD_HBPD		 128
#define LCD_HFPD		 5
#define LCD_HSPW	   	 5

#define LCD_PCLK_FALLING_EDGE  0 // 1=Falling, 0=Raising
#define LCD_HSYNC_POLARITY     0 // 1=High, 0=Low
#define LCD_VSYNC_POLARITY     0 // 1=High, 0=Low
#define LCD_DE_POLARITY        1 // 1=Low, 0=High

#define LAYER1_START_ADDR  0
#define LAYER2_START_ADDR  460800
#define LAYER3_START_ADDR  921600


#define PANEL_INIT er_tft371_1_init

extern const unsigned char er_tft371_1_init[];

#endif /* _ER_TFT371_1_H */
