/*
    DIYWeatherTracker - a Particle Photon app that will light up
    a lego scene depending on the current weather. Uses IFTTT for webhooks.
    Uses Adafruit TLC5947 display driver.

    CC BY-SA Teddy Lowe, May 2016
    Lunchbox Electronics
    http://www.lunchboxelectronics.com/
*/

#define DEBUG             0   // 1 is on and 0 is off
#define FORCECONDITION    10   // force a certain weather condition, with Debug

// How many boards do you have chained?
#define NUM_TLC5947 1

#define _DAT       D4   // The pin plugged into the DIN line
#define _CLK       D5   // The pin plugged into the CLK line
#define _LAT       D6   // The pin plugged into the LAT line

//
// These are the pins used for the LEDs on the TLC5947
//

int _sun[6] = {0,1,2,3,4,5};
int _rain[6] = {6,7,8,9,10,11};
int _clouds[4] = {12,13,14,15};
int _snow[2] = {16,17};
int _trees[3] = {18,19,20};
int _reds[3] = {21,22,23};

//
// This is the max value for PWM flicker
//

#define MAXPWM      4095

//
// This is the time between API calls, in milliseconds
//

#define REFRESHTIME   3600000

//
// These are the global variables
//

int condition;   // This stores data about the current condition
int min_pwm = 20;             // This describes how dim the LEDs will get
volatile int head = min_pwm;  // This changes over time, to provide some pulsing light
int pwm_steps = 100;          // This describes how fast the light will pulse
int dir_head = 1;             // This describes whether the light is getting dimmer or brighter
int prev_cond = 0;            // This is used to store the condition during the last loop

unsigned long currentMillis = 0;  // These are used for time between API calls
unsigned long prevMillis = 0;

// These will store the size variables for the arrays
int sizeof_sun, sizeof_rain, sizeof_clouds, sizeof_snow, sizeof_trees, sizeof_reds;

//
// The Adafruit library doesn't work with the Photon! These functions emulate it
//

unsigned int pwmbuffer[24*NUM_TLC5947]; // This array stores the value each channel should be at

// This function sets up the output pins and begins communication with the chip
boolean driverBegin() {
  if (!pwmbuffer) return false;

  pinMode(_DAT, OUTPUT);
  pinMode(_CLK, OUTPUT);
  pinMode(_LAT, OUTPUT);
  digitalWrite(_LAT, LOW);

  return true;
}

// This checks to see if the PWM is in range and then writes the channel to the buffer
void driverSetPWM(int chan, int pwm){
  if (pwm > 4095) pwm = 4095;
  if (pwm < 0) pwm = 0;
  if (chan > 24*NUM_TLC5947) return;
  pwmbuffer[chan] = pwm;
}

// This perfoms the SPI communication to write the entire channel buffer to the LED driver chip
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

//
// These are functions for performing blinks or other lighting fx
//

// This is the quick function for setting an entire array to the same value
void setArray(int array[], int size, int value)
{
  int i;
  for(i = 0; i < size; i++)
  {
    driverSetPWM(array[i], value);
  }
  driverWrite();
}

// This function sets PWM values at a gradient defined by spread. 0 in spread will cause
void flowThru(int array[], int size, int spread)
{
  int i;                          // setup loop variable
  int prev_dir = dir_head;        // Setup previous direction without changing
                                  // direction variable

  if (!spread) spread = MAXPWM - min_pwm;   // If default spread, calculate
  int offset = spread/size;                 // Offset is based on spread and number of channels
  int off[size];                  // create holding array for offsets

  off[0] = head;                  // first channel based directly on head

  for (i = 1; i < size; i++){
    if (prev_dir){                // if the prev_dir indicates moving up
      off[i] = off[i-1] - offset; // off is equal to previous MINUS offset
      if (off[i] < min_pwm){      // if that puts us below the minimum allowed
        off[i] = offset - off[i-1]; //then it actually is the opposite
        prev_dir = !prev_dir;     // and now change the direction
      }
    }else if (!prev_dir){         // This is the exact same code as above, in reverse
      off[i] = off[i-1] + offset; // Don't be afraid of that big line below. It's not too bad!
      if (off[i] > MAXPWM){
        off[i] = MAXPWM - (offset - (MAXPWM - off[i-1]));
        prev_dir = !prev_dir;
      }
    }
  }

  // Now write all of the values to the driver
  for(i = 0; i < size; i++)
  {
    driverSetPWM(array[i], off[i]);
  }
  driverWrite();
}

// This is a quick function to set all weather elements plus the red LEDs off
void weatherOff()
{
  setArray(_sun, sizeof_sun, 0);
  setArray(_clouds, sizeof_clouds, 0);
  setArray(_rain, sizeof_rain, 0);
  setArray(_snow, sizeof_snow, 0);
  setArray(_reds, sizeof_reds, 0);
}

//
// This is called when the webhook response is received, it will set condition
//

void parseWeather(const char *event, const char *value){
  if (DEBUG) Serial.print(value);

  // Set the current condition to the value from the API call
  // clear-day, clear-night, rain, snow, sleet, wind, fog, cloudy, partly-cloudy-day, or partly-cloudy-night
  if (*value == 'c'){
    value = value+2;
    if (*value == 'e'){
      value = value + 4;
      if (*value == 'd'){
        condition = 1;  // clear-day
        return;
      }else if (*value == 'n'){
        condition = 2;  // clear-night
        return;
      }
    }else if (*value == 'o'){
      condition = 8;  // cloudy
      return;
    }
  }else if (*value == 'r'){
    condition = 3;  // rain
    return;
  }else if (*value == 's'){
    value = value + 1;
    if (*value == 'n'){
      condition = 4;  // snow
      return;
    }else if (*value == 'l'){
      condition = 5;  // sleet
      return;
    }
  }else if (*value == 'w'){
    condition = 6;  // wind
    return;
  }else if (*value == 'f'){
    condition = 7;  // fog
    return;
  }else if (*value == 'p'){
    value = value + 14;
    if (*value == 'd'){
      condition = 9;  // partly-cloudy-day
      return;
    }else if (*value == 'n'){
      condition = 10;  // partly-cloudy-night
      return;
    }
  }
  else condition = 0;
  Serial.println(condition);
}

//
// This is the main block of code
//

void setup()
{
  if(DEBUG) Serial.begin(9600);

  // These lines make the Photon subscribe to the event whose name is in quotes.
  // When something happens with that event, it calls the function whose name is
  // between the first and second commas. The MY_DEVICES flag ensures that your
  // device isn't picking up every public event with the same name as yours
  Particle.subscribe("hook-response/getWeather", parseWeather, MY_DEVICES);

  // Setup the driver
  driverBegin();

  if (DEBUG){
    pinMode(D7, OUTPUT);
    digitalWrite(D7, HIGH);
  }

  // This calculates the sizeof variables
  sizeof_sun = sizeof(_sun)/4;
  sizeof_rain = sizeof(_rain)/4;
  sizeof_clouds = sizeof(_clouds)/4;
  sizeof_snow = sizeof(_snow)/4;
  sizeof_trees = sizeof(_trees)/4;
  sizeof_reds = sizeof(_reds)/4;

  // Make the initial API call
  Particle.publish("getWeather", PRIVATE);
  if (DEBUG)  Serial.println("Initial API publish...");

  // Flash the trees 3 times to indicate finished setup
  weatherOff();
  setArray(_trees, sizeof_trees, MAXPWM);
  delay(200);
  setArray(_trees, sizeof_trees, 0);
  delay(200);
  setArray(_trees, sizeof_trees, MAXPWM);
  delay(200);
  setArray(_trees, sizeof_trees, 0);
  delay(200);
  setArray(_trees, sizeof_trees, MAXPWM);

  if (DEBUG) Serial.println("finish setup");
}

void loop()
{
  // The is the head range limiter for the PWM oscillator
  if (head <= min_pwm){    // if less than or equal to minimum
    dir_head = 1;          // Swap direction of the head
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
    case 1 :  // clear-day
      // This if statement is used to reduce flicker, may or may not be necessary in your case.
      // It simply turns off all weather elements ONLY the first time it hits this loop
      if (prev_cond != 1){
        weatherOff();
        prev_cond = 1;
      }
      // Use the next lines to tune what you want your effects to look like!
      setArray(_sun, sizeof_sun, MAXPWM);   // This is the type of effect
      break;

    case 2 :  //clear-night
      // This if statement is used to reduce flicker, may or may not be necessary in your case.
      // It simply turns off all weather elements ONLY the first time it hits this loop
      if (prev_cond != 2){
        weatherOff();
        prev_cond = 2;
      }
      // Use the next 4 lines to tune what you want your effects to look like!
      pwm_steps = 30;
      flowThru(_sun, sizeof_sun, 0);   // This is the type of effect
      break;

    case 3 :      // Rain
      // This if statement is used to reduce flicker, may or may not be necessary in your case.
      // It simply turns off all weather elements ONLY the first time it hits this loop
      if (prev_cond != 3){
        weatherOff();
        prev_cond = 3;
      }
      // Use the next 4 lines to tune what you want your effects to look like!
      min_pwm = 0;
      pwm_steps = 100;
      setArray(_clouds, sizeof_clouds, MAXPWM);
      flowThru(_rain, sizeof_rain, 0);
      break;

    case 4 :  // snow
      // This if statement is used to reduce flicker, may or may not be necessary in your case.
      // It simply turns off all weather elements ONLY the first time it hits this loop
      if (prev_cond != 4){
        weatherOff();
        prev_cond = 4;
      }
      min_pwm = 0;
      pwm_steps = 10;
      setArray(_clouds, sizeof_clouds, MAXPWM);
      flowThru(_snow, sizeof_snow, 1000);
      //flowThru(_rain, sizeof_rain, 6000);
      break;

    case 5 :  // sleet
      // This if statement is used to reduce flicker, may or may not be necessary in your case.
      // It simply turns off all weather elements ONLY the first time it hits this loop
      if (prev_cond != 5){
        weatherOff();
        prev_cond = 5;
      }
      // Use the next 4 lines to tune what you want your effects to look like!
      min_pwm = 0;
      pwm_steps = 10;
      setArray(_clouds, sizeof_clouds, MAXPWM);
      flowThru(_snow, sizeof_snow, 1000);
      flowThru(_rain, sizeof_rain, 6000);
      break;

    case 6 :  // wind
      // This if statement is used to reduce flicker, may or may not be necessary in your case.
      // It simply turns off all weather elements ONLY the first time it hits this loop
      if (prev_cond != 6){
        weatherOff();
        prev_cond = 6;
      }
      // Use the next 4 lines to tune what you want your effects to look like!
      min_pwm = 0;
      pwm_steps = 60;
      flowThru(_clouds, sizeof_clouds, 0);   // This is the type of effect
      break;

    case 7 :  // fog
      // This if statement is used to reduce flicker, may or may not be necessary in your case.
      // It simply turns off all weather elements ONLY the first time it hits this loop
      if (prev_cond != 7){
        weatherOff();
        prev_cond = 7;
      }
      // Use the next lines to tune what you want your effects to look like!
      min_pwm = 400;
      pwm_steps = 1;
      setArray(_clouds, sizeof_clouds, head);
      break;

    case 8 :      // cloudy
      // This if statement is used to reduce flicker, may or may not be necessary in your case.
      // It simply turns off all weather elements ONLY the first time it hits this loop
      if (prev_cond != 8){
        weatherOff();
        prev_cond = 8;
      }
      // Use the next lines to tune what you want your effects to look like!
      min_pwm = 400;
      pwm_steps = 1;
      setArray(_clouds, sizeof_clouds, head);
      break;

    case 9 :  // partly-cloudy-day
      // This if statement is used to reduce flicker, may or may not be necessary in your case.
      // It simply turns off all weather elements ONLY the first time it hits this loop
      if (prev_cond != 9){
        weatherOff();
        prev_cond = 9;
      }
      // Use the next lines to tune what you want your effects to look like!
      min_pwm = 400;
      pwm_steps = 10;
      flowThru(_clouds, sizeof_clouds, 0);
      setArray(_sun, sizeof_sun, MAXPWM);
      break;

    case 10 :  // partly-cloudy-night
      // This if statement is used to reduce flicker, may or may not be necessary in your case.
      // It simply turns off all weather elements ONLY the first time it hits this loop
      if (prev_cond != 10){
        weatherOff();
        prev_cond = 10;
      }
      // Use the next lines to tune what you want your effects to look like!
      min_pwm = 400;
      pwm_steps = 10;
      flowThru(_clouds, sizeof_clouds, 0);
      flowThru(_sun, sizeof_sun, 0);
      break;

    default :     // None of the above, throw error
      // This if statement is used to reduce flicker, may or may not be necessary in your case.
      // It simply turns off all weather elements ONLY the first time it hits this loop
      if (prev_cond != 0){
        weatherOff();
        prev_cond = 0;
      }
      min_pwm = 0;
      pwm_steps = 20;
      setArray(_clouds, sizeof_clouds, MAXPWM);
      flowThru(_reds, sizeof_reds, 0);
  }

  // This is the actual PWM oscillator
  if (dir_head){
    head = head + pwm_steps;
  }else{
    head = head - pwm_steps;
  }

  // This counts to REFRESHTIME and then makes another API call
  currentMillis = millis();
  if (currentMillis - prevMillis > REFRESHTIME){
    if (DEBUG) Serial.println("Sending publish...");

    prevMillis = currentMillis;
    Particle.publish("getWeather", PRIVATE);
  }

  // Debug section
  if (DEBUG)
  {
    if (FORCECONDITION != -1)
      condition = FORCECONDITION;
    //Serial.println("in loop");
  }
}
