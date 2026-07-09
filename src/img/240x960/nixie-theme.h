/* nixie-theme.h for 240x960 resolution */

#define DISPLAY_GRAPHICS "img/240x960/nixie1.png"
#define INDICATOR_GRAPHICS "img/240x960/nixie2.png"

#define DISPLAY_X_OFFSET   50
#define DISPLAY_Y_OFFSET   0

#define DISPLAY_CHAR_W     190
#define DISPLAY_CHAR_H     80
#define DISPLAY_CHAR_COUNT 55
#define DISPLAY_CHAR_MAP_W 4
#define DISPLAY_CHAR_MAP_H 14
#define DISPLAY_CHAR_BLANK_IDX 53

#define DISPLAY_CHAR_OVERLAY_COUNT 3

#define DISPLAY_IND_W      38
#define DISPLAY_IND_H      80

#define DISPLAY_IND_COUNT  16
#define DISPLAY_IND_MAP_W  8
#define DISPLAY_IND_MAP_H  2



#if !defined(__ASSEMBLER__)

/* values map to location in graphics asset */
enum lcd_indicators_types {
	INDICATOR_ADRS   = 0,
	INDICATOR_RMT    = 1,
	INDICATOR_MAN    = 2,
	INDICATOR_TRIG   = 3,
	INDICATOR_HOLD   = 4,
	INDICATOR_MEM    = 5,
	INDICATOR_RATIO  = 6,
	INDICATOR_MATH   = 7,
 	INDICATOR_ERROR  = 8,
	INDICATOR_REAR   = 9,
	INDICATOR_SHIFT  = 10,
	INDICATOR_DIODE  = 11,
	INDICATOR_CONT   = 12,
	INDICATOR_4W     = 13,
	INDICATOR_SMP    = 14,
	INDICATOR_BLANK  = 15
};

const int8_t lcd_indicator_map[16] = {
	INDICATOR_SMP,
	INDICATOR_ADRS,
	INDICATOR_RMT,
	INDICATOR_MAN,
	INDICATOR_TRIG,
	INDICATOR_HOLD,
	INDICATOR_MEM,
	INDICATOR_RATIO,
	INDICATOR_MATH,
	INDICATOR_ERROR,
	INDICATOR_REAR,
	INDICATOR_SHIFT,
	INDICATOR_DIODE,
	INDICATOR_CONT,
	INDICATOR_4W,
	INDICATOR_BLANK,
};

struct lcd_indicator {
	int16_t tile;
	int16_t h;
	int16_t y;
	int8_t mode;
	uint16_t mask;
};

const struct lcd_indicator display_indicators[] = {
	{  0, -1, 788, 0, 0x0001 },
	{  1, -1, 708, 0, 0x0002 },
	{  2, -1, 628, 0, 0x0004 },
	{  3, -1, 548, 1, 0x0018 },
	{  4, -1, 548, 2, 0x0018 },
	{  5, -1, 468, 0, 0x0020 },
	{  6, -1, 388, 0, 0x0040 },
	{  7, -1, 308, 0, 0x0080 },
	{  8, -1,  68, 0, 0x0100 },
	{  9, -1, 228, 0, 0x0200 },
	{ 10, -1, 148, 0, 0x0400 },
	{ 11, 64,   0, 1, 0x3800 },
	{ 12, 64,   0, 2, 0x3800 },
	{ 13, 64,   0, 2, 0x3800 },
	{ 14, 64, 896, 0, 0x4000 },
	{ 15, -1,  -1, 0, 0x0000 },
};

enum lcd_char_overlays {
	OVERLAY_PERIOD   = 0,
	OVERLAY_COMMA    = 1,
	OVERLAY_DOT      = 2
};

struct lcd_char_overlay {
	int16_t tile;
	int16_t x;
	int16_t y;
	int16_t w;
	int16_t h;
};

const struct lcd_char_overlay display_char_overlays[] = {
	{  54, 140,   4,  20,   8 }, // OVERLAY_PERIOD
	{  55, 140,   4,  24,  12 }, // OVERLAY_COMMA
	{  55,  60,   4,  20,   8 }, // OVERLAY_DOT
};

const int display_chars[] = {
	'0', '1', '2', '3',
	'4', '5', '6', '7',
	'8', '9', 'A', 'B',
	'C', 'D', 'E', 'F',
	'G', 'H', 'I', 'J',
	'K', 'L', 'M', 'N',
	'O', 'P', 'Q', 'R',
	'S', 'T', 'U', 'V',
	'W', 'X', 'Y', 'Z',
	'd', 'k', 'm', 'n',
	'z', '!', '?', '\'',
	'-', '^', '#', '/',
	'+', '=', '<', '>',
	'_', ' ',
	-1 };




#endif
