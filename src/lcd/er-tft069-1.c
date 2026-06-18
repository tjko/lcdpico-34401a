/* er-tft069-1.c
 *
 * Initialization command array for ER-TFT061-1 LCD panel using JS9365DA controller.
 * LCD panel controller is accessed indirectly through the SSD2828 RGB-to-MIPI bridge
 * chip using its Generic Packet Drop mechanism.
 *
 * Array format per row:  <count>, <cmd>, [data bytes...]
 *   count     = total number of bytes in this entry (command + all data bytes)
 *   cmd       = SPI command byte
 *   data      = zero or more data bytes following the command
 *   These are used to configure SSD2828 Bridge registers via direct SPI writes.
 *
 * Special entries:
 *     LCD_DELAY, <ms>
 *     Inserts a delay of <ms> milliseconds between commands.
 *
 *     GP_PACKET, <count>, [data bytes...]
 *     Data bytes to be sent to LCD panel controller through SSD2828 chip
 *     using Generic Packet Drop register.
 *
 * Terminator:            0
 *   A single zero byte marks the end of the list.
 *
 */

#include "er-tft069-1.h"

#define GP_PACKET    0xfe    // generic packet drop marker (distinct from any real count)
#define LCD_DELAY    0xff    // delay marker (distinct from any real count)

#define hsize 600
#define vsize 1424


const unsigned char er_tft069_1_init[] = {

    // =====================================================================
    // PHASE 1 - SSD2828 pre-config (low-speed setup before sending panel
    // init commands). PLL is configured for a slower clock first because
    // the panel init sequence below is sent at reduced MIPI clock speed.
    // =====================================================================

    // -------------------------------------------------------------------
    // 0xB7 - CFGR: Configuration Register
    //   Data[0] = 0x50 -> LPE=1 (Long Packet), EOT=1 (send EOT), DCS=0
    //             (Generic packet mode selected for the init commands
    //             that follow), HS=0 (LP mode for these writes)
    //   Data[1] = 0x00
    // -------------------------------------------------------------------
    3, 0xb7, 0x50, 0x00,

    // -------------------------------------------------------------------
    // 0xB8 - VCR: Virtual Channel ID Control Register
    //   Data[0] = 0x00 -> Virtual Channel = 0
    //   Data[1] = 0x00
    // -------------------------------------------------------------------
    3, 0xb8, 0x00, 0x00,

    // -------------------------------------------------------------------
    // 0xB9 - PLL Control Register: PLL disable
    //   Data[0] = 0x00 -> PLL enabled (bit=0 means PLL not disabled)
    //   Data[1] = 0x00
    // -------------------------------------------------------------------
    3, 0xb9, 0x00, 0x00,

    // -------------------------------------------------------------------
    // 0xBA - PLLCR: PLL Configuration Register
    //   PLL frequency = (TX_CLK / MS) * NS
    //   Data[0] = 0x14 -> NS[7:0] = 0x14 (NS = 20)
    //   Data[1] = 0x42 -> PLL range bits[15:14]=01 (126-250MHz range),
    //             MS[12:8] = 0x01 (MS = 1)
    // -------------------------------------------------------------------
    3, 0xba, 0x14, 0x42,

    // -------------------------------------------------------------------
    // 0xBB - LPCDR: LP Clock Divider Register
    //   LP clock = HS clock / LPD / 8
    //   Data[0] = 0x03 -> LPD[5:0] = 3 (divide ratio)
    //   Data[1] = 0x00
    // -------------------------------------------------------------------
    3, 0xbb, 0x03, 0x00,

    // -------------------------------------------------------------------
    // 0xB9 - PLL Control Register: enable PLL
    //   Data[0] = 0x01 -> PLL enable bit set (starts the PLL with the
    //             configuration just programmed in 0xBA/0xBB)
    //   Data[1] = 0x00
    // -------------------------------------------------------------------
    3, 0xb9, 0x01, 0x00,

    // -------------------------------------------------------------------
    // 0xDE - Lane Configuration Register
    //   Data[0] = 0x00 -> 00 = 1 MIPI data lane (used for the slow
    //             panel-init phase; lane count is increased later)
    //   Data[1] = 0x00
    // -------------------------------------------------------------------
    3, 0xde, 0x00, 0x00,

    // -------------------------------------------------------------------
    // 0xC9 - Analog/Timing Control Register
    //   Data[0] = 0x02 -> HS-Data-Zero parameter (p1)
    //   Data[1] = 0x23 -> HS-Data-Prepare parameter (p2)
    // -------------------------------------------------------------------
    3, 0xc9, 0x02, 0x23,


    // =====================================================================
    // PHASE 2 - JD9365DA panel initialization, sent through the SSD2828
    // Generic Packet Drop mechanism (see header comment for the exact
    // byte sequence each of these rows represents in hardware).
    //
    // The JD9365DA register map is organized into pages selected by
    // writing to register 0xE0 with a page index:
    //   0xE0 = 0x00 -> Page 0  (User/normal command page)
    //   0xE0 = 0x01 -> Page 1  (Power and gamma settings)
    //   0xE0 = 0x02 -> Page 2  (GIP timing / gate driver settings)
    // =====================================================================

    // --- Page 0 (0xE0=0x00): basic panel/timing control ---

    GP_PACKET, 2, 0xe0, 0x00,

    GP_PACKET, 2, 0xe1, 0x93,   // Password/unlock byte 1 for manufacturer commands
    GP_PACKET, 2, 0xe2, 0x65,   // Password/unlock byte 2
    GP_PACKET, 2, 0xe3, 0xf8,   // Password/unlock byte 3
    GP_PACKET, 2, 0x80, 0x03,   // Panel control: sets number of lanes / output config

    // --- Page 1 (0xE0=0x01): power and positive/negative gamma ---
    GP_PACKET, 2, 0xe0, 0x01,

    GP_PACKET, 2, 0x00, 0x00,  // VCOM control / VREG1OUT setting
    GP_PACKET, 2, 0x01, 0x5b,  // VCOM amplitude (note: 0xA0 alt. value in comment)
    GP_PACKET, 2, 0x03, 0x10,  // VCOM_R setting
    GP_PACKET, 2, 0x04, 0x37,  // VCOM_R amplitude (note: 0xA0 alt. value in comment)

    GP_PACKET, 2, 0x0c, 0x74,  // Power control setting

    GP_PACKET, 2, 0x17, 0x00,  // VGMP (gamma positive ref voltage) high byte
    GP_PACKET, 2, 0x18, 0xdf,  // VGMP low byte -> VGMP = 4.9V
    GP_PACKET, 2, 0x19, 0x01,  // VGMP fine adjust
    GP_PACKET, 2, 0x1a, 0x00,  // VGMN (gamma negative ref voltage) high byte
    GP_PACKET, 2, 0x1b, 0xdf,  // VGMN low byte -> VGMN = -4.9V
    GP_PACKET, 2, 0x1c, 0x01,  // VGMN fine adjust

    GP_PACKET, 2, 0x1f, 0x2f,  // VGH_R = 15V
    GP_PACKET, 2, 0x20, 0x2f,  // VGL_R = -12V
    GP_PACKET, 2, 0x21, 0x2f,  // VGL_R2 = -12V
    GP_PACKET, 2, 0x22, 0x0e,  // PA[6]=0, PA[5]=0, PA[4]=0, PA[0]=0 (power seq control)

    GP_PACKET, 2, 0x24, 0xfe,  // Power control setting

    GP_PACKET, 2, 0x37, 0x09,  // SS=1 (source output scan direction), BGR=1 (color order)

    GP_PACKET, 2, 0x38, 0x04,  // JDT[2:0]=101 -> column inversion type select
    GP_PACKET, 2, 0x39, 0x00,  // RGB_N_EQ1 timing
    GP_PACKET, 2, 0x3a, 0x01,  // RGB_N_EQ2 timing
    GP_PACKET, 2, 0x3c, 0x90,  // EQ3 setting for TE_H (tearing effect high pulse)
    GP_PACKET, 2, 0x3d, 0xff,  // CHGEN_ON (charge sharing enable on time)
    GP_PACKET, 2, 0x3e, 0xff,  // CHGEN_OFF (charge sharing disable time)
    GP_PACKET, 2, 0x3f, 0xff,  // CHGEN_OFF2

    GP_PACKET, 2, 0x40, 0x02,  // RSO[2:0] -> 720 RGB resolution select
    GP_PACKET, 2, 0x41, 0xb2,  // LN[6:0] -> 712, doubled internally -> 1424 lines
    GP_PACKET, 2, 0x43, 0x06,  // VFP (Vertical Front Porch, panel-internal)
    GP_PACKET, 2, 0x44, 0x0a,  // VBP (Vertical Back Porch, panel-internal)
    GP_PACKET, 2, 0x45, 0x3c,  // HBP (Horizontal Back Porch, panel-internal)
    GP_PACKET, 2, 0x4b, 0x04,  // Source EQ off setting

    GP_PACKET, 2, 0x55, 0x02,  // Power mode select: 0x02=2-power mode, 0x0C=3-power mode
    GP_PACKET, 2, 0x56, 0x01,  // Power control
    GP_PACKET, 2, 0x57, 0x89,  // Power control
    GP_PACKET, 2, 0x58, 0x0a,  // Power control
    GP_PACKET, 2, 0x59, 0x0a,  // VCL = -2.9V
    GP_PACKET, 2, 0x5a, 0x28,  // VGH = 15.2V
    GP_PACKET, 2, 0x5b, 0x1a,  // VGL = -12.2V

    // --- Positive gamma curve (18 points) ---
    GP_PACKET, 2, 0x5d, 0x7c,
    GP_PACKET, 2, 0x5e, 0x57,
    GP_PACKET, 2, 0x5f, 0x44,
    GP_PACKET, 2, 0x60, 0x36,
    GP_PACKET, 2, 0x61, 0x31,
    GP_PACKET, 2, 0x62, 0x23,
    GP_PACKET, 2, 0x63, 0x26,
    GP_PACKET, 2, 0x64, 0x0f,
    GP_PACKET, 2, 0x65, 0x28,
    GP_PACKET, 2, 0x66, 0x26,
    GP_PACKET, 2, 0x67, 0x27,
    GP_PACKET, 2, 0x68, 0x45,
    GP_PACKET, 2, 0x69, 0x35,
    GP_PACKET, 2, 0x6a, 0x3d,
    GP_PACKET, 2, 0x6b, 0x2f,
    GP_PACKET, 2, 0x6c, 0x2b,
    GP_PACKET, 2, 0x6d, 0x1e,
    GP_PACKET, 2, 0x6e, 0x0d,
    GP_PACKET, 2, 0x6f, 0x00,

    // --- Negative gamma curve (18 points) ---
    GP_PACKET, 2, 0x70, 0x7c,
    GP_PACKET, 2, 0x71, 0x57,
    GP_PACKET, 2, 0x72, 0x44,
    GP_PACKET, 2, 0x73, 0x36,
    GP_PACKET, 2, 0x74, 0x31,
    GP_PACKET, 2, 0x75, 0x23,
    GP_PACKET, 2, 0x76, 0x26,
    GP_PACKET, 2, 0x77, 0x0f,
    GP_PACKET, 2, 0x78, 0x28,
    GP_PACKET, 2, 0x79, 0x26,
    GP_PACKET, 2, 0x7a, 0x27,
    GP_PACKET, 2, 0x7b, 0x45,
    GP_PACKET, 2, 0x7c, 0x35,
    GP_PACKET, 2, 0x7d, 0x3d,
    GP_PACKET, 2, 0x7e, 0x2f,
    GP_PACKET, 2, 0x7f, 0x2b,
    GP_PACKET, 2, 0x80, 0x1e,
    GP_PACKET, 2, 0x81, 0x0d,
    GP_PACKET, 2, 0x82, 0x00,

    // --- Page 2 (0xE0=0x02): GIP (Gate-In-Panel) timing control ---
    GP_PACKET, 2, 0xe0, 0x02,

    // GIP control group A (registers 0x00-0x15)
    GP_PACKET, 2, 0x00, 0x75,  // GCL (gate clock control)
    GP_PACKET, 2, 0x01, 0x50,  // STV0 (gate start vertical pulse, signal 0)
    GP_PACKET, 2, 0x02, 0x55,  // GCH (gate clock high)
    GP_PACKET, 2, 0x03, 0x43,  // STV3
    GP_PACKET, 2, 0x04, 0x5e,  // VDS
    GP_PACKET, 2, 0x05, 0x4f,  // VSD
    GP_PACKET, 2, 0x06, 0x41,  // STV1
    GP_PACKET, 2, 0x07, 0x5f,  // VGL
    GP_PACKET, 2, 0x08, 0x45,  // CLK5
    GP_PACKET, 2, 0x09, 0x47,  // CLK7
    GP_PACKET, 2, 0x0a, 0x49,  // CLK1
    GP_PACKET, 2, 0x0b, 0x4b,  // CLK3
    GP_PACKET, 2, 0x0c, 0x5f,
    GP_PACKET, 2, 0x0d, 0x5f,
    GP_PACKET, 2, 0x0e, 0x5f,
    GP_PACKET, 2, 0x0f, 0x5f,
    GP_PACKET, 2, 0x10, 0x5f,
    GP_PACKET, 2, 0x11, 0x5f,
    GP_PACKET, 2, 0x12, 0x5f,
    GP_PACKET, 2, 0x13, 0x5f,
    GP_PACKET, 2, 0x14, 0x5f,
    GP_PACKET, 2, 0x15, 0x5f,

    // GIP control group B (registers 0x16-0x2B) - mirror of group A
    GP_PACKET, 2, 0x16, 0x75,  // GCL
    GP_PACKET, 2, 0x17, 0x50,  // STV0
    GP_PACKET, 2, 0x18, 0x55,  // GCH
    GP_PACKET, 2, 0x19, 0x42,  // STV4
    GP_PACKET, 2, 0x1a, 0x5e,  // VDS
    GP_PACKET, 2, 0x1b, 0x5f,  // VSD
    GP_PACKET, 2, 0x1c, 0x40,  // STV2
    GP_PACKET, 2, 0x1d, 0x5f,  // VGL
    GP_PACKET, 2, 0x1e, 0x44,  // CLK6
    GP_PACKET, 2, 0x1f, 0x46,  // CLK8
    GP_PACKET, 2, 0x20, 0x48,  // CLK2
    GP_PACKET, 2, 0x21, 0x4a,  // CLK4
    GP_PACKET, 2, 0x22, 0x5f,
    GP_PACKET, 2, 0x23, 0x5f,
    GP_PACKET, 2, 0x24, 0x5f,
    GP_PACKET, 2, 0x25, 0x5f,
    GP_PACKET, 2, 0x26, 0x5f,
    GP_PACKET, 2, 0x27, 0x5f,
    GP_PACKET, 2, 0x28, 0x5f,
    GP_PACKET, 2, 0x29, 0x5f,
    GP_PACKET, 2, 0x2a, 0x5f,
    GP_PACKET, 2, 0x2b, 0x5f,

    // GIP control group C (registers 0x2C-0x41) - additional gate timing
    GP_PACKET, 2, 0x2c, 0x5e,
    GP_PACKET, 2, 0x2d, 0x5e,
    GP_PACKET, 2, 0x2e, 0x75,
    GP_PACKET, 2, 0x2f, 0x50,
    GP_PACKET, 2, 0x30, 0x47,
    GP_PACKET, 2, 0x31, 0x47,
    GP_PACKET, 2, 0x32, 0x45,
    GP_PACKET, 2, 0x33, 0x45,
    GP_PACKET, 2, 0x34, 0x4b,
    GP_PACKET, 2, 0x35, 0x4b,
    GP_PACKET, 2, 0x36, 0x49,
    GP_PACKET, 2, 0x37, 0x49,
    GP_PACKET, 2, 0x38, 0x5f,
    GP_PACKET, 2, 0x39, 0x5f,
    GP_PACKET, 2, 0x3a, 0x55,
    GP_PACKET, 2, 0x3b, 0x55,
    GP_PACKET, 2, 0x3c, 0x43,
    GP_PACKET, 2, 0x3d, 0x41,
    GP_PACKET, 2, 0x3e, 0x5f,
    GP_PACKET, 2, 0x3f, 0x5f,
    GP_PACKET, 2, 0x40, 0x5f,
    GP_PACKET, 2, 0x41, 0x5f,

    // GIP control group D (registers 0x42-0x57) - additional gate timing
    GP_PACKET, 2, 0x42, 0x5e,
    GP_PACKET, 2, 0x43, 0x5f,
    GP_PACKET, 2, 0x44, 0x75,
    GP_PACKET, 2, 0x45, 0x50,
    GP_PACKET, 2, 0x46, 0x46,
    GP_PACKET, 2, 0x47, 0x46,
    GP_PACKET, 2, 0x48, 0x44,
    GP_PACKET, 2, 0x49, 0x44,
    GP_PACKET, 2, 0x4a, 0x4a,
    GP_PACKET, 2, 0x4b, 0x4a,
    GP_PACKET, 2, 0x4c, 0x48,
    GP_PACKET, 2, 0x4d, 0x48,
    GP_PACKET, 2, 0x4e, 0x5f,
    GP_PACKET, 2, 0x4f, 0x5f,
    GP_PACKET, 2, 0x50, 0x55,
    GP_PACKET, 2, 0x51, 0x55,
    GP_PACKET, 2, 0x52, 0x42,
    GP_PACKET, 2, 0x53, 0x40,
    GP_PACKET, 2, 0x54, 0x5f,
    GP_PACKET, 2, 0x55, 0x5f,
    GP_PACKET, 2, 0x56, 0x5f,
    GP_PACKET, 2, 0x57, 0x5f,

    // GIP timing group E: STV/ETV pulse widths and start offsets
    GP_PACKET, 2, 0x58, 0x00,
    GP_PACKET, 2, 0x59, 0x00,
    GP_PACKET, 2, 0x5a, 0x00,
    GP_PACKET, 2, 0x5b, 0x30,  // STV_Num
    GP_PACKET, 2, 0x5c, 0x00,  // STV_S0
    GP_PACKET, 2, 0x5d, 0x30,  // STV_W, STV_S1
    GP_PACKET, 2, 0x5e, 0x00,  // STV_S2
    GP_PACKET, 2, 0x5f, 0x00,  // STV_S3
    GP_PACKET, 2, 0x60, 0x30,  // ETV_W, ETV_S1
    GP_PACKET, 2, 0x61, 0x00,  // ETV_S2
    GP_PACKET, 2, 0x62, 0x00,  // ETV_S3
    GP_PACKET, 2, 0x63, 0x06,  // SETV_ON
    GP_PACKET, 2, 0x64, 0x6a,  // SETV_OFF
    GP_PACKET, 2, 0x65, 0x45,  // ETV_EN, ETV_NUM
    GP_PACKET, 2, 0x66, 0xaf,  // ETV_S0
    GP_PACKET, 2, 0x67, 0x73,  // CKV0_NUM, CKV0_W
    GP_PACKET, 2, 0x68, 0x04,  // CKV0_S0
    GP_PACKET, 2, 0x69, 0x06,  // CKV0_ON
    GP_PACKET, 2, 0x6a, 0x6a,  // CKV0_OFF
    GP_PACKET, 2, 0x6b, 0x08,  // CKV0_DUM
    GP_PACKET, 2, 0x6c, 0x00,  // EOLR, GEQ_LINE, GEQ_W
    GP_PACKET, 2, 0x6d, 0x04,  // GEQ_GGND1
    GP_PACKET, 2, 0x6e, 0x04,  // GEQ_GGND2
    GP_PACKET, 2, 0x6f, 0x88,  // GIPDR, VGHO_SEL, VGLO_SEL

    GP_PACKET, 2, 0x70, 0x00,  // CKV1_NUM, CKV1_W
    GP_PACKET, 2, 0x71, 0x00,  // CKV1_S0
    GP_PACKET, 2, 0x72, 0x06,  // CKV1_ON
    GP_PACKET, 2, 0x73, 0x7b,  // CKV1_OFF
    GP_PACKET, 2, 0x74, 0x00,  // CKV1_DUM
    GP_PACKET, 2, 0x75, 0x07,  // FLM_EN, FLM_W
    GP_PACKET, 2, 0x76, 0x00,  // FLM_ON
    GP_PACKET, 2, 0x77, 0xd0,  // VEN_EN, VEN_W, FLM_NUM
    GP_PACKET, 2, 0x78, 0x17,  // FLM_OFF
    GP_PACKET, 2, 0x79, 0xb0,  // VEN_W
    GP_PACKET, 2, 0x7a, 0x00,  // VEN_S0
    GP_PACKET, 2, 0x7b, 0x00,  // VEN_S1
    GP_PACKET, 2, 0x7c, 0x00,  // VEN_DUM
    GP_PACKET, 2, 0x7d, 0x06,  // VEN_ON
    GP_PACKET, 2, 0x7e, 0x6a,  // VEN_OFF
    GP_PACKET, 2, 0x7f, 0x40,

    // --- Return to Page 0 and set Source/EQ enhancement bits ---
    GP_PACKET, 2, 0xe0, 0x00,
    GP_PACKET, 2, 0xe6, 0x02,  // Source driver enhancement setting
    GP_PACKET, 2, 0xe7, 0x0c, // Source driver enhancement setting

    // --- Standard DCS-equivalent commands, still sent via Generic Packet ---
    GP_PACKET, 2, 0x11, 0x00,  // SLPOUT: Sleep Out
    LCD_DELAY, 120,            // 120ms wait for panel power/oscillator stabilization
    GP_PACKET, 2, 0x29, 0x00,  // DISPON: Display On
    LCD_DELAY, 10,             // 10ms settling time
    GP_PACKET, 2, 0x35, 0x00,  // TEON: Tearing Effect Line On


    // =====================================================================
    // PHASE 3 - SSD2828 final config (full-speed video operation).
    // PLL and lane count are reprogrammed for the actual video data rate
    // now that the panel has been initialized at the slower setup speed.
    // =====================================================================

    // -------------------------------------------------------------------
    // 0xB7 - CFGR: Configuration Register (re-write, same as Phase 1)
    // -------------------------------------------------------------------
    3, 0xb7, 0x50, 0x00,

    // -------------------------------------------------------------------
    // 0xB8 - VCR: Virtual Channel ID Control Register
    // -------------------------------------------------------------------
    3, 0xb8, 0x00, 0x00,

    // -------------------------------------------------------------------
    // 0xB9 - PLL Control Register: disable PLL before reconfiguring
    // -------------------------------------------------------------------
    3, 0xb9, 0x00, 0x00,

    // -------------------------------------------------------------------
    // 0xBA - PLLCR: PLL Configuration Register (video-rate PLL settings)
    //   Data[0] = 0x31 -> NS[7:0] = 0x31 (NS = 49)
    //   Data[1] = 0x82 -> PLL range bits[15:14]=10 (251-500MHz range),
    //             MS[12:8] = 0x02 (MS = 2)
    // -------------------------------------------------------------------
    3, 0xba, 0x31, 0x82,

    // -------------------------------------------------------------------
    // 0xBB - LPCDR: LP Clock Divider Register (video-rate)
    //   Data[0] = 0x07 -> LPD[5:0] = 7
    // -------------------------------------------------------------------
    3, 0xbb, 0x07, 0x00,

    // -------------------------------------------------------------------
    // 0xB9 - PLL Control Register: enable PLL with new settings
    // -------------------------------------------------------------------
    3, 0xb9, 0x01, 0x00,

    // -------------------------------------------------------------------
    // 0xC9 - Analog/Timing Control Register (HS-Data-Zero / HS-Data-Prepare)
    // -------------------------------------------------------------------
    3, 0xc9, 0x02, 0x23,
    LCD_DELAY, 100,            // 100ms wait for PLL lock at new frequency

    // -------------------------------------------------------------------
    // 0xCA - Analog/Timing Control Register: Clock lane timing
    //   Data[0] = 0x01 -> Clk Prepare
    //   Data[1] = 0x23 -> Clk Zero
    // -------------------------------------------------------------------
    3, 0xca, 0x01, 0x23,

    // -------------------------------------------------------------------
    // 0xCB - Analog/Timing Control Register: Clock post/period timing
    //   Data[0] = 0x10 -> Clk Post
    //   Data[1] = 0x05 -> Clk Per
    // -------------------------------------------------------------------
    3, 0xcb, 0x10, 0x05,

    // -------------------------------------------------------------------
    // 0xCC - Analog/Timing Control Register: HS/Clk trail timing
    //   Data[0] = 0x05 -> HS Trail
    //   Data[1] = 0x10 -> Clk Trail
    // -------------------------------------------------------------------
    3, 0xcc, 0x05, 0x10,

    // -------------------------------------------------------------------
    // 0xD0 - Reserved/analog timing register
    // -------------------------------------------------------------------
    3, 0xd0, 0x00, 0x00,

    // -------------------------------------------------------------------
    // 0xB1 - VICR1: Video Interface Configuration Register 1
    //   Data[0] = LCD_HSPW -> HSA (Horizontal Sync Pulse Width)  [undefined - see TODO]
    //   Data[1] = LCD_VSPW -> VSA (Vertical Sync Pulse Width)    [undefined - see TODO]
    // -------------------------------------------------------------------
    3, 0xb1, LCD_HSPW, LCD_VSPW,

    // -------------------------------------------------------------------
    // 0xB2 - VICR2: Video Interface Configuration Register 2
    //   Data[0] = LCD_HBPD -> HBP (Horizontal Back Porch)  [undefined - see TODO]
    //   Data[1] = LCD_VBPD -> VBP (Vertical Back Porch)    [undefined - see TODO]
    // -------------------------------------------------------------------
    3, 0xb2, LCD_HBPD, LCD_VBPD,

    // -------------------------------------------------------------------
    // 0xB3 - VICR3: Video Interface Configuration Register 3
    //   Data[0] = LCD_HFPD -> HFP (Horizontal Front Porch)  [undefined - see TODO]
    //   Data[1] = LCD_VFPD -> VFP (Vertical Front Porch)    [undefined - see TODO]
    // -------------------------------------------------------------------
    3, 0xb3, LCD_HFPD, LCD_VFPD,

    // -------------------------------------------------------------------
    // 0xB4 - VICR4: Horizontal Active Period
    //   Data[0] = hsize & 0xFF       = 600 & 0xFF = 0x58
    //   Data[1] = (hsize >> 8) & 0xFF = 0x02
    // -------------------------------------------------------------------
    3, 0xb4, (hsize & 0xff), ((hsize >> 8) & 0xff),

    // -------------------------------------------------------------------
    // 0xB5 - VICR5: Vertical Active Period
    //   Data[0] = vsize & 0xFF        = 1424 & 0xFF = 0x90
    //   Data[1] = (vsize >> 8) & 0xFF = 0x05
    // -------------------------------------------------------------------
    3, 0xb5, (vsize & 0xff), ((vsize >> 8) & 0xff),

    // -------------------------------------------------------------------
    // 0xB6 - VICR6: RGB Interface / Pixel Format Control
    //   Data[0] = 0x0B -> D1-D0 = 11 -> 24bpp RGB888 format
    //   Data[1] = 0x00 -> D15=VS, D14=HS, D13=CLK polarity bits = 0;
    //             D8=0 -> Video with blanking packet
    // -------------------------------------------------------------------
    3, 0xb6, 0x0b, 0x00,

    // -------------------------------------------------------------------
    // 0xDE - Lane Configuration Register (final, full-speed lane count)
    //   Data[0] = 0x03 -> 11 = 4 MIPI data lanes
    //   Data[1] = 0x00
    // -------------------------------------------------------------------
    3, 0xde, 0x03, 0x00,

    // -------------------------------------------------------------------
    // 0xD6 - Color order / byte order configuration
    //   Data[0] = 0x01 -> D0=1 (BGR color order), D1 implied MSB-first
    //   Data[1] = 0x00
    // -------------------------------------------------------------------
    3, 0xd6, 0x01, 0x00,

    // -------------------------------------------------------------------
    // 0xDB - Reserved/analog control register
    //   Data[0] = 0x58
    // -------------------------------------------------------------------
    3, 0xdb, 0x58, 0x00,

    // -------------------------------------------------------------------
    // 0xB7 - CFGR: Configuration Register (final video-mode configuration)
    //   Data[0] = 0x4B -> VEN=1 (enable video), HS=1 (high speed mode),
    //             CKE=1 (clock enable), other control bits set for
    //             continuous video streaming
    //   Data[1] = 0x02 -> HCLK=1 (select pclk as reference where applicable)
    // -------------------------------------------------------------------
    3, 0xb7, 0x4b, 0x02,

    // -------------------------------------------------------------------
    // 0x2C - Start Memory Write / enter video transfer mode
    //   No data bytes. This DCS-style command (sent as a direct SSD2828
    //   command, not through Generic Packet Drop) triggers the SSD2828
    //   to begin continuous RGB-to-MIPI video stream transmission.
    // -------------------------------------------------------------------
    1, 0x2c,

    0                          // END OF LIST
};

