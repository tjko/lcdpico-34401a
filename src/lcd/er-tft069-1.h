/* er-tft069-1.h
 *
 * ER-TFT069-1 LCD panel (JD9365DA Controller)
 *
 */


#ifndef _ER_TFT069_1_H_
#define _ER_TFT069_1_H_

#define LCD_WIDTH   280
#define LCD_HEIGHT  1424

#define LCD_VBPD         20
#define LCD_VFPD         12
#define LCD_VSPW         3
#define LCD_HBPD         20
#define LCD_HFPD         20
#define LCD_HSPW         20

#define LCD_PCLK_FALLING_EDGE  0 // 1=Falling, 0=Raising
#define LCD_HSYNC_POLARITY     0 // 1=High, 0=Low
#define LCD_VSYNC_POLARITY     0 // 1=High, 0=Low
#define LCD_DE_POLARITY        1 // 1=Low, 0=High

#define LAYER1_START_ADDR  0
#define LAYER2_START_ADDR  797440
#define LAYER3_START_ADDR  1594880

#define PANEL_INIT er_tft069_1_init

extern const unsigned char er_tft069_1_init[];

#endif /* _ER_TFT069_1_H */
