#include <gb/gb.h>
#include "sound.h"

// Initialize the sound registers (must call once at startup)
void init_sound(void) {
    // Turn on sound hardware
    NR52_REG = 0x80;

    // Set master volume (Left/Right) to max
    NR50_REG = 0x77;

    // Enable Channel 1 for both Left and Right output
    NR51_REG = 0x11;
}

void play_select_sound(void) {
    // High-pitched short beep
    // Sweep: Off
    NR10_REG = 0x00;
    // Duty cycle 50%, Length (short)
    NR11_REG = 0x81;
    // Volume envelope: Start loud, fade out fast
    NR12_REG = 0xF1;
    // Frequency Low bits
    NR13_REG = 0x00;
    // Frequency High bits + Trigger (0xC0 = Trigger + Length Enable) + Freq 0x07 (High pitch)
    NR14_REG = 0xC7;
}

void play_error_sound(void) {
    // Low-pitched buzz
    NR10_REG = 0x00;
    // Duty cycle 50%, Length slightly longer
    NR11_REG = 0x84;
    // Volume: Constant volume, no fade
    NR12_REG = 0x70;
    // Frequency Low bits
    NR13_REG = 0x00;
    // Frequency High bits + Trigger + Freq 0x01 (Low pitch)
    NR14_REG = 0xC1;
}
