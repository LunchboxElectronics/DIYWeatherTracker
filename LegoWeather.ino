/*
    LegoWeather - a Particle Photon app that will light up
    a lego scene depending on the current weather. Uses IFTTT for webhooks.

    Teddy Lowe, May 2016
    Lunchbox Electronics
    http://www.lunchboxelectronics.com/
*/

#define DEBUG             0   // 1 is on and 0 is off
#define FORCECONDITION    4   // force a certain weather condition

// These are the pins used for the LEDs, keep in mind problems with the number
// of PWM outputs on the Photon! Check this for more info:
// https://docs.particle.io/datasheets/photon-datasheet/#peripherals-and-gpio
#define rain1   D0
#define rain2   D1
#define cloud1  D2
#define cloud2  D3
#define trees   D4
#define sun1    TX
#define sun2    RX
#define reds    D5

// This is the max value for PWM flicker
#define MAXPWM  255

// These are the global variables
volatile int condition = 0;
int min_pwm = 20;
int head = min_pwm;
int dir_head = 1;
int toggler = 0;
int delaytime = 10;


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

void weatherOff()
{
  digitalWrite(reds, LOW);
  digitalWrite(cloud1, LOW);
  digitalWrite(cloud2, LOW);
  digitalWrite(rain1, LOW);
  digitalWrite(rain2, LOW);
  digitalWrite(sun1, LOW);
  digitalWrite(sun2, LOW);
}

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

  if (DEBUG){
    pinMode(D7, OUTPUT);
    digitalWrite(D7, HIGH);
  }

  // These lines set up our LED output pins
  pinMode(rain1, OUTPUT);
  pinMode(rain2, OUTPUT);
  pinMode(sun1, OUTPUT);
  pinMode(sun2, OUTPUT);
  pinMode(cloud1, OUTPUT);
  pinMode(cloud2, OUTPUT);
  pinMode(trees, OUTPUT);
  pinMode(reds, OUTPUT);

  digitalWrite(trees, HIGH);
  delay(500);
  digitalWrite(trees, LOW);
  delay(500);
  digitalWrite(trees, HIGH);

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
    toggler = !toggler;       // Toggler used for changing LED flickers
    if (head > MAXPWM){
      head = MAXPWM;
    }
  }

  // This switch statement checks the weather condition and sets LEDs
  switch (condition)
  {
    case 1 :      // Rain
      weatherOff();
      min_pwm = 0;
      delaytime = 0;
      if (toggler){
        analogWrite(rain1, head);
        digitalWrite(rain2, HIGH);
      }else{
        analogWrite(rain2, head);
        digitalWrite(rain1, HIGH);
      }
      digitalWrite(cloud1, HIGH);
      digitalWrite(cloud2, HIGH);
      break;

    case 2 :      // Cloudy
      weatherOff();
      min_pwm = 20;
      delaytime = 10;
      if (toggler){
        analogWrite(cloud1, head);
        digitalWrite(cloud2, HIGH);
      }else{
        analogWrite(cloud2, head);
        digitalWrite(cloud1, HIGH);
      }
      break;

    case 3 :      // Clear
      weatherOff();
      min_pwm = 20;
      digitalWrite(sun1, HIGH);
      digitalWrite(sun2, HIGH);
      break;

    case 4 :      // Snow
      weatherOff();
      min_pwm = 0;
      analogWrite(rain1, head);
      analogWrite(rain2, head);
      digitalWrite(cloud1, HIGH);
      digitalWrite(cloud2, HIGH);
      break;

    default :
      weatherOff();
      digitalWrite(reds, HIGH);
  }

  // This is the actual PWM oscillator
  if (dir_head){
    head = head + 1;
  }else{
    head = head - 1;
  }
  delay(delaytime);



  // Debug section
  if (DEBUG)
  {
    condition = FORCECONDITION;
    Serial.println("in loop");
  }
}
