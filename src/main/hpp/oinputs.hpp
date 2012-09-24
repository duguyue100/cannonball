#pragma once

#include "outrun.hpp"
#include "input.hpp"

class OInputs
{
public:

    const static uint8_t BRAKE_THRESHOLD1 = 0x80;
    const static uint8_t BRAKE_THRESHOLD2 = 0xA0;
    const static uint8_t BRAKE_THRESHOLD3 = 0xC0;
    const static uint8_t BRAKE_THRESHOLD4 = 0xE0;

    int8_t crash_input;

    // Acceleration Input
    int16_t input_acc;

    // Steering Input
    int16_t input_steering;

    // Processed / Adjusted Values
    int16_t steering_adjust;
    int16_t acc_adjust;
    int16_t brake_adjust;
    bool gear; // True is high

    OInputs(void);
    ~OInputs(void);

    void init();
    void simulate_analog();
    void adjust_inputs();
    void do_gear();
    void do_credits();

private:
    // ------------------------------------------------------------------------
    // Variables for port
    // ------------------------------------------------------------------------

    // Amount to adjust steering per tick.
    const static uint8_t STEERING_ADJUST = 0x3;

    // Amount to adjust acceleration per tick.
    const static uint8_t ACC_ADJUST = 0x10;

    // Amount to adjust brake per tick.
    const static uint8_t BRAKE_ADJUST = 0x10;

    // ------------------------------------------------------------------------
    // Variables from original code
    // ------------------------------------------------------------------------

    const static uint8_t STEERING_MIN = 0x48;
    const static uint8_t STEERING_MAX = 0xB8;
    const static uint8_t STEERING_CENTRE = 0x80;
    
    // Current steering value
    int16_t steering_old;
    int16_t steering_change;

    const static uint8_t PEDAL_MIN = 0x30;
    const static uint8_t PEDAL_MAX = 0x90;

    // Brake Input
    int16_t input_brake;
};

extern OInputs oinputs;

