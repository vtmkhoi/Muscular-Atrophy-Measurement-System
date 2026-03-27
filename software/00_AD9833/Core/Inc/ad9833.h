#ifndef __AD9833_H
#define __AD9833_H

#include "main.h"
#include <stdint.h>

typedef enum
{
    AD9833_WAVE_SINE = 0,
    AD9833_WAVE_TRIANGLE,
    AD9833_WAVE_SQUARE
} AD9833_Waveform_t;

void AD9833_Init(void);
void AD9833_SetFrequencyHz(uint32_t fout_hz);
void AD9833_SetPhaseDeg(float phase_deg);
void AD9833_SetWaveform(AD9833_Waveform_t wave);

#endif
