/* er-tft3.71-1.c
 *
 * Initialization command array for ER-TFT3.71-1 LCD panel (ST7701S Controller)
 *
 * Array format per row:  <count>, <cmd>, [data bytes...]
 *   count     = total number of bytes in this entry (command + all data bytes)
 *   cmd       = SPI command byte
 *   data      = zero or more data bytes following the command
 *
 * Special entry:         LCD_DELAY, <ms>
 *   Inserts a delay of <ms> milliseconds between commands.
 *
 * Terminator:            0
 *   A single zero byte marks the end of the list.
 *
 * The ST7701S register pages are selected by command 0xFF (CND2BKxSEL).
 * The 5th data byte selects the active page:
 *   0x13 -> Command2 BK3  (NVM and internal analog/pump control registers)
 *   0x10 -> Command2 BK0  (display geometry, gamma, image enhancement)
 *   0x11 -> Command2 BK1  (power supply voltages, source timing)
 *   0x00 -> CMD1          (standard DCS commands: disable Command2)
 *
 */

#include "st7701.h"


const unsigned char er_tft371_1_init[] = {

    // -------------------------------------------------------------------------
    // STEP 1: 0xFF - CND2BKxSEL: Select Command2 BK3 page
    //   Data[0..3] = 0x77,0x01,0x00,0x00  (fixed signature bytes)
    //   Data[4]    = 0x13  -> CN2=1 (enable CMD2), BKxSEL=3 -> BK3 page
    // BK3 contains NVM and internal analog control registers.
    // -------------------------------------------------------------------------
    6, 0xff, 0x77, 0x01, 0x00, 0x00, 0x13,

    // -------------------------------------------------------------------------
    // STEP 2: 0xEF - BK3 internal register
    //   Data[0] = 0x08  (internal test/NVM control setting)
    // -------------------------------------------------------------------------
    2, 0xef, 0x08,

    // -------------------------------------------------------------------------
    // STEP 3: 0xFF - CND2BKxSEL: Select Command2 BK0 page
    //   Data[4] = 0x10  -> CN2=1, BKxSEL=0 -> BK0 page
    // BK0 contains display geometry, gamma, and image enhancement registers.
    // -------------------------------------------------------------------------
    6, 0xff, 0x77, 0x01, 0x00, 0x00, 0x10,

    // -------------------------------------------------------------------------
    // STEP 4: 0xC0 - LNESET: Display Line Setting (BK0)
    //   Data[0] = 0x77  -> LDE_EN=0, Line[6:0]=0x77
    //             NL = (0x77 + 1) x 8 = 120 x 8 = 960 active lines
    //   Data[1] = 0x00  -> Line_delta[1:0] = 0 (no extra delta lines)
    // -------------------------------------------------------------------------
    3, 0xc0, 0x77, 0x00,

    // -------------------------------------------------------------------------
    // STEP 5: 0xC1 - PORCTRL: Porch Control (BK0)
    //   Data[0] = 0x11  -> VBP[7:0] = 17  (Vertical Back Porch = 17 lines)
    //   Data[1] = 0x0C  -> VFP[7:0] = 12  (Vertical Front Porch = 12 lines)
    // -------------------------------------------------------------------------
    3, 0xc1, 0x11, 0x0c,

    // -------------------------------------------------------------------------
    // STEP 6: 0xC2 - INVSET: Inversion Selection & Frame Rate Control (BK0)
    //   Data[0] = 0x07  -> NLINV[2:0] = 7 -> Column inversion mode
    //   Data[1] = 0x02  -> RTNI[4:0]  = 2 -> min PCLK per line = 512+(2x16) = 544 clocks
    // -------------------------------------------------------------------------
    3, 0xc2, 0x07, 0x02,

    // -------------------------------------------------------------------------
    // STEP 7: 0xC3 - RGBCTRL: RGB Interface Control (BK0)
    //   Data[0] = 0x80  -> DE/HV=1  : HV (sync) mode selected (not DE mode)
    //                      VSP=0    : VSYNC active low
    //                      HSP=0    : HSYNC active low
    //                      DP=0     : data sampled on rising DOTCLK edge
    //                      EP=0     : data valid when ENABLE=1
    //   Data[1] = 0x10  -> HBP_HVRGB = 16  (Hsync back porch for HV mode)
    //   Data[2] = 0x10  -> VBP_HVRGB = 16  (Vsync back porch for HV mode)
    // -------------------------------------------------------------------------
    4, 0xc3, 0x80, 0x10, 0x10,

    // -------------------------------------------------------------------------
    // STEP 8: 0xCC - Gate/NVM control register (BK3 address space)
    //   Data[0] = 0x30  -> gate scan direction / NVM program active control
    // -------------------------------------------------------------------------
    2, 0xcc, 0x30,

    // -------------------------------------------------------------------------
    // STEP 9: 0xB0 - PVGAMCTRL: Positive Voltage Gamma Control (BK0)
    // 16 parameters defining the positive gamma curve voltage nodes:
    //   [0]  0x06 -> AJ0P + VC0P      [8]  0x07 -> VC147P
    //   [1]  0xCF -> AJ1P + VC4P      [9]  0x1B -> VC175P
    //   [2]  0x14 -> AJ2P + VC8P      [10] 0x03 -> AJ4P + VC203P
    //   [3]  0x0C -> VC16P            [11] 0x12 -> AJ5P + VC231P
    //   [4]  0x0F -> VC24P            [12] 0x10 -> AJ5P + VC239P
    //   [5]  0x03 -> VC52P            [13] 0x25 -> AJ6P + VC247P
    //   [6]  0x00 -> VC80P            [14] 0x36 -> AJ7P + VC251P
    //   [7]  0x0A -> VC108P           [15] 0x1E -> VC255P
    // -------------------------------------------------------------------------
    17, 0xb0, 0x06, 0xcf, 0x14, 0x0c, 0x0f, 0x03, 0x00, 0x0a, 0x07, 0x1b, 0x03, 0x12, 0x10, 0x25, 0x36, 0x1e,

    // -------------------------------------------------------------------------
    // STEP 10: 0xB1 - NVGAMCTRL: Negative Voltage Gamma Control (BK0)
    // 16 parameters defining the negative gamma curve voltage nodes:
    //   [0]  0x0C -> AJ0N + VC0N      [8]  0x08 -> VC147N
    //   [1]  0xD4 -> AJ1N + VC4N      [9]  0x23 -> VC175N
    //   [2]  0x18 -> AJ2N + VC8N      [10] 0x06 -> AJ4N + VC203N
    //   [3]  0x0C -> VC16N            [11] 0x12 -> AJ5N + VC231N
    //   [4]  0x0E -> VC24N            [12] 0x10 -> AJ5N + VC239N
    //   [5]  0x06 -> VC52N            [13] 0x30 -> AJ6N + VC247N
    //   [6]  0x03 -> VC80N            [14] 0x2F -> AJ7N + VC251N
    //   [7]  0x06 -> VC108N           [15] 0x1F -> VC255N
    // -------------------------------------------------------------------------
    17, 0xb1, 0x0c, 0xd4, 0x18, 0x0c, 0x0e, 0x06, 0x03, 0x06, 0x08, 0x23, 0x06, 0x12, 0x10, 0x30, 0x2f, 0x1f,

    // -------------------------------------------------------------------------
    // STEP 11: 0xFF - CND2BKxSEL: Select Command2 BK1 page
    //   Data[4] = 0x11  -> CN2=1, BKxSEL=1 -> BK1 page
    // BK1 contains power supply voltage and source timing registers.
    // -------------------------------------------------------------------------
    6, 0xff, 0x77, 0x01, 0x00, 0x00, 0x11,

    // -------------------------------------------------------------------------
    // STEP 12: 0xB0 - VRHS: Vop Amplitude Setting (BK1)
    //   Data[0] = 0x73  -> VRHA[7:0] = 0x73 (positive gamma reference voltage)
    // -------------------------------------------------------------------------
    2, 0xb0, 0x73,

    // -------------------------------------------------------------------------
    // STEP 13: 0xB1 - VCOMS: VCOM Amplitude Setting (BK1)
    //   Data[0] = 0x7C  -> VCOM[7:0] = 0x7C (VCOM voltage amplitude)
    // -------------------------------------------------------------------------
    2, 0xb1, 0x7c,

    // -------------------------------------------------------------------------
    // STEP 14: 0xB2 - VGHSS: VGH Voltage Setting (BK1)
    //   Data[0] = 0x83  -> VGHSS[3:0] = 0x3 -> VGH = 13.0V
    // -------------------------------------------------------------------------
    2, 0xb2, 0x83,

    // -------------------------------------------------------------------------
    // STEP 15: 0xB3 - TESTCMD: TEST Command Setting (BK1)
    //   Data[0] = 0x80  -> D7=1, others=0 (required value per datasheet)
    // -------------------------------------------------------------------------
    2, 0xb3, 0x80,

    // -------------------------------------------------------------------------
    // STEP 16: 0xB5 - VGLS: VGL Voltage Setting (BK1)
    //   Data[0] = 0x49  -> VGLS[3:0] = 0x9 (negative gate voltage level)
    // -------------------------------------------------------------------------
    2, 0xb5, 0x49,

    // -------------------------------------------------------------------------
    // STEP 17: 0xB7 - PWCTRL1: Power Control 1 (BK1)
    //   Data[0] = 0x87  -> AP[1:0], APIS[1:0], APOS[1:0] source amplifier power mode
    // -------------------------------------------------------------------------
    2, 0xb7, 0x87,

    // -------------------------------------------------------------------------
    // STEP 18: 0xB8 - PWCTRL2: Power Control 2 (BK1)
    //   Data[0] = 0x33  -> AVDD[1:0]=3 (AVDD boost x3), AVCL[1:0]=3 (AVCL boost x3)
    // -------------------------------------------------------------------------
    2, 0xb8, 0x33,

    // -------------------------------------------------------------------------
    // STEP 19: 0xB9 - PWCTRL3: Power Control 3 (BK1)
    //   Data[0] = 0x10  -> SVPO_PUM / SVNO_PUM settings
    //   Data[1] = 0x1F  -> additional power control bits
    // -------------------------------------------------------------------------
    3, 0xb9, 0x10, 0x1f,

    // -------------------------------------------------------------------------
    // STEP 20: 0xBB - PCLKS2: Power Pumping Clock Selection 2 (BK1)
    //   Data[0] = 0x03  -> SBSTCKS[1:0] = 3 (substrate boost pump clock select)
    // -------------------------------------------------------------------------
    2, 0xbb, 0x03,

    // -------------------------------------------------------------------------
    // STEP 21: 0xC1 - PDR1/SPD1: Source Pre-Drive Timing Set 1 (BK1)
    //   Data[0] = 0x08  -> T2D source pre-drive timing
    // -------------------------------------------------------------------------
    2, 0xc1, 0x08,

    // -------------------------------------------------------------------------
    // STEP 22: 0xC2 - PDR2: Source Pre-Drive Timing Set 2 (BK1)
    //   Data[0] = 0x08  -> T3D source pre-drive timing
    // -------------------------------------------------------------------------
    2, 0xc2, 0x08,

    // -------------------------------------------------------------------------
    // STEP 23: 0xD0 - MIPISET1: MIPI Setting 1 (BK1)
    //   Data[0] = 0x88  -> EOTP_EN + ERR_SEL MIPI interface configuration
    // -------------------------------------------------------------------------
    2, 0xd0, 0x88,

    // -------------------------------------------------------------------------
    // STEP 24: 0xE0 - SECTRL: Sunlight Readable Enhancement (BK0)
    // 6 bytes configuring the sunlight-readable enhancement block.
    // SRE is effectively disabled with these values (SRE_alpha in byte[2]).
    //   Data[0] = 0x00
    //   Data[1] = 0x00
    //   Data[2] = 0x02  -> SRE_alpha setting
    //   Data[3] = 0x00
    //   Data[4] = 0x00
    //   Data[5] = 0x0C
    // -------------------------------------------------------------------------
    7, 0xe0, 0x00, 0x00, 0x02, 0x00, 0x00, 0x0c,

    // -------------------------------------------------------------------------
    // STEP 25: 0xE1 - NRCTRL: Noise Reduce Control (BK0)
    // 11 bytes configuring noise reduction timing for odd/even lanes.
    //   Data[0]  = 0x05  -> odd-lane timing
    //   Data[1]  = 0x96  -> clock for lane 1
    //   Data[2]  = 0x07  -> odd-lane
    //   Data[3]  = 0x96  -> even-lane
    //   Data[4]  = 0x06
    //   Data[5]  = 0x96
    //   Data[6]  = 0x08
    //   Data[7]  = 0x96
    //   Data[8]  = 0x00
    //   Data[9]  = 0x44  -> NR_md and Y_gain settings
    //   Data[10] = 0x44
    // -------------------------------------------------------------------------
    12, 0xe1, 0x05, 0x96, 0x07, 0x96, 0x06, 0x96, 0x08, 0x96, 0x00, 0x44, 0x44,

    // -------------------------------------------------------------------------
    // STEP 26: 0xE2 - Sharpness Control (BK0)
    // 12 bytes configuring sharpness enhancement parameters.
    //   Data[0]  = 0x00    Data[6]  = 0x02
    //   Data[1]  = 0x00    Data[7]  = 0x00
    //   Data[2]  = 0x03    Data[8]  = 0x00
    //   Data[3]  = 0x03    Data[9]  = 0x00
    //   Data[4]  = 0x00    Data[10] = 0x02
    //   Data[5]  = 0x00    Data[11] = 0x00
    // -------------------------------------------------------------------------
    13, 0xe2, 0x00, 0x00, 0x03, 0x03, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00,

    // -------------------------------------------------------------------------
    // STEP 27: 0xE3 - CCCTRL: Color Calibration Control (BK0)
    // 4 bytes configuring color calibration (CCE and skin_ce_mid bits).
    //   Data[0] = 0x00
    //   Data[1] = 0x00
    //   Data[2] = 0x33  -> CCE and skin_ce_mid[1:0]
    //   Data[3] = 0x33
    // -------------------------------------------------------------------------
    5, 0xe3, 0x00, 0x00, 0x33, 0x33,

    // -------------------------------------------------------------------------
    // STEP 28: 0xE4 - SKCTRL: Skin Tone Preservation Control (BK0)
    //   Data[0] = 0x44
    //   Data[1] = 0x44
    // -------------------------------------------------------------------------
    3, 0xe4, 0x44, 0x44,

    // -------------------------------------------------------------------------
    // STEP 29: 0xE5 - GIP Timing Control Set 1 (BK0)
    // 16 bytes mapping display signals to panel gate driver (GIP) pins.
    // Each group of 4 bytes = [signal_id, clock_source, rise_time, fall_time]:
    //   Data[0..3]   = 0x0D,0xD4,0x28,0x8C -> GIP signal 1
    //   Data[4..7]   = 0x0F,0xD6,0x28,0x8C -> GIP signal 2
    //   Data[8..11]  = 0x09,0xD0,0x28,0x8C -> GIP signal 3
    //   Data[12..15] = 0x0B,0xD2,0x28,0x8C -> GIP signal 4
    // -------------------------------------------------------------------------
    17, 0xe5, 0x0d, 0xd4, 0x28, 0x8c, 0x0f, 0xd6, 0x28, 0x8c, 0x09, 0xd0, 0x28, 0x8c, 0x0b, 0xd2, 0x28, 0x8c,

    // -------------------------------------------------------------------------
    // STEP 30: 0xE6 - Color Calibration Control 2 (BK0)
    // Mirror of 0xE3 for the second set of calibration registers.
    //   Data[0] = 0x00, Data[1] = 0x00, Data[2] = 0x33, Data[3] = 0x33
    // -------------------------------------------------------------------------
    5, 0xe6, 0x00, 0x00, 0x33, 0x33,

    // -------------------------------------------------------------------------
    // STEP 31: 0xE7 - SKCTRL 2 (BK0)
    // Mirror of 0xE4 for the second skin tone preservation register.
    //   Data[0] = 0x44, Data[1] = 0x44
    // -------------------------------------------------------------------------
    3, 0xe7, 0x44, 0x44,

    // -------------------------------------------------------------------------
    // STEP 32: 0xE8 - GIP Timing Control Set 2 (BK0)
    // 16 bytes of additional GIP signal routing and timing.
    //   Data[0..3]   = 0x0E,0xD5,0x28,0x8C -> GIP signal 5
    //   Data[4..7]   = 0x10,0xD7,0x28,0x8C -> GIP signal 6
    //   Data[8..11]  = 0x0A,0xD1,0x28,0x8C -> GIP signal 7
    //   Data[12..15] = 0x0C,0xD3,0x28,0x8C -> GIP signal 8
    // -------------------------------------------------------------------------
    17, 0xe8, 0x0e, 0xd5, 0x28, 0x8c, 0x10, 0xd7, 0x28, 0x8c, 0x0a, 0xd1, 0x28, 0x8c, 0x0c, 0xd3, 0x28, 0x8c,

    // -------------------------------------------------------------------------
    // STEP 33: 0xEB - GIP EQ Timing / Equalization Control (BK0)
    // 6 bytes controlling equalization timing for GIP signals.
    //   Data[0] = 0x00, Data[1] = 0x01, Data[2] = 0xE4
    //   Data[3] = 0xE4, Data[4] = 0x44, Data[5] = 0x00
    // -------------------------------------------------------------------------
    7, 0xeb, 0x00, 0x01, 0xe4, 0xe4, 0x44, 0x00,

    // -------------------------------------------------------------------------
    // STEP 34: 0xED - GIP Pin Assignment (BK0)
    // 16 bytes defining the gate output pin assignment for the GIP driver.
    // Each nibble maps a gate output to a specific GIP signal.
    // Pattern is symmetric, covering left and right gate driver outputs.
    //   Data[0..7]   = 0xF3,0xC1,0xBA,0x0F,0x66,0x77,0x44,0x55
    //   Data[8..15]  = 0x55,0x44,0x77,0x66,0xF0,0xAB,0x1C,0x3F
    // -------------------------------------------------------------------------
    17, 0xed, 0xf3, 0xc1, 0xba, 0x0f, 0x66, 0x77, 0x44, 0x55, 0x55, 0x44, 0x77, 0x66, 0xf0, 0xab, 0x1c, 0x3f,

    // -------------------------------------------------------------------------
    // STEP 35: 0xEF - Internal Analog / Gate Timing Control (BK0)
    // 6 bytes of internal analog and gate timing settings.
    //   Data[0] = 0x10, Data[1] = 0x0D, Data[2] = 0x04
    //   Data[3] = 0x08, Data[4] = 0x3F, Data[5] = 0x1F
    // -------------------------------------------------------------------------
    7, 0xef, 0x10, 0x0d, 0x04, 0x08, 0x3f, 0x1f,

    // -------------------------------------------------------------------------
    // STEP 36: 0xFF - CND2BKxSEL: Select Command2 BK3 page (again)
    //   Data[4] = 0x13  -> CN2=1, BKxSEL=3 -> BK3 page
    // Required before the charge pump ramp-up sequence below.
    // -------------------------------------------------------------------------
    6, 0xff, 0x77, 0x01, 0x00, 0x00, 0x13,

    // -------------------------------------------------------------------------
    // STEPS 37-39: 0xE8 - Charge Pump Ramp-Up Sequence (BK3)
    // Three sequential writes to 0xE8 perform a stepped power-on sequence
    // for the internal charge pump, allowing voltages to ramp up safely.
    //
    // STEP 37 - Pump sequence part 1: initial state
    //   Data[0] = 0x00, Data[1] = 0x0E
    // -------------------------------------------------------------------------
    3, 0xe8, 0x00, 0x0e,

    // STEP 38 - Pump sequence part 2: intermediate state
    //   Data[0] = 0x00, Data[1] = 0x0C
    3, 0xe8, 0x00, 0x0c,
    LCD_DELAY, 10,              // 10ms settling time for pump voltage transition

    // STEP 39 - Pump sequence part 3: final active state
    //   Data[0] = 0x40, Data[1] = 0x00
    3, 0xe8, 0x40, 0x00,

    // -------------------------------------------------------------------------
    // STEP 40: 0xFF - CND2BKxSEL: Disable Command2, return to CMD1
    //   Data[4] = 0x00  -> CN2=0, disables CMD2 and returns to standard CMD1
    // All remaining commands use the standard DCS (CMD1) command set.
    // -------------------------------------------------------------------------
    6, 0xff, 0x77, 0x01, 0x00, 0x00, 0x00,

    // -------------------------------------------------------------------------
    // STEP 41: 0x36 - MADCTL: Memory Data Access Control (CMD1)
    //   Data[0] = 0x00  -> ML=0 (normal gate scan direction)
    //                      BGR=0 (RGB color order, not BGR)
    // -------------------------------------------------------------------------
    2, 0x36, 0x00,

    // -------------------------------------------------------------------------
    // STEP 42: 0x3A - COLMOD: Interface Pixel Format (CMD1)
    //   Data[0] = 0x66  -> VIPF[2:0] = 0b110 = 6 -> 18-bit/pixel (RGB666)
    // -------------------------------------------------------------------------
    2, 0x3a, 0x66,

    // -------------------------------------------------------------------------
    // STEP 43: 0x11 - SLPOUT: Sleep Out (CMD1)
    // No parameters. Exits sleep mode, enables DC/DC converter,
    // starts internal oscillator and panel scanning.
    // Mandatory 120ms wait for booster and oscillator to stabilize.
    // -------------------------------------------------------------------------
    1, 0x11,
    LCD_DELAY, 120,             // 120ms mandatory oscillator/booster stabilization

    // -------------------------------------------------------------------------
    // STEP 44: 0x29 - DISPON: Display On (CMD1)
    // No parameters. Exits Display Off mode and turns on the display output.
    // -------------------------------------------------------------------------
    1, 0x29,
    LCD_DELAY, 20,              // 20ms settling time after display enable

    0                           // END OF LIST
};

