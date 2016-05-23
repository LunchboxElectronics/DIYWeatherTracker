/*
    LegoWeather - a Particle Photon app that will light up
    a lego scene depending on the current weather. Uses IFTTT for webhooks.

    Teddy Lowe, May 2016
    Lunchbox Electronics
    http://www.lunchboxelectronics.com/
*/

#define DEBUG   0   // 1 is on and 0 is off

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

// These are the global variables
volatile int condition = 0;

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

  if (DEBUG)
    pinMode(D7, OUTPUT);

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
  delay(1000);
  digitalWrite(trees, LOW);
  delay(1000);
  digitalWrite(trees, HIGH);

  if (DEBUG)
    Serial.println("finish setup");
}

void loop()
{
  switch (condition)
  {
    case 1 :      // Rain
      weatherOff();
      digitalWrite(rain1, HIGH);
      digitalWrite(rain2, HIGH);
      digitalWrite(cloud1, HIGH);
      digitalWrite(cloud2, HIGH);
      break;

    case 2 :      // Cloudy
      weatherOff();
      digitalWrite(cloud1, HIGH);
      digitalWrite(cloud2, HIGH);
      break;

    case 3 :      // Clear
      weatherOff();
      digitalWrite(sun1, HIGH);
      digitalWrite(sun2, HIGH);
      break;

    case 4 :      // Snow
      weatherOff();
      digitalWrite(rain1, HIGH);
      digitalWrite(rain2, HIGH);
      digitalWrite(cloud1, HIGH);
      digitalWrite(cloud2, HIGH);
      break;

    default :
      weatherOff();
      digitalWrite(reds, HIGH);
  }

  if (DEBUG)
  {
    Serial.println("in loop");
    digitalWrite(D7, HIGH);
    delay(1000);
    Serial.println("a");
    digitalWrite(D7, LOW);
    delay(1000);
  }
}
