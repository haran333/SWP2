#include <Servo.h>

#define PIN_SERVO 10
#define PIN_TRIG 12
#define PIN_ECHO 13

// configurable parameters
#define SND_VEL 346.0
#define INTERVAL 100
#define PULSE_DURATION 10
#define _DIST_MOVING 100.0

#define SCALE (0.001 * 0.5 * SND_VEL)
#define TIMEOUT 25000
#define MODE 1// 0. sigmoid, 1. smoothstep
#define SIGMOID_K 10.0 //sigmoid 기울기



Servo myServo;
unsigned long MOVING_TIME = 1000;
unsigned long moveStartTime;
unsigned long lastSampleTime = 0;
unsigned long lastDetectTime = 0;

float currentAngle = 0;
float startAngle = 0;
float stopAngle  = 0;

float USS_measure(int TRIG, int ECHO)
{
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(PULSE_DURATION);
  digitalWrite(TRIG, LOW);
  
  return pulseIn(ECHO, HIGH, TIMEOUT) * SCALE; // unit: mm
}

float sigmoid(float t) {
  float s0 = 1.0 / (1.0 + exp(SIGMOID_K * 0.5));    // s(0)
  float s1 = 1.0 / (1.0 + exp(-SIGMOID_K * 0.5));   // s(1)
  float s  = 1.0 / (1.0 + exp(-SIGMOID_K * (t - 0.5)));
  return (s - s0) / (s1 - s0);
}

float smoothstep(float t) {
  return t * t * (3.0 - 2.0 * t);
}

float dist_p(float t) {
  if (t <= 0.0) return 0.0;
  if (t >= 1.0) return 1.0;
  
  if (MODE == 0) return sigmoid(t);
  else return smoothstep(t);
}

void startMoving(float target) {
  if (target == stopAngle) return;

  startAngle = currentAngle;
  stopAngle  = target;
  moveStartTime = millis();
}

void moving() {
  unsigned long progress = millis() - moveStartTime;

  if (progress <= MOVING_TIME) {
    // while moving
    float t = (float)progress / MOVING_TIME;
    currentAngle = startAngle + (stopAngle - startAngle) * dist_p(t);
  } else {
    // movement finished
    currentAngle = stopAngle;
  }
  myServo.write((int)(currentAngle + 0.5));
}

void setup() {
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  digitalWrite(PIN_TRIG, LOW);
  
  myServo.attach(PIN_SERVO);
  moveStartTime = millis(); // start moving

  myServo.write(startAngle); // Set position
  delay(500);

  Serial.begin(57600);
}

void loop() {
  if (millis() - lastSampleTime >= INTERVAL) {
    lastSampleTime = millis();

    float distance = USS_measure(PIN_TRIG, PIN_ECHO); // read distance

    if (distance > 0 && distance <= _DIST_MOVING) {
      lastDetectTime = millis();
      startMoving(90);
    } else if (millis() - lastDetectTime >= 2000) {
      startMoving(0);
    }

    Serial.print("dist:");
    Serial.print(distance);
    Serial.print(",angle:");
    Serial.println(currentAngle);
  }

  moving();
}
