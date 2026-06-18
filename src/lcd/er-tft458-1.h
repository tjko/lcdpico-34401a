/* er-tft458-1.h
 *
 * ER-TFT4.58-1 LCD panel (ST7701S Controller)
 *
 */


#ifndef _ER_TFT458_1_H_
#define _ER_TFT458_1_H_

#define LCD_WIDTH   320
#define LCD_HEIGHT  960

#define LCD_VBPD                 10
#define LCD_VFPD                 12
#define LCD_VSPW                 3
#define LCD_HBPD                 80
#define LCD_HFPD                 20
#define LCD_HSPW                 20

#define LCD_PCLK_FALLING_EDGE  0 // 1=Falling, 0=Raising
#define LCD_HSYNC_POLARITY     0 // 1=High, 0=Low
#define LCD_VSYNC_POLARITY     0 // 1=High, 0=Low
#define LCD_DE_POLARITY        1 // 1=Low, 0=High

#define LAYER1_START_ADDR  0
#define LAYER2_START_ADDR  614400
#define LAYER3_START_ADDR  1288000

#define PANEL_INIT er_tft458_1_init

extern const unsigned char er_tft458_1_init[];

#endif /* _ER_TFT458_1_H */
