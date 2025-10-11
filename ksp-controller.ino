//use UBS-Type "Custom Game Controller"
//see: https://github.com/ThreePounds/KSP-Custom-Controller for instructions

#include <Bounce2.h>  // debouncing

int BUTTON_PINS[] = { 0, 1, 5, 6, 11, 10, 9, 8, 7, 3, 4, 2 };
const int BUTTON_NUMBER = 12;
const int SWITCH_PIN = 12; // used for switching between spacecraft mode (normal) and aircraft mode (x-asis and z-axis on rotation joystick swapped)
const int JOY1_X_PIN = A4;
const int JOY1_Y_PIN = A3;
const int JOY1_Z_PIN = A5;
const int JOY2_X_PIN = A0;
const int JOY2_Y_PIN = A1;
const int JOY2_Z_PIN = A2;
const int SLIDER_PIN = A7;
const int ANALOG_MAX = 1023;
const int ANALOG_CENTER = 512;

Bounce2::Button buttonBouncers[BUTTON_NUMBER];
Bounce2::Button switchBouncer;

const int DEBOUNCE_MS = 10;

bool planeMode = false;

struct JoystickState {
  int xRaw;
  int yRaw;
  int zRaw;  
  float x;
  float y;
  float z;
};

JoystickState joystick1;
JoystickState joystick2;

int sliderRaw = 0;
float sliderValue = 0;

unsigned long lastUpdateTime = 0;
const int UPDATE_INTERVAL_MS = 5;

void setup() {
  for (int i = 0; i < BUTTON_NUMBER; i++) {
    buttonBouncers[i].attach(BUTTON_PINS[i], INPUT_PULLUP);
    buttonBouncers[i].interval(DEBOUNCE_MS);
  }
  switchBouncer.attach(SWITCH_PIN, INPUT_PULLUP);
  Joystick.useManualSend(true);
}

void loop() {
  switchBouncer.update();
  for (int i = 0; i < BUTTON_NUMBER; i++) {
    buttonBouncers[i].update();
  }

  if (millis() - lastUpdateTime >= UPDATE_INTERVAL_MS) {
    lastUpdateTime = millis();

    planeMode = !switchBouncer.read();

    joystick1.xRaw = ANALOG_MAX - analogRead(JOY1_X_PIN); //potentiometer soldered inverted by accident
    joystick1.yRaw = analogRead(JOY1_Y_PIN);
    joystick1.zRaw = analogRead(JOY1_Z_PIN);
    
    joystick1.x = applySmoothing(applyDeadzone(joystick1.xRaw), joystick1.x); 
    joystick1.y = applySmoothing(applyDeadzone(joystick1.yRaw), joystick1.y);
    joystick1.z = applySmoothing(applyDeadzone(joystick1.zRaw), joystick1.z);

    joystick2.xRaw = ANALOG_MAX - analogRead(JOY2_X_PIN); //potentiometer soldered inverted by accident
    joystick2.yRaw = ANALOG_MAX - analogRead(JOY2_Y_PIN); //potentiometer soldered inverted by accident
    joystick2.zRaw = analogRead(JOY2_Z_PIN);

    joystick2.x = applySmoothing(applyDeadzone(joystick2.xRaw), joystick2.x);
    joystick2.y = applySmoothing(applyDeadzone(joystick2.yRaw), joystick2.y);
    joystick2.z = applySmoothing(applyDeadzone(joystick2.zRaw), joystick2.z);

    //Right Joystick
    if (planeMode) {
      Joystick.X(applyExpoCurve(joystick1.z)); //assigning roll to jaw
      Joystick.Z(applyExpoCurve(joystick1.x)); //assigning jaw to roll
    } else {
      Joystick.X(applyExpoCurve(joystick1.x));
      Joystick.Z(applyExpoCurve(joystick1.z));
    }

    Joystick.Y(applyExpoCurve(joystick1.y));

    //Left Joystick
    Joystick.Xrotate(applyExpoCurve(joystick2.x));
    Joystick.Yrotate(applyExpoCurve(joystick2.y));
    Joystick.Zrotate(applyExpoCurve(joystick2.z));

    //slider
    sliderRaw = ANALOG_MAX - analogRead(SLIDER_PIN); //soldered inverted
    sliderValue = applySmoothing(sliderRaw, sliderValue);
    Joystick.slider(sliderValue);

    for (int i = 0; i < BUTTON_NUMBER; i++) {
      Joystick.button(i + 1, !buttonBouncers[i].read());
    }
    
    Joystick.send_now();
  }
}

int applyDeadzone(int value) {
  const int DEADZONE_RADIUS = 20;

  if (abs(value - ANALOG_CENTER) < DEADZONE_RADIUS) {
    return ANALOG_CENTER;
  }

  if (value < ANALOG_CENTER) {
    return map(value, 0, ANALOG_CENTER - DEADZONE_RADIUS, 0, ANALOG_CENTER); //lower range
  } else {
    return map(value, ANALOG_CENTER + DEADZONE_RADIUS, ANALOG_MAX, ANALOG_CENTER, ANALOG_MAX); //upper range
  }
}

float applySmoothing(int rawValue, float value) {
  const float SMOOTHING_FACTOR = 0.2; //0 = no smoothing, 1.0 = heavy smoothing
  return (rawValue * SMOOTHING_FACTOR) + (value * (1.0 - SMOOTHING_FACTOR));
}

int applyExpoCurve(float value) {
  const float EXPO_INTENSITY = 0.6; //adjust this: 0.0=linear, 1.0=fully cubic
  float normalizedValue = (value - ANALOG_CENTER) / (float)ANALOG_CENTER;
  float curvedValue = normalizedValue * normalizedValue * normalizedValue;
  float blendedValue = (normalizedValue * (1.0 - EXPO_INTENSITY)) + (curvedValue * EXPO_INTENSITY);
  int finalValue = (blendedValue * ANALOG_CENTER) + ANALOG_CENTER;
  return constrain(finalValue, 0, ANALOG_MAX);
}