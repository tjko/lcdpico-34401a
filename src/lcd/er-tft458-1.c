/* er-tft458-1.c
 *
 * Initialization command array for ER-TFT4.58-1 LCD panel (ST7701S Controller)
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


const unsigned char er_tft458_1_init[] = {

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
    //   Data[0] = 0x09  -> VBP[7:0] = 9   (Vertical Back Porch = 9 lines)
    //   Data[1] = 0x08  -> VFP[7:0] = 8   (Vertical Front Porch = 8 lines)
    // -------------------------------------------------------------------------
    3, 0xc1, 0x09, 0x08,

    // -------------------------------------------------------------------------
    // STEP 6: 0xC2 - INVSET: Inversion Selection & Frame Rate Control (BK0)
    //   Data[0] = 0x37  -> NLINV[2:0] = 7 -> Column inversion mode
    //                      upper bits set frame rate related divider
    //   Data[1] = 0x02  -> RTNI[4:0]  = 2 -> min PCLK per line = 512+(2x16) = 544 clocks
    // -------------------------------------------------------------------------
    3, 0xc2, 0x37, 0x02,

    // -------------------------------------------------------------------------
    // STEP 7: 0xC3 - RGBCTRL: RGB Interface Control (BK0)
    //   Data[0] = 0x80  -> DE/HV=1  : HV (sync) mode selected (not DE mode)
    //                      VSP=0    : VSYNC active low
    //                      HSP=0    : HSYNC active low
    //                      DP=0     : data sampled on rising DOTCLK edge
    //                      EP=0     : data valid when ENABLE=1
    //   Data[1] = 0x05  -> HBP_HVRGB = 5   (Hsync back porch for HV mode)
    //   Data[2] = 0x0D  -> VBP_HVRGB = 13  (Vsync back porch for HV mode)
    // -------------------------------------------------------------------------
    4, 0xc3, 0x80, 0x05, 0x0d,

    // -------------------------------------------------------------------------
    // STEP 8: 0xCC - Gate/NVM control register
    //   Data[0] = 0x10  -> gate scan direction / NVM program active control
    // -------------------------------------------------------------------------
    2, 0xcc, 0x10,

    // -------------------------------------------------------------------------
    // STEP 9: 0xB0 - PVGAMCTRL: Positive Voltage Gamma Control (BK0)
    // 16 parameters defining the positive gamma curve voltage nodes:
    //   [0]  0x40 -> AJ0P + VC0P      [8]  0x05 -> VC147P
    //   [1]  0x14 -> AJ1P + VC4P      [9]  0x1E -> VC175P
    //   [2]  0x59 -> AJ2P + VC8P      [10] 0x05 -> AJ4P + VC203P
    //   [3]  0x10 -> VC16P            [11] 0x14 -> AJ5P + VC231P
    //   [4]  0x12 -> VC24P            [12] 0x10 -> AJ5P + VC239P
    //   [5]  0x08 -> VC52P            [13] 0x68 -> AJ6P + VC247P
    //   [6]  0x03 -> VC80P            [14] 0x33 -> AJ7P + VC251P
    //   [7]  0x09 -> VC108P           [15] 0x15 -> VC255P
    // -------------------------------------------------------------------------
    17, 0xb0, 0x40, 0x14, 0x59, 0x10, 0x12, 0x08, 0x03, 0x09, 0x05, 0x1e, 0x05, 0x14, 0x10, 0x68, 0x33, 0x15,

    // -------------------------------------------------------------------------
    // STEP 10: 0xB1 - NVGAMCTRL: Negative Voltage Gamma Control (BK0)
    // 16 parameters defining the negative gamma curve voltage nodes:
    //   [0]  0x40 -> AJ0N + VC0N      [8]  0x09 -> VC147N
    //   [1]  0x08 -> AJ1N + VC4N      [9]  0x1A -> VC175N
    //   [2]  0x53 -> AJ2N + VC8N      [10] 0x04 -> AJ4N + VC203N
    //   [3]  0x09 -> VC16N            [11] 0x12 -> AJ5N + VC231N
    //   [4]  0x11 -> VC24N            [12] 0x12 -> AJ5N + VC239N
    //   [5]  0x09 -> VC52N            [13] 0x64 -> AJ6N + VC247N
    //   [6]  0x02 -> VC80N            [14] 0x29 -> AJ7N + VC251N
    //   [7]  0x07 -> VC108N           [15] 0x29 -> VC255N
    // -------------------------------------------------------------------------
    17, 0xb1, 0x40, 0x08, 0x53, 0x09, 0x11, 0x09, 0x02, 0x07, 0x09, 0x1a, 0x04, 0x12, 0x12, 0x64, 0x29, 0x29,

    // -------------------------------------------------------------------------
    // STEP 11: 0xFF - CND2BKxSEL: Select Command2 BK1 page
    //   Data[4] = 0x11  -> CN2=1, BKxSEL=1 -> BK1 page
    // BK1 contains power supply voltage and source timing registers.
    // -------------------------------------------------------------------------
    6, 0xff, 0x77, 0x01, 0x00, 0x00, 0x11,

    // -------------------------------------------------------------------------
    // STEP 12: 0xB0 - VRHS: Vop Amplitude Setting (BK1)
    //   Data[0] = 0x6D  -> VRHA[7:0] = 0x6D (positive gamma reference voltage)
    // -------------------------------------------------------------------------
    2, 0xb0, 0x6d,

    // -------------------------------------------------------------------------
    // STEP 13: 0xB1 - VCOMS: VCOM Amplitude Setting (BK1)
    //   Data[0] = 0x1D  -> VCOM[7:0] = 0x1D (VCOM voltage amplitude)
    // -------------------------------------------------------------------------
    2, 0xb1, 0x1d,

    // -------------------------------------------------------------------------
    // STEP 14: 0xB2 - VGHSS: VGH Voltage Setting (BK1)
    //   Data[0] = 0x87  -> VGHSS[3:0] = 0x7 -> VGH voltage level
    // -------------------------------------------------------------------------
    2, 0xb2, 0x87,

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
    //   Data[0] = 0x85  -> AP[1:0], APIS[1:0], APOS[1:0] source amplifier power mode
    // -------------------------------------------------------------------------
    2, 0xb7, 0x85,

    // -------------------------------------------------------------------------
    // STEP 18: 0xB8 - PWCTRL2: Power Control 2 (BK1)
    //   Data[0] = 0x20  -> AVDD[1:0]=2 (AVDD boost x2), AVCL[1:0]=0
    // -------------------------------------------------------------------------
    2, 0xb8, 0x20,

    // -------------------------------------------------------------------------
    // STEP 19: 0xC1 - PDR1/SPD1: Source Pre-Drive Timing Set 1 (BK1)
    //   Data[0] = 0x78  -> T2D source pre-drive timing
    // -------------------------------------------------------------------------
    2, 0xc1, 0x78,

    // -------------------------------------------------------------------------
    // STEP 20: 0xC2 - PDR2: Source Pre-Drive Timing Set 2 (BK1)
    //   Data[0] = 0x78  -> T3D source pre-drive timing
    // -------------------------------------------------------------------------
    2, 0xc2, 0x78,

    // -------------------------------------------------------------------------
    // STEP 21: 0xD0 - MIPISET1: MIPI Setting 1 (BK1)
    //   Data[0] = 0x88  -> EOTP_EN + ERR_SEL MIPI interface configuration
    // -------------------------------------------------------------------------
    2, 0xd0, 0x88,

    // -------------------------------------------------------------------------
    // STEP 22: 0xE0 - SECTRL: Sunlight Readable Enhancement (BK0)
    // 3 bytes configuring the sunlight-readable enhancement block.
    //   Data[0] = 0x00
    //   Data[1] = 0x00
    //   Data[2] = 0x02  -> SRE_alpha setting
    // -------------------------------------------------------------------------
    4, 0xe0, 0x00, 0x00, 0x02,

    // -------------------------------------------------------------------------
    // STEP 23: 0xE1 - NRCTRL: Noise Reduce Control (BK0)
    // 11 bytes configuring noise reduction timing for odd/even lanes.
    //   Data[0]  = 0x02  -> odd-lane signal index
    //   Data[1]  = 0x8C  -> clock source for lane 0
    //   Data[2]  = 0x00
    //   Data[3]  = 0x00
    //   Data[4]  = 0x03  -> even-lane signal index
    //   Data[5]  = 0x8C  -> clock source for lane 1
    //   Data[6]  = 0x00
    //   Data[7]  = 0x00
    //   Data[8]  = 0x00
    //   Data[9]  = 0x33  -> NR_md and Y_gain settings
    //   Data[10] = 0x33
    // -------------------------------------------------------------------------
    12, 0xe1, 0x02, 0x8c, 0x00, 0x00, 0x03, 0x8c, 0x00, 0x00, 0x00, 0x33, 0x33,

    // -------------------------------------------------------------------------
    // STEP 24: 0xE2 - Sharpness Control (BK0)
    // 13 bytes configuring sharpness enhancement parameters.
    //   Data[0]  = 0x33    Data[7]  = 0x00
    //   Data[1]  = 0x33    Data[8]  = 0xCA
    //   Data[2]  = 0x33    Data[9]  = 0x3C
    //   Data[3]  = 0x33    Data[10] = 0x00
    //   Data[4]  = 0xC9    Data[11] = 0x00
    //   Data[5]  = 0x3C    Data[12] = 0x00
    //   Data[6]  = 0x00
    // -------------------------------------------------------------------------
    14, 0xe2, 0x33, 0x33, 0x33, 0x33, 0xc9, 0x3c, 0x00, 0x00, 0xca, 0x3c, 0x00, 0x00, 0x00,

    // -------------------------------------------------------------------------
    // STEP 25: 0xE3 - CCCTRL: Color Calibration Control (BK0)
    // 4 bytes configuring color calibration (CCE and skin_ce_mid bits).
    //   Data[0] = 0x00
    //   Data[1] = 0x00
    //   Data[2] = 0x33  -> CCE and skin_ce_mid[1:0]
    //   Data[3] = 0x33
    // -------------------------------------------------------------------------
    5, 0xe3, 0x00, 0x00, 0x33, 0x33,

    // -------------------------------------------------------------------------
    // STEP 26: 0xE4 - SKCTRL: Skin Tone Preservation Control (BK0)
    //   Data[0] = 0x44
    //   Data[1] = 0x44
    // -------------------------------------------------------------------------
    3, 0xe4, 0x44, 0x44,

    // -------------------------------------------------------------------------
    // STEP 27: 0xE5 - GIP Timing Control Set 1 (BK0)
    // 16 bytes mapping display signals to panel gate driver (GIP) pins.
    // Each group of 4 bytes = [signal_id, clock_source, rise_time, fall_time]:
    //   Data[0..3]   = 0x05,0xCD,0x82,0x82 -> GIP signal 1
    //   Data[4..7]   = 0x01,0xC9,0x82,0x82 -> GIP signal 2
    //   Data[8..11]  = 0x07,0xCF,0x82,0x82 -> GIP signal 3
    //   Data[12..15] = 0x03,0xCB,0x82,0x82 -> GIP signal 4
    // -------------------------------------------------------------------------
    17, 0xe5, 0x05, 0xcd, 0x82, 0x82, 0x01, 0xc9, 0x82, 0x82, 0x07, 0xcf, 0x82, 0x82, 0x03, 0xcb, 0x82, 0x82,

    // -------------------------------------------------------------------------
    // STEP 28: 0xE6 - Color Calibration Control 2 (BK0)
    // Mirror of 0xE3 for the second set of calibration registers.
    //   Data[0] = 0x00, Data[1] = 0x00, Data[2] = 0x33, Data[3] = 0x33
    // -------------------------------------------------------------------------
    5, 0xe6, 0x00, 0x00, 0x33, 0x33,

    // -------------------------------------------------------------------------
    // STEP 29: 0xE7 - SKCTRL 2 (BK0)
    // Mirror of 0xE4 for the second skin tone preservation register.
    //   Data[0] = 0x44, Data[1] = 0x44
    // -------------------------------------------------------------------------
    3, 0xe7, 0x44, 0x44,

    // -------------------------------------------------------------------------
    // STEP 30: 0xE8 - GIP Timing Control Set 2 (BK0)
    // 16 bytes of additional GIP signal routing and timing.
    //   Data[0..3]   = 0x06,0xCE,0x82,0x82 -> GIP signal 5
    //   Data[4..7]   = 0x02,0xCA,0x82,0x82 -> GIP signal 6
    //   Data[8..11]  = 0x08,0xD0,0x82,0x82 -> GIP signal 7
    //   Data[12..15] = 0x04,0xCC,0x82,0x82 -> GIP signal 8
    // -------------------------------------------------------------------------
    17, 0xe8, 0x06, 0xce, 0x82, 0x82, 0x02, 0xca, 0x82, 0x82, 0x08, 0xd0, 0x82, 0x82, 0x04, 0xcc, 0x82, 0x82,

    // -------------------------------------------------------------------------
    // STEP 31: 0xEB - GIP EQ Timing / Equalization Control (BK0)
    // 7 bytes controlling equalization timing for GIP signals.
    //   Data[0] = 0x08, Data[1] = 0x01, Data[2] = 0xE4
    //   Data[3] = 0xE4, Data[4] = 0x88, Data[5] = 0x00, Data[6] = 0x40
    // -------------------------------------------------------------------------
    8, 0xeb, 0x08, 0x01, 0xe4, 0xe4, 0x88, 0x00, 0x40,

    // -------------------------------------------------------------------------
    // STEP 32: 0xEC - Additional GIP / EQ control (BK0)
    // Not present in the first panel init sequence. 3 bytes.
    //   Data[0] = 0x00
    //   Data[1] = 0x00
    //   Data[2] = 0x00
    // -------------------------------------------------------------------------
    4, 0xec, 0x00, 0x00, 0x00,

    // -------------------------------------------------------------------------
    // STEP 33: 0xED - GIP Pin Assignment (BK0)
    // 16 bytes defining the gate output pin assignment for the GIP driver.
    // Each nibble maps a gate output to a specific GIP signal.
    // Pattern is symmetric, covering left and right gate driver outputs.
    //   Data[0..7]   = 0xFF,0xF0,0x07,0x65,0x4F,0xFC,0xC2,0x2F
    //   Data[8..15]  = 0xF2,0x2C,0xCF,0xF4,0x56,0x70,0x0F,0xFF
    // -------------------------------------------------------------------------
    17, 0xed, 0xff, 0xf0, 0x07, 0x65, 0x4f, 0xfc, 0xc2, 0x2f, 0xf2, 0x2c, 0xcf, 0xf4, 0x56, 0x70, 0x0f, 0xff,

    // -------------------------------------------------------------------------
    // STEP 34: 0xEF - Internal Analog / Gate Timing Control (BK0)
    // 6 bytes of internal analog and gate timing settings.
    //   Data[0] = 0x10, Data[1] = 0x0D, Data[2] = 0x04
    //   Data[3] = 0x08, Data[4] = 0x3F, Data[5] = 0x1F
    // -------------------------------------------------------------------------
    7, 0xef, 0x10, 0x0d, 0x04, 0x08, 0x3f, 0x1f,

    // -------------------------------------------------------------------------
    // STEP 35: 0xFF - CND2BKxSEL: Disable Command2, return to CMD1
    //   Data[4] = 0x00  -> CN2=0, disables CMD2 and returns to standard CMD1
    // All remaining commands use the standard DCS (CMD1) command set.
    // -------------------------------------------------------------------------
    6, 0xff, 0x77, 0x01, 0x00, 0x00, 0x00,

    // -------------------------------------------------------------------------
    // STEP 36: 0x11 - SLPOUT: Sleep Out (CMD1)
    // No parameters. Exits sleep mode, enables DC/DC converter,
    // starts internal oscillator and panel scanning.
    // Mandatory 120ms wait for booster and oscillator to stabilize.
    // -------------------------------------------------------------------------
    1, 0x11,
    LCD_DELAY, 120,             // 120ms mandatory oscillator/booster stabilization

    // -------------------------------------------------------------------------
    // STEP 37: 0x35 - TEON: Tearing Effect Line On (CMD1)
    //   Data[0] = 0x00  -> TE output mode 0 (V-blanking only, no H-blanking)
    // Enables the TE signal on the TE pin for display sync.
    // -------------------------------------------------------------------------
    2, 0x35, 0x00,

    // -------------------------------------------------------------------------
    // STEP 38: 0x3A - COLMOD: Interface Pixel Format (CMD1)
    //   Data[0] = 0x66  -> VIPF[2:0] = 0b110 = 6 -> 18-bit/pixel (RGB666)
    // -------------------------------------------------------------------------
    2, 0x3a, 0x66,

    // -------------------------------------------------------------------------
    // STEP 39: 0x29 - DISPON: Display On (CMD1)
    // No parameters. Exits Display Off mode and turns on the display output.
    // -------------------------------------------------------------------------
    1, 0x29,
    LCD_DELAY, 20,

    0                           // END OF LIST
};

