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
#define DISPLAY_CHAR_BLANK_IDX 52

#define DISPLAY_CHAR_OVERLAY_COUNT 3
#define DISPLAY_CHAR_OVERLAY_DOT 

#define DISPLAY_IND_W      38
#define DISPLAY_INT_H      128

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
	INDICATOR_4W     = 11,
	INDICATOR_DIODE  = 12,
	INDICATOR_CONT   = 13,
	INDICATOR_STAR   = 14,
	INDICATOR_BLANK  = 15
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
	{  53, 140,   4,  20,   8 }, // OVERLAY_PERIOD
	{  54, 140,   4,  24,  12 }, // OVERLAY_COMMA
	{  54,  60,   4,  20,   8 }, // OVERLAY_DOT
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
	'.', ',', ':', ';',
	'!', '?', '\'', '"',
	'-', '^', '#', '*',
	'+', '=', '<', '>',
	' ',
	0 };




#endif
