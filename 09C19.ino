// Arduino pin assignment
#define PIN_LED  9
#define PIN_TRIG 12
#define PIN_ECHO 13

// configurable parameters
#define SND_VEL 346.0     // sound velocity at 24 celsius degree (unit: m/sec)
#define INTERVAL 100       // sampling interval (unit: msec)
#define PULSE_DURATION 10 // ultra-sound Pulse Duration (unit: usec)
#define _DIST_MIN 100     // minimum distance to be measured (unit: mm)
#define _DIST_MAX 300     // maximum distance to be measured (unit: mm)

#define TIMEOUT ((INTERVAL / 2) * 1000.0) // maximum echo waiting time (unit: usec)
#define SCALE (0.001 * 0.5 * SND_VEL)     // coefficent to convert duration to distance

#define _EMA_ALPHA 0.8    // EMA weight of new sample (range: 0 to 1)
                          // Setting EMA to 1 effectively disables EMA filter.
#define N 20

// global variables
unsigned long last_sampling_time;   // unit: msec
float dist_ema= 0;                     // EMA distance
float dist_median = 0;
float values[N];
int count = 0;

void setup() {
  // initialize GPIO pins
  pinMode(PIN_LED,OUTPUT);
  pinMode(PIN_TRIG,OUTPUT);
  pinMode(PIN_ECHO,INPUT);
  digitalWrite(PIN_TRIG, LOW);

  // initialize serial port
  Serial.begin(57600);
}

void loop() {
  float dist_raw;
  
  // wait until next sampling time. 
  // millis() returns the number of milliseconds since the program started. 
  // will overflow after 50 days.
  if (millis() < last_sampling_time + INTERVAL)
    return;

  // get a distance reading from the USS
  dist_raw = USS_measure(PIN_TRIG,PIN_ECHO);

  // Modify the below line to implement the EMA equation
  dist_ema = _EMA_ALPHA*dist_median + (1-_EMA_ALPHA)*dist_ema;

  Queue(N, values, dist_raw);
  if (count < N) {
    count++;
  }
  else {
    dist_median = Median(N, values);
  }

  // output the distance to the serial port
  Serial.print("Min:");   Serial.print(_DIST_MIN);
  Serial.print(",raw:"); Serial.print(min(dist_raw, _DIST_MAX + 100));
  Serial.print(",median:");  Serial.print(min(dist_median, _DIST_MAX + 100));
  Serial.print(",ema:");  Serial.print(min(dist_ema, _DIST_MAX + 100));
  Serial.print(",Max:");  Serial.print(_DIST_MAX);
  Serial.println("");

  // do something here
  if ((dist_raw < _DIST_MIN) || (dist_raw > _DIST_MAX))
    digitalWrite(PIN_LED, 1);       // LED OFF
  else
    digitalWrite(PIN_LED, 0);       // LED ON

  // update last sampling time
  last_sampling_time += INTERVAL;
}

// get a distance reading from USS. return value is in millimeter.
float USS_measure(int TRIG, int ECHO)
{
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(PULSE_DURATION);
  digitalWrite(TRIG, LOW);
  
  return pulseIn(ECHO, HIGH, TIMEOUT) * SCALE; // unit: mm
}

void Queue(int num, float values[], float recent_value){
  
  for(int i=0; i < num-1; i++){
    values[i] = values[i+1];
  }
  values[num-1] = recent_value;
}

float Median(int num, float values[])
{
  float temp[num];
  for (int i = 0; i < num; i++) {
    temp[i] = values[i];
  }
  for (int i = 0; i < num - 1; i++) {
    for (int j = i + 1; j < num; j++) {

      if (temp[i] > temp[j]) {
        float tmp = temp[i];
        temp[i] = temp[j];
        temp[j] = tmp;
      }
    }
  }
  if (N % 2 == 0)    return (temp[N/2] + temp[N/2-1])/2;
  else  return temp[N/2];
}
