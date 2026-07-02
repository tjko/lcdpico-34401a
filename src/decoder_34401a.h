#ifndef DECODER_34401A_H
#define DECODER_34401A_H

#include "pico.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef enum dmm_annunciators {
	ANN_SMP   = 0,
	ANN_ADRS  = 1,
	ANN_RMT   = 2,
	ANN_MAN   = 3,
	ANN_TRIG  = 4,
	ANN_HOLD  = 5,
	ANN_MEM   = 6,
	ANN_RATIO = 7,
	ANN_MATH  = 8,
	ANN_ERROR = 9,
	ANN_REAR  = 10,
	ANN_SHIFT = 11,
	ANN_DIODE = 12,
	ANN_CONT  = 13,
	ANN_4WIRE = 14,
} dmm_annunciators_t;


// ===== Extracted "display model" =====
extern volatile char     dmm_main[16];          // null-terminated
extern volatile uint16_t dmm_ann_state;         // annunciator bitfield (incl shift)
extern volatile int16_t  dmm_bar;               // parsed bargraph value
extern volatile uint8_t  dmm_bar_style;         // 0=POSITIVE, 1=FULLSCALE
extern volatile uint32_t dmm_new_data_counter;  // increments whenever any of above changes
extern volatile uint32_t dmm_main_counter;
extern volatile uint32_t dmm_ann_counter;
extern volatile uint32_t dmm_bar_counter;
extern volatile uint16_t dmm_blink_mask;   // bits 0..13 = blink this character position
//extern volatile uint8_t  dmm_text_dim;     // 0 = normal, 1 = dim

// ===== Decoder API =====
void decoder34401_init(void);       // enable DWT micros (recommended)
void __time_critical_func(decoder34401_sckedge)(void);    // call from GPIO interrupt callback
void decoder34401_process(void);    // call frequently in main loop

#ifdef __cplusplus
}
#endif

#endif
