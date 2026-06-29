/*
DC Motor PWM Control With Manual and Test Modes

This program controls a motor through PWM using an ESP32 LEDC channel.
One button toggles motor on/off for manual speed control from a potentiometer,
while another button runs a fade up/down test sequence.
*/

#include <Arduino.h>
#include <Bounce2.h>
#include "driver/ledc.h"

// Input and output pins.
constexpr uint8_t speed_ctrl_pin = 5;
constexpr uint8_t motor_pin = 18;
constexpr uint8_t btn_onoff_pin = 12;
constexpr uint8_t btn_test_pin = 13;

// ADC settings for speed control input.
constexpr uint16_t speed_ctrl_resolution = 9;
constexpr uint16_t min_speed_ctrl = 0;
constexpr uint16_t max_speed_ctrl = (1 << speed_ctrl_resolution) - 1;

// PWM settings and constrained motor speed range.
constexpr uint16_t pwm_resolution = 9;
constexpr uint16_t max_speed = (1 << pwm_resolution) - 1;
constexpr uint16_t min_speed = max_speed * 0.4;

// Debounced buttons.
Bounce2::Button btn_onoff = Bounce2::Button();
Bounce2::Button btn_test = Bounce2::Button();

// true: motor running in manual mode, false: motor stopped.
bool motor_state;

void motorOn()
{
  // Start and keep the maximum speed for a short time to overcome initial inertia.
  ledcWrite(0, max_speed);
  delay(100);
}

void motorOff()
{
  ledcWrite(0, 0);
}

// Runs a blocking speed sweep test from minimum to maximum and back.
void testMotor()
{
  ledcWrite(0, min_speed);

  ledc_fade_func_install(0);

  ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, max_speed, 10000);
  ledc_fade_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, LEDC_FADE_WAIT_DONE);

  ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, min_speed, 10000);
  ledc_fade_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, LEDC_FADE_WAIT_DONE);

  ledc_fade_func_uninstall();
}

void setup()
{
  Serial.begin(115200);

  // Configure debounced buttons.
  btn_onoff.attach(btn_onoff_pin, INPUT_PULLUP);
  btn_onoff.interval(20);
  btn_onoff.setPressedState(LOW);

  btn_test.attach(btn_test_pin, INPUT_PULLUP);
  btn_test.interval(20);
  btn_test.setPressedState(LOW);

  // Configure ADC for potentiometer input.
  analogReadResolution(speed_ctrl_resolution);
  analogSetAttenuation(ADC_0db);

  // Configure LEDC PWM channel for motor drive.
  ledcSetup(0, 20000, pwm_resolution);
  ledcAttachPin(motor_pin, 0);

  motorOff();
  motor_state = false;
}

void loop()
{
  btn_onoff.update();
  btn_test.update();

  if (btn_onoff.pressed())
  {
    if (motor_state)
      motorOff();
    else
      motorOn();

    motor_state = !motor_state;
  }

  // Run test sweep only when motor is currently off.
  if (btn_test.pressed() && !motor_state)
  {
    motorOn();
    testMotor();
    motorOff();
  }

  // In manual mode, map potentiometer value to PWM duty.
  if (motor_state)
  {
    uint16_t speed_ctrl = analogRead(speed_ctrl_pin);
    uint16_t speed = map(speed_ctrl, min_speed_ctrl, max_speed_ctrl, min_speed, max_speed);
    ledcWrite(0, speed);
  }
}
