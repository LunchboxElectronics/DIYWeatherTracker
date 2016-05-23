/*
    LegoWeather - a Particle Photon app that will light up
    a lego scene depending on the current weather

    Teddy Lowe, May 2016
    Lunchbox Electronics
*/

void weatherHandler(const char *event, const char *data);

int _timer = 0;

void setup()
{
  Serial.begin(9600);
  Particle.subscribe("weather", weatherHandler, MY_DEVICES);
  
  pinMode(D7, OUTPUT);
  pinMode(D0, OUTPUT);
  pinMode(D1, OUTPUT);

  digitalWrite(D0, HIGH);
  digitalWrite(D1, HIGH);
  delay(1000);
  digitalWrite(D0, LOW);
  digitalWrite(D1, LOW);
  Serial.println("finish setup");
}

void loop()
{
  Serial.println("in loop");
  digitalWrite(D7, HIGH);
  delay(1000);
  Serial.println("a");
  digitalWrite(D7, LOW);
  delay(1000);
}

void weatherHandler(const char *event, const char *data)
{
  Serial.println("in weatherHandler");
  if (data)
  {
    Serial.println("data received");
    Serial.println(data);
    digitalWrite(D1, HIGH);
  }
  //delay(10000);
  //digitalWrite(D1, LOW);
}
