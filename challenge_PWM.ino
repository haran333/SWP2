const int led = 7;

int pwm_period = 10000;
int pwm_duty   = 50;

void set_period(int period) {
  pwm_period = constrain(period, 100, 10000);
}

void set_duty(int duty) {
  pwm_duty = constrain(duty, 0, 100);
}

void pwm_output() {
  int on_time = (long)pwm_period * pwm_duty / 100;
  int off_time = pwm_period - on_time;

  if (on_time > 0) {
    digitalWrite(led, LOW);
    delayMicroseconds(on_time);
  }
  if (off_time > 0) {
    digitalWrite(led, HIGH);
    delayMicroseconds(off_time);
  }
}

void setup() {
  pinMode(led, OUTPUT);
  set_period(2000);
  set_duty(30);
}

void loop() {
  unsigned long t = millis() % (1000UL);

  int duty;
  if (t < 500){
    duty = t * 100UL / 500;
  }
  else{
    duty = (1000UL - t) * 100UL / 500;
  }

  set_duty(duty);
  pwm_output();
}
