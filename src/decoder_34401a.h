#ifndef DECODER_34401A_H
#define DECODER_34401A_H

#include "pico.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

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
void Decoder34401_Init(void);       // enable DWT micros (recommended)
void __time_critical_func(Decoder34401_SckEdge)(void);    // call from EXTI callback on PB13
void Decoder34401_Process(void);    // call frequently in main loop

#ifdef __cplusplus
}
#endif

#endif
