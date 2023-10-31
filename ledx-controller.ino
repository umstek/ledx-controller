#define LED_RED 3
#define LED_GREEN 5
#define LED_BLUE 6
#define POT A3
#define LDR A0
#define LED_PWM 10
#define BTN 7
#define MAINS_SENSE 13
#define NUM_MODES 3

enum BUTTON_STATE {
  PUSHED = LOW,
  RELEASED = HIGH
};

enum ILLUMINATION {
  LIGHT,
  INDETERMINATE,
  DARKNESS
};

enum MAINS {
  POWER_CUT,
  HAS_POWER
};

enum LIGHT_STATE {
  AUTO,
  FORCE_ON,
  FORCE_OFF
};

byte mode = 0;

//////////////////////////////
/* COMMON UTILITY FUNCTIONS */

void blink(int pin) {
  digitalWrite(pin, HIGH);
  delay(200);
  digitalWrite(pin, LOW);
  delay(200);
}

void blinkNumber(int num, int pin) {
  for (int i = 0; i < num; i++) {
    blink(pin);
  }
}

void pulse() {
  for (int i = 0; i < 256; i += 1) {
    analogWrite(LED_PWM, i);
    delay(1);
  }
  for (int i = 255; i >= 0; i -= 1) {
    analogWrite(LED_PWM, i);
    delay(2);
  }
  analogWrite(LED_PWM, 0);
  delay(232);
}

/* END COMMON UTILITY FUNCTIONS */
//////////////////////////////////

//////////////////////////////////////////////////////
/* POWER-ON SELF TESTS, THOUGH THIS HAS NO FEEDBACK */

void post() {
  pulse();
  blink(LED_RED);
  blink(LED_GREEN);
  blink(LED_BLUE);
}

/* END POWER-ON SELF TESTS */
/////////////////////////////

//////////////////////////////////////
/* BUTTON PRESS DETECT AND DEBOUNCE */

const unsigned long DEBOUNCE_DURATION = 50;
void checkButtonState() {
  static byte lastButtonState = RELEASED;
  static unsigned long lastTimeButtonStateChanged = 0;

  if (millis() - lastTimeButtonStateChanged > DEBOUNCE_DURATION) {
    byte buttonState = digitalRead(BTN);
    if (buttonState != lastButtonState) {
      lastTimeButtonStateChanged = millis();
      lastButtonState = buttonState;
      if (buttonState == HIGH) {
        mode = (mode + 1) % NUM_MODES;
        digitalWrite(LED_RED, LOW);
        digitalWrite(LED_GREEN, LOW);
        digitalWrite(LED_BLUE, LOW);
        blinkNumber(mode + 1, LED_BLUE);
      }
    }
  }
}

/* END BUTTON PRESS DETECT AND DEBOUNCE */
//////////////////////////////////////////

void setup() {
  Serial.begin(9600);

  pinMode(POT, INPUT);
  pinMode(LDR, INPUT);
  pinMode(BTN, INPUT_PULLUP);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_PWM, OUTPUT);
  pinMode(MAINS_SENSE, INPUT);

  post();
}

const int LIGHT_BELOW = 190;
const int DARKNESS_ABOVE = 245;
ILLUMINATION getIllumination() {
  int ldr_value = analogRead(LDR);
  byte ldr_value256 = map(ldr_value, 0, 1023, 0, 255);
  Serial.print(ldr_value256);
  Serial.print(",");

  if (ldr_value256 < LIGHT_BELOW) {
    return LIGHT;
  } else if (ldr_value256 > DARKNESS_ABOVE) {
    return DARKNESS;
  } else {
    return INDETERMINATE;
  }
}

MAINS getMains() {
  byte mainsState = digitalRead(MAINS_SENSE);
  Serial.print(mainsState);
  Serial.print(",");

  return mainsState;
}

byte getPotentiometer() {
  int pot_value = analogRead(POT);
  byte pot_value256 = map(pot_value, 0, 1023, 0, 255);
  return pot_value256;
}

ILLUMINATION illuminationStatus = LIGHT;
MAINS mainsStatus = HAS_POWER;
bool shouldLightUp = false;
void loop() {
  checkButtonState();

  ILLUMINATION illuminationStatusNew = getIllumination();
  if (illuminationStatusNew == DARKNESS && illuminationStatus == LIGHT) {
    illuminationStatus = illuminationStatusNew;
    mode = 0;
  } else if (illuminationStatusNew == LIGHT && illuminationStatus == DARKNESS) {
    illuminationStatus = illuminationStatusNew;
    mode = 0;
  }

  mainsStatus = getMains();

  if (mainsStatus) {
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_GREEN, HIGH);
  } else {
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_GREEN, LOW);
  }

  shouldLightUp = illuminationStatus == DARKNESS && mainsStatus == POWER_CUT;
  byte brightness = 255 - getPotentiometer();

  if (shouldLightUp || mode == FORCE_ON) {
    analogWrite(LED_PWM, brightness);
  } else if (!shouldLightUp || mode == FORCE_OFF) {
    analogWrite(LED_PWM, 0);
  }

  Serial.println("");
}
