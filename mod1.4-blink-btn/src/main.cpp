/*
Two-Button LED Mode Controller

Two LEDs are controlled by two buttons:
- Short press BTN1: decrease blink delay in BLINK mode, or return to BLINK from ON/OFF.
- Short press BTN2: increase blink delay in BLINK mode, or return to BLINK from ON/OFF.
- Press both buttons together: switch LEDs to always ON.
- Long press BTN1 or BTN2: switch LEDs to always OFF.

In BLINK mode, LEDs alternate (LED1 -> LED2 -> LED1 ...) using a configurable delay.
*/

#include <Arduino.h>

// Output pins for LEDs.
const uint8_t led_1_pin = 10;
const uint8_t led_2_pin = 12;

// Input pins for buttons.
const uint8_t btn_1_pin = 4;
const uint8_t btn_2_pin = 16;

// Blink delay tuning parameters.
const uint16_t blink_delay_step_ms = 100;
const uint16_t min_blink_delay_ms = 100;
const uint16_t max_blink_delay_ms = 1000;

// Press duration that separates short and long button actions.
const uint16_t long_press_threshold_ms = 1000;

// Raw states of both buttons.
struct BtnStates
{
  bool btn_1;
  bool btn_2;

  bool operator==(const BtnStates &other) const
  {
    return btn_1 == other.btn_1 && btn_2 == other.btn_2;
  }
};

// Current state classification of the two buttons together.
enum BtnsState
{
  BTN_1_HELD,
  BTN_2_HELD,
  BOTH_HELD,
  NONE_HELD
};

// Discrete actions produced when buttons are released.
enum BtnAction
{
  NONE,
  BTN_1,
  BTN_1_LONG,
  BTN_2,
  BTN_2_LONG,
  BOTH
};

// High-level operating modes of LEDs.
enum LEDsMode
{
  BLINK,
  ON,
  OFF
};

// Active LED in blink alternation.
enum BlinkState
{
  LED_1_ON,
  LED_2_ON
};

// Full state container for LED control logic.
struct LEDsState
{
  LEDsMode mode;
  BlinkState blink_state;
  uint16_t blink_delay_ms;
  uint64_t last_blink_time_ms;
};

uint64_t espMillis()
{
  return esp_timer_get_time() / 1000;
}

// Returns true when duration has elapsed since start_time_ms.
bool timePassed(uint64_t start_time_ms, uint16_t duration_ms)
{
  return espMillis() - start_time_ms >= duration_ms;
}

// Read and debounce buttons: accept state only when two reads match.
BtnStates readBtnStates()
{
  // Last stable state is reused if the current sample pair is inconsistent.
  static BtnStates prev_states = {false, false};

  BtnStates read_states_1 = {digitalRead(btn_1_pin) == HIGH, digitalRead(btn_2_pin) == HIGH};

  delay(20);

  BtnStates read_states_2 = {digitalRead(btn_1_pin) == HIGH, digitalRead(btn_2_pin) == HIGH};

  if (read_states_1 == read_states_2)
  {
    prev_states = read_states_1;
  }

  return prev_states;
}

// Translate raw button states into a held-state.
BtnsState readBtnsState(BtnsState prev_btns_state)
{
  BtnsState btns_state;

  BtnStates btn_states = readBtnStates();

  if (btn_states.btn_1 && btn_states.btn_2)
  {
    btns_state = BOTH_HELD;
  }
  // Keep BOTH_HELD latched until both buttons are released.
  else if (prev_btns_state == BOTH_HELD && (btn_states.btn_1 || btn_states.btn_2))
  {
    btns_state = BOTH_HELD;
  }
  else if (btn_states.btn_1)
  {
    btns_state = BTN_1_HELD;
  }
  else if (btn_states.btn_2)
  {
    btns_state = BTN_2_HELD;
  }
  else
  {
    btns_state = NONE_HELD;
  }

  return btns_state;
}

// Convert held-state transitions into one-shot actions on release.
BtnAction readBtnsAction()
{
  // Persistent state between loop() iterations.
  static BtnsState prev_btns_state = NONE_HELD;
  static uint32_t press_time_ms = 0;

  BtnsState btns_state = readBtnsState(prev_btns_state);

  if (prev_btns_state == NONE_HELD && btns_state != NONE_HELD)
  {
    // Start press timing at the moment any button becomes held.
    press_time_ms = espMillis();
  }

  BtnAction action = NONE;

  if (btns_state == NONE_HELD)
  {
    // Action is emitted only once, when buttons are released.
    if (prev_btns_state == BOTH_HELD)
    {
      action = BOTH;
    }
    else if (prev_btns_state == BTN_1_HELD)
    {
      action = timePassed(press_time_ms, long_press_threshold_ms) ? BTN_1_LONG : BTN_1;
    }
    else if (prev_btns_state == BTN_2_HELD)
    {
      action = timePassed(press_time_ms, long_press_threshold_ms) ? BTN_2_LONG : BTN_2;
    }
  }

  prev_btns_state = btns_state;

  return action;
}

// Increase/decrease blink delay with clamping to configured bounds.
LEDsState changeBlinkDelay(LEDsState leds_state, bool increase)
{
  if (increase)
  {
    leds_state.blink_delay_ms += blink_delay_step_ms;
    if (leds_state.blink_delay_ms > max_blink_delay_ms)
    {
      leds_state.blink_delay_ms = max_blink_delay_ms;
    }
    else
    {
      Serial.printf("Delay: %dms\n", leds_state.blink_delay_ms);
    }
  }
  else
  {
    leds_state.blink_delay_ms -= blink_delay_step_ms;
    if (leds_state.blink_delay_ms < min_blink_delay_ms)
    {
      leds_state.blink_delay_ms = min_blink_delay_ms;
    }
    else
    {
      Serial.printf("Delay: %dms\n", leds_state.blink_delay_ms);
    }
  }

  return leds_state;
}

// Enter BLINK mode and restart blink alternation timing.
LEDsState initBlinkMode(LEDsState leds_state)
{
  leds_state.mode = BLINK;
  leds_state.blink_state = LED_1_ON;
  leds_state.last_blink_time_ms = espMillis();
  Serial.println("Mode: BLINK");

  return leds_state;
}

// Overload to enter BLINK mode with an explicit delay value.
LEDsState initBlinkMode(LEDsState leds_state, uint16_t blink_delay_ms)
{
  leds_state = initBlinkMode(leds_state);
  leds_state.blink_delay_ms = blink_delay_ms;

  return leds_state;
}

// Enter ON mode.
LEDsState initOnMode(LEDsState leds_state)
{
  leds_state.mode = ON;
  Serial.println("Mode: ON");

  return leds_state;
}

// Enter OFF mode.
LEDsState initOffMode(LEDsState leds_state)
{
  leds_state.mode = OFF;
  Serial.println("Mode: OFF");

  return leds_state;
}

// Get LEDs state for the next iteration.
LEDsState nextLEDsState(LEDsState leds_state)
{
  // Advance blink state when enough time has elapsed.
  if (leds_state.mode == BLINK && timePassed(leds_state.last_blink_time_ms, leds_state.blink_delay_ms))
  {
    // Toggle active LED: LED1 <-> LED2.
    leds_state.blink_state = (leds_state.blink_state == LED_1_ON) ? LED_2_ON : LED_1_ON;
    leds_state.last_blink_time_ms = espMillis();
  }

  return leds_state;
}

// Apply BLINK sub-state to physical pins.
void setBlinkState(BlinkState state)
{
  digitalWrite(led_1_pin, state == LED_1_ON);
  digitalWrite(led_2_pin, state == LED_2_ON);
}

// Apply high-level LED state to hardware outputs.
void setLEDsState(LEDsState leds_state)
{
  switch (leds_state.mode)
  {
  case BLINK:
    setBlinkState(leds_state.blink_state);
    break;
  case ON:
    digitalWrite(led_1_pin, HIGH);
    digitalWrite(led_2_pin, HIGH);
    break;
  case OFF:
    digitalWrite(led_1_pin, LOW);
    digitalWrite(led_2_pin, LOW);
    break;
  }
}

// Global runtime state used by setup() and loop().
LEDsState leds_state;

void setup()
{
  Serial.begin(115200);

  pinMode(led_1_pin, OUTPUT);
  pinMode(led_2_pin, OUTPUT);

  pinMode(btn_1_pin, INPUT_PULLDOWN);
  pinMode(btn_2_pin, INPUT_PULLDOWN);

  // Start in BLINK mode with a medium delay.
  leds_state = initBlinkMode(leds_state, 500);
}

void loop()
{
  // Handle one-shot button actions and adjust mode/timing state.
  switch (readBtnsAction())
  {
  case BTN_1:
    Serial.println("Button 1 released");
    if (leds_state.mode == BLINK)
    {
      leds_state = changeBlinkDelay(leds_state, false);
    }
    else
    {
      leds_state = initBlinkMode(leds_state);
    }
    break;
  case BTN_2:
    Serial.println("Button 2 released");
    if (leds_state.mode == BLINK)
    {
      leds_state = changeBlinkDelay(leds_state, true);
    }
    else
    {
      leds_state = initBlinkMode(leds_state);
    }
    break;
  case BOTH:
    Serial.println("Both buttons released");
    leds_state = initOnMode(leds_state);
    break;
  case BTN_1_LONG:
    Serial.println("Button 1 long pressed");
    leds_state = initOffMode(leds_state);
    break;
  case BTN_2_LONG:
    Serial.println("Button 2 long pressed");
    leds_state = initOffMode(leds_state);
    break;
  case NONE:
    break;
  }

  // Push state to outputs first, then update timed blink transition.
  setLEDsState(leds_state);

  leds_state = nextLEDsState(leds_state);
}
