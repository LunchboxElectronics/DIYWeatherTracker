/*
    LegoWeather - a Particle Photon app that will light up
    a lego scene depending on the current weather. Uses IFTTT for webhooks.

    Teddy Lowe, May 2016
    Lunchbox Electronics
    http://www.lunchboxelectronics.com/
*/

#define DEBUG             0   // 1 is on and 0 is off
#define FORCECONDITION    3   // force a certain weather condition, with Debug

// How many boards do you have chained?
#define NUM_TLC5947 1

#define _DAT       D4
#define _CLK       D5
#define _LAT       D6

// These are the pins used for the LEDs on the TLC5947
int _sun[6] = {0,1,2,3,4,5};
int _rain[6] = {6,7,8,9,10,11};
int _clouds[6] = {12,13,14,15,16,17};
int _trees[3] = {18,19,20};
int _reds[3] = {21,22,23};

// This is the max value for PWM flicker
#define MAXPWM      4095

// These are the global variables
volatile int condition = 0;
int min_pwm = 20;
int head = min_pwm;
int pwm_steps = 100;
int dir_head = 1;
int prev_cond = 0;

// The Adafruit library doesn't work with the Photon! These functions emulate it
unsigned int pwmbuffer[2*24*NUM_TLC5947];

boolean driverBegin() {
  if (!pwmbuffer) return false;

  pinMode(_DAT, OUTPUT);
  pinMode(_CLK, OUTPUT);
  pinMode(_LAT, OUTPUT);
  digitalWrite(_LAT, LOW);

  return true;
}

void driverSetPWM(int chan, int pwm){
  if (pwm > 4095) pwm = 4095;
  if (chan > 24*NUM_TLC5947) return;
  pwmbuffer[chan] = pwm;
}

void driverWrite(){
  digitalWrite(_LAT, LOW);
  // 24 channels per TLC5974
  for (int c=24*NUM_TLC5947 - 1; c >= 0 ; c--) {
    // 12 bits per channel, send MSB first
    for (int b=11; b>=0; b--) {
      digitalWrite(_CLK, LOW);

      if (pwmbuffer[c] & (1 << b))
        digitalWrite(_DAT, HIGH);
      else
        digitalWrite(_DAT, LOW);

      digitalWrite(_CLK, HIGH);
    }
  }
  digitalWrite(_CLK, LOW);

  digitalWrite(_LAT, HIGH);
  digitalWrite(_LAT, LOW);
}

// These are the functions for each weather condition, called by an IFTTT recipe
void rain(const char *event, const char *data)
{
  condition = 1;
}

void cloud(const char *event, const char *data)
{
  condition = 2;
}

void clear(const char *event, const char *data)
{
  condition = 3;
}

void snow(const char *event, const char *data)
{
  condition = 4;
}


// These are functions for performing blinks or other lighting fx

void setArray(int array[], int size, int value)
{
  int i;
  for(i = 0; i < size; i++)
  {
    driverSetPWM(array[i], value);
  }
  driverWrite();
}

void weatherOff()
{
  setArray(_sun, 6, 0);
  setArray(_clouds, 6, 0);
  setArray(_rain, 6, 0);
  setArray(_reds, 3, 0);
}


// This is the main block of code

void setup()
{
  if(DEBUG)
    Serial.begin(9600);

  // These lines make the Photon subscribe to the event whose name is in quotes.
  // When something happens with that event, it calls the function whose name is
  // between the first and second commas. The MY_DEVICES flag ensures that your
  // device isn't picking up every public event with the same name as yours
  Particle.subscribe("rain", rain, MY_DEVICES);
  Particle.subscribe("snow", snow, MY_DEVICES);
  Particle.subscribe("clear", clear, MY_DEVICES);
  Particle.subscribe("cloud", cloud, MY_DEVICES);

  // Setup the driver
  driverBegin();

  if (DEBUG){
    pinMode(D7, OUTPUT);
    digitalWrite(D7, HIGH);
  }

  weatherOff();
  setArray(_trees, 3, MAXPWM);
  delay(200);
  setArray(_trees, 3, 0);
  delay(200);
  setArray(_trees, 3, MAXPWM);
  delay(200);
  setArray(_trees, 3, 0);
  delay(200);
  setArray(_trees, 3, MAXPWM);

  if (DEBUG)
    Serial.println("finish setup");
}

void loop()
{
  // The is the head range limiter for the PWM oscillator
  if (head <= min_pwm){    // if less than or equal to minimum
    dir_head = 1;         // Swap direction of the head
    if (head < min_pwm){   // if it is less than the minimum, set to minimum
      head = min_pwm;
    }
  }else if (head >= MAXPWM){  // Same as above in reverse
    dir_head = 0;
    if (head >= MAXPWM){
      head = MAXPWM;
    }
  }

  // This switch statement checks the weather condition and sets LEDs
  switch (condition)
  {
    case 1 :      // Rain
      if (prev_cond != 1){
        weatherOff();
        prev_cond = 1;
      }
      setArray(_clouds, 6, MAXPWM);
      setArray(_rain, 6, head);
      break;

    case 2 :      // Cloudy
      if (prev_cond != 2){
        weatherOff();
        prev_cond = 2;
      }
      min_pwm = 400;
      pwm_steps = 10;
      setArray(_clouds, 6, head);
      break;

    case 3 :      // Clear
      if (prev_cond != 3){
        weatherOff();
        prev_cond = 3;
      }
      setArray(_sun, 6, MAXPWM);
      break;

    case 4 :      // Snow
      if (prev_cond != 4){
        weatherOff();
        prev_cond = 4;
      }
      setArray(_clouds, 6, MAXPWM);
      setArray(_rain, 6, head);
      break;

    default :
      if (prev_cond != 0){
        weatherOff();
        prev_cond = 0;
      }
      setArray(_reds, 3, MAXPWM);
  }

  // This is the actual PWM oscillator
  if (dir_head){
    head = head + pwm_steps;
  }else{
    head = head - pwm_steps;
  }
  delay(100);


  // Debug section
  if (DEBUG)
  {
    condition = FORCECONDITION;
    Serial.println("in loop");
  }
}
