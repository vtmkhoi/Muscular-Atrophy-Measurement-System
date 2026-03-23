/*
 * ad9833.c
 *
 *  Created on: Mar 24, 2026
 *      Author: minhk
 */

#include "ad9833.h"

/*
 * Module AD9833 phổ biến thường dùng oscillator 25 MHz.
 * Nếu sau này bạn đọc trên vỏ oscillator thấy khác, sửa lại macro này.
 */
#define AD9833_MCLK_HZ        25000000UL

/* Vì project của bạn không có spi.h, khai báo extern ở đây */
extern SPI_HandleTypeDef hspi4;

/* Dùng thẳng pin PE11 để khỏi phụ thuộc label */
#define AD9833_FSYNC_GPIO_Port   GPIOE
#define AD9833_FSYNC_Pin         GPIO_PIN_11

/* Control register bits */
#define AD9833_B28            (1U << 13)
#define AD9833_HLB            (1U << 12)
#define AD9833_FSEL           (1U << 11)
#define AD9833_PSEL           (1U << 10)
#define AD9833_RESET          (1U << 8)
#define AD9833_SLEEP1         (1U << 7)
#define AD9833_SLEEP12        (1U << 6)
#define AD9833_OPBITEN        (1U << 5)
#define AD9833_DIV2           (1U << 3)
#define AD9833_MODE           (1U << 1)

/* Register addresses */
#define AD9833_REG_FREQ0      0x4000
#define AD9833_REG_PHASE0     0xC000

static uint16_t g_ctrl = AD9833_B28;

static void AD9833_FSYNC_Low(void)
{
    HAL_GPIO_WritePin(AD9833_FSYNC_GPIO_Port, AD9833_FSYNC_Pin, GPIO_PIN_RESET);
}

static void AD9833_FSYNC_High(void)
{
    HAL_GPIO_WritePin(AD9833_FSYNC_GPIO_Port, AD9833_FSYNC_Pin, GPIO_PIN_SET);
}

static void AD9833_WriteWord(uint16_t word)
{
    uint8_t tx[2];
    tx[0] = (uint8_t)(word >> 8);
    tx[1] = (uint8_t)(word & 0xFF);

    AD9833_FSYNC_Low();
    HAL_SPI_Transmit(&hspi4, tx, 2, HAL_MAX_DELAY);
    AD9833_FSYNC_High();
}

static void AD9833_WriteControl(uint16_t ctrl)
{
    g_ctrl = ctrl;
    AD9833_WriteWord(g_ctrl);
}

void AD9833_SetFrequencyHz(uint32_t fout_hz)
{
    uint32_t freq_word = (uint32_t)(((uint64_t)fout_hz << 28) / AD9833_MCLK_HZ);

    uint16_t lsb = AD9833_REG_FREQ0 | (uint16_t)(freq_word & 0x3FFF);
    uint16_t msb = AD9833_REG_FREQ0 | (uint16_t)((freq_word >> 14) & 0x3FFF);

    AD9833_WriteWord(lsb);
    AD9833_WriteWord(msb);
}

void AD9833_SetPhaseDeg(float phase_deg)
{
    while (phase_deg < 0.0f)    phase_deg += 360.0f;
    while (phase_deg >= 360.0f) phase_deg -= 360.0f;

    uint16_t phase_word = (uint16_t)((phase_deg * 4096.0f) / 360.0f) & 0x0FFF;
    AD9833_WriteWord(AD9833_REG_PHASE0 | phase_word);
}

void AD9833_SetWaveform(AD9833_Waveform_t wave)
{
    uint16_t ctrl = g_ctrl;

    ctrl &= ~(AD9833_OPBITEN | AD9833_DIV2 | AD9833_MODE |
              AD9833_SLEEP1 | AD9833_SLEEP12);

    switch (wave)
    {
    case AD9833_WAVE_SINE:
        break;

    case AD9833_WAVE_TRIANGLE:
        ctrl |= AD9833_MODE;
        break;

    case AD9833_WAVE_SQUARE:
        ctrl |= AD9833_OPBITEN | AD9833_DIV2;
        break;

    default:
        break;
    }

    AD9833_WriteControl(ctrl);
}

void AD9833_Init(void)
{
    AD9833_FSYNC_High();
    HAL_Delay(10);

    /* reset + B28 */
    AD9833_WriteControl(AD9833_B28 | AD9833_RESET);

    /* cấu hình mặc định */
    AD9833_SetFrequencyHz(10000);     // 10 kHz
    AD9833_SetPhaseDeg(0.0f);
    AD9833_SetWaveform(AD9833_WAVE_SINE);

    /* clear reset => bắt đầu phát sóng */
    AD9833_WriteControl(AD9833_B28);
}
