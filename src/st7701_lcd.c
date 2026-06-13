/**
  ******************************************************************************
  * @file    lcd.c
  * @brief   This file provides code for the configuration
  *          of the lcd
  ******************************************************************************
  * ST7701S supports two kinds of RGB interface, DE mode (mode 1) and HV mode
  * (mode 2), and 16bit/18bit and 24 bit data format. When DE mode is selected
  * and the VSYNC, HSYNC, DOTCLK, DE, D23:0 pins can be used; when HV mode is
  * selected and the VSYNC, HSYNC, DOTCLK, D23:0 pins can be used. When using
  * RGB interface, only serial interface can be selected.
*/

/* Includes ------------------------------------------------------------------*/

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "lt7680.h"
#include "config.h"



static void DelayMicroseconds(uint16_t us);
#define HAL_Delay sleep_ms

#define SET_CS(cs)  gpio_put(LCD_CS_PIN, cs)
#define SET_CLK(clk) gpio_put(LCD_CLK_PIN, clk)
#define SET_DI(di) gpio_put(LCD_MOSI_PIN, di)


//************************************************************************************************************************************************************

// Bit bang SPI to LCD (9bit)
void LCD_SPI_Write(uint16_t data, uint8_t bits) {
	for (int i = bits - 1; i >= 0; i--) {  // Loop through each bit (MSB first)
		// Set SDA based on the current bit
		if (data & (1 << i)) {
			//HAL_GPIO_WritePin(LCD_SDI_Port, LCD_SDI_Pin, GPIO_PIN_SET);  // SDA = 1
			SET_DI(1);
		}
		else {
			//HAL_GPIO_WritePin(LCD_SDI_Port, LCD_SDI_Pin, GPIO_PIN_RESET); // SDA = 0
			SET_DI(0);
		}

		// Toggle the clock signal
		//HAL_GPIO_WritePin(LCD_SCK_Port, LCD_SCK_Pin, GPIO_PIN_SET); // CLK high
		SET_CLK(1);
		DelayMicroseconds(5);                                   // Hold high
		//HAL_GPIO_WritePin(LCD_SCK_Port, LCD_SCK_Pin, GPIO_PIN_RESET); // CLK low
		SET_CLK(0);
		DelayMicroseconds(5);                                   // Hold low
	}
}


void LCDWriteRegister(uint8_t reg) {
	//HAL_GPIO_WritePin(LCD_CS_Port, LCD_CS_Pin, GPIO_PIN_RESET); // Pull CS low
	SET_CS(0);
	DelayMicroseconds(10);

	// Use LCD_SPI_Write for 9-bit SPI communication
	LCD_SPI_Write((0 << 8) | reg, 9); // D/CX = 0, reg[7:0]

	//HAL_GPIO_WritePin(LCD_CS_Port, LCD_CS_Pin, GPIO_PIN_SET);   // Pull CS high
	SET_CS(1);
	DelayMicroseconds(10);
}



void LCDWriteData(uint8_t data) {
	//HAL_GPIO_WritePin(LCD_CS_Port, LCD_CS_Pin, GPIO_PIN_RESET); // Pull CS low
	SET_CS(0);
	DelayMicroseconds(10);

	// Use LCD_SPI_Write for 9-bit SPI communication
	LCD_SPI_Write((1 << 8) | data, 9); // D/CX = 1, data[7:0]

	//HAL_GPIO_WritePin(LCD_CS_Port, LCD_CS_Pin, GPIO_PIN_SET);   // Pull CS high
	SET_CS(1);
	DelayMicroseconds(10);
}



// delay used by the bit bang SPI	
static void DelayMicroseconds(uint16_t us) {
#if 0
	uint32_t start = SysTick->VAL; // Get current SysTick value
	uint32_t ticks = (HAL_RCC_GetHCLKFreq() / 1000000) * us; // Ticks for desired delay
	uint32_t reload = SysTick->LOAD + 1;

	while (((start - SysTick->VAL) & 0xFFFFFF) < ticks) {
		if (SysTick->VAL > reload) { // Handle SysTick counter wrap-around
			start -= reload;
		}
	}
#else
	sleep_us(us);
#endif
}


//**************************************************************************************************
// Commands to LCD


void LCD_Clear(uint16_t color) {

	LCDWriteRegister(0x2A); // Column Address Set
	LCDWriteData(0x00);     // Start column high byte
	LCDWriteData(0x00);     // Start column low byte
	LCDWriteData((LCD_WIDTH - 1) >> 8);  // End column high byte
	LCDWriteData((LCD_WIDTH - 1) & 0xFF); // End column low byte

	LCDWriteRegister(0x2B); // Row Address Set
	LCDWriteData(0x00);     // Start row high byte
	LCDWriteData(0x00);     // Start row low byte
	LCDWriteData((LCD_HEIGHT - 1) >> 8);  // End row high byte
	LCDWriteData((LCD_HEIGHT - 1) & 0xFF); // End row low byte

	// Write the color to the display RAM
	LCDWriteRegister(0x2C); // Memory Write command

	// Send color data for the entire screen
	for (uint32_t i = 0; i < (LCD_WIDTH * LCD_HEIGHT); i++) {
		LCDWriteData(color >> 8); // Send high byte of color
		LCDWriteData(color & 0xFF); // Send low byte of color
	}
}


void LCD_Hor_Ver_Timing() {
	// Horizontal Timing
	LCDWriteRegister(0x14); LCDWriteData(0x1D); // Horizontal Display Width				240
	LCDWriteRegister(0x16); LCDWriteData(0x09); // Horizontal Back Porch
	LCDWriteRegister(0x18); LCDWriteData(0x0C); // Horizontal Front Porch
	LCDWriteRegister(0x19); LCDWriteData(0x00); // HSYNC Pulse Width

	// Vertical Timing
	LCDWriteRegister(0x1A); LCDWriteData(0xFF); // Vertical Display Height (Low byte)	960 try BF is FF doesn't work properly
	LCDWriteRegister(0x1B); LCDWriteData(0x03); // Vertical Display Height (High bits)	960
	LCDWriteRegister(0x1C); LCDWriteData(0x05); // Vertical Back Porch (Low byte)
	LCDWriteRegister(0x1D); LCDWriteData(0x00); // Vertical Back Porch (High bits)
	LCDWriteRegister(0x1E); LCDWriteData(0x18); // Vertical Front Porch
	LCDWriteRegister(0x1F); LCDWriteData(0x01); // VSYNC Pulse Width
}


void BuyDisplay_Init() {

	LCDWriteRegister(0xff);
	LCDWriteData(0x77);
	LCDWriteData(0x01);
	LCDWriteData(0x00);
	LCDWriteData(0x00);
	LCDWriteData(0x13);
	LCDWriteRegister(0xef);
	LCDWriteData(0x08);
	LCDWriteRegister(0xff);
	LCDWriteData(0x77);
	LCDWriteData(0x01);
	LCDWriteData(0x00);
	LCDWriteData(0x00);
	LCDWriteData(0x10);
	LCDWriteRegister(0xc0);
	LCDWriteData(0x77);
	LCDWriteData(0x00);
	LCDWriteRegister(0xc1);
	LCDWriteData(0x11);
	LCDWriteData(0x0c);
	LCDWriteRegister(0xc2);
	LCDWriteData(0x07);
	LCDWriteData(0x02);

	LCDWriteRegister(0xC3);
	LCDWriteData(0x80);
	LCDWriteData(0x10);
	LCDWriteData(0x10);

	LCDWriteRegister(0xcc);
	LCDWriteData(0x30);
	LCDWriteRegister(0xB0);
	LCDWriteData(0x06);
	LCDWriteData(0xCF);
	LCDWriteData(0x14);
	LCDWriteData(0x0C);
	LCDWriteData(0x0F);
	LCDWriteData(0x03);
	LCDWriteData(0x00);
	LCDWriteData(0x0A);
	LCDWriteData(0x07);
	LCDWriteData(0x1B);
	LCDWriteData(0x03);
	LCDWriteData(0x12);
	LCDWriteData(0x10);
	LCDWriteData(0x25);
	LCDWriteData(0x36);
	LCDWriteData(0x1E);
	LCDWriteRegister(0xB1);
	LCDWriteData(0x0C);
	LCDWriteData(0xD4);
	LCDWriteData(0x18);
	LCDWriteData(0x0C);
	LCDWriteData(0x0E);
	LCDWriteData(0x06);
	LCDWriteData(0x03);
	LCDWriteData(0x06);
	LCDWriteData(0x08);
	LCDWriteData(0x23);
	LCDWriteData(0x06);
	LCDWriteData(0x12);
	LCDWriteData(0x10);
	LCDWriteData(0x30);
	LCDWriteData(0x2F);
	LCDWriteData(0x1F);
	LCDWriteRegister(0xff);
	LCDWriteData(0x77);
	LCDWriteData(0x01);
	LCDWriteData(0x00);
	LCDWriteData(0x00);
	LCDWriteData(0x11);
	LCDWriteRegister(0xb0);
	LCDWriteData(0x73);
	LCDWriteRegister(0xb1);
	LCDWriteData(0x7C);
	LCDWriteRegister(0xb2);
	LCDWriteData(0x83);
	LCDWriteRegister(0xb3);
	LCDWriteData(0x80);
	LCDWriteRegister(0xb5);
	LCDWriteData(0x49);
	LCDWriteRegister(0xb7);
	LCDWriteData(0x87);
	LCDWriteRegister(0xb8);
	LCDWriteData(0x33);
	LCDWriteRegister(0xb9);
	LCDWriteData(0x10);
	LCDWriteData(0x1f);
	LCDWriteRegister(0xbb);
	LCDWriteData(0x03);
	LCDWriteRegister(0xc1);
	LCDWriteData(0x08);
	LCDWriteRegister(0xc2);
	LCDWriteData(0x08);
	LCDWriteRegister(0xd0);
	LCDWriteData(0x88);
	LCDWriteRegister(0xe0);
	LCDWriteData(0x00);
	LCDWriteData(0x00);
	LCDWriteData(0x02);
	LCDWriteData(0x00);
	LCDWriteData(0x00);
	LCDWriteData(0x0c);
	LCDWriteRegister(0xe1);
	LCDWriteData(0x05);
	LCDWriteData(0x96);
	LCDWriteData(0x07);
	LCDWriteData(0x96);
	LCDWriteData(0x06);
	LCDWriteData(0x96);
	LCDWriteData(0x08);
	LCDWriteData(0x96);
	LCDWriteData(0x00);
	LCDWriteData(0x44);
	LCDWriteData(0x44);
	LCDWriteRegister(0xe2);
	LCDWriteData(0x00);
	LCDWriteData(0x00);
	LCDWriteData(0x03);
	LCDWriteData(0x03);
	LCDWriteData(0x00);
	LCDWriteData(0x00);
	LCDWriteData(0x02);
	LCDWriteData(0x00);
	LCDWriteData(0x00);
	LCDWriteData(0x00);
	LCDWriteData(0x02);
	LCDWriteData(0x00);
	LCDWriteRegister(0xe3);
	LCDWriteData(0x00);
	LCDWriteData(0x00);
	LCDWriteData(0x33);
	LCDWriteData(0x33);
	LCDWriteRegister(0xe4);
	LCDWriteData(0x44);
	LCDWriteData(0x44);
	LCDWriteRegister(0xe5);
	LCDWriteData(0x0d);
	LCDWriteData(0xd4);
	LCDWriteData(0x28);
	LCDWriteData(0x8c);
	LCDWriteData(0x0f);
	LCDWriteData(0xd6);
	LCDWriteData(0x28);
	LCDWriteData(0x8c);
	LCDWriteData(0x09);
	LCDWriteData(0xd0);
	LCDWriteData(0x28);
	LCDWriteData(0x8c);
	LCDWriteData(0x0b);
	LCDWriteData(0xd2);
	LCDWriteData(0x28);
	LCDWriteData(0x8c);
	LCDWriteRegister(0xe6);
	LCDWriteData(0x00);
	LCDWriteData(0x00);
	LCDWriteData(0x33);
	LCDWriteData(0x33);
	LCDWriteRegister(0xe7);
	LCDWriteData(0x44);
	LCDWriteData(0x44);
	LCDWriteRegister(0xe8);
	LCDWriteData(0x0e);
	LCDWriteData(0xd5);
	LCDWriteData(0x28);
	LCDWriteData(0x8c);
	LCDWriteData(0x10);
	LCDWriteData(0xd7);
	LCDWriteData(0x28);
	LCDWriteData(0x8c);
	LCDWriteData(0x0a);
	LCDWriteData(0xd1);
	LCDWriteData(0x28);
	LCDWriteData(0x8c);
	LCDWriteData(0x0c);
	LCDWriteData(0xd3);
	LCDWriteData(0x28);
	LCDWriteData(0x8c);
	LCDWriteRegister(0xeb);
	LCDWriteData(0x00);
	LCDWriteData(0x01);
	LCDWriteData(0xe4);
	LCDWriteData(0xe4);
	LCDWriteData(0x44);
	LCDWriteData(0x00);
	LCDWriteRegister(0xed);
	LCDWriteData(0xf3);
	LCDWriteData(0xc1);
	LCDWriteData(0xba);
	LCDWriteData(0x0f);
	LCDWriteData(0x66);
	LCDWriteData(0x77);
	LCDWriteData(0x44);
	LCDWriteData(0x55);
	LCDWriteData(0x55);
	LCDWriteData(0x44);
	LCDWriteData(0x77);
	LCDWriteData(0x66);
	LCDWriteData(0xf0);
	LCDWriteData(0xab);
	LCDWriteData(0x1c);
	LCDWriteData(0x3f);
	LCDWriteRegister(0xef);
	LCDWriteData(0x10);
	LCDWriteData(0x0d);
	LCDWriteData(0x04);
	LCDWriteData(0x08);
	LCDWriteData(0x3f);
	LCDWriteData(0x1f);
	LCDWriteRegister(0xff);
	LCDWriteData(0x77);
	LCDWriteData(0x01);
	LCDWriteData(0x00);
	LCDWriteData(0x00);
	LCDWriteData(0x13);
	LCDWriteRegister(0xe8);
	LCDWriteData(0x00);
	LCDWriteData(0x0e);

	LCDWriteRegister(0xe8);
	LCDWriteData(0x00);
	LCDWriteData(0x0c);
	HAL_Delay(10);
	LCDWriteRegister(0xe8);
	LCDWriteData(0x40);
	LCDWriteData(0x00);
	LCDWriteRegister(0xff);
	LCDWriteData(0x77);
	LCDWriteData(0x01);
	LCDWriteData(0x00);
	LCDWriteData(0x00);
	LCDWriteData(0x00);

	LCDWriteRegister(0x36);
	LCDWriteData(0x00);
	LCDWriteRegister(0x3A);
	LCDWriteData(0x66);
	LCDWriteRegister(0x11);
	HAL_Delay(120);
	LCDWriteRegister(0x29);
	HAL_Delay(20);

}
