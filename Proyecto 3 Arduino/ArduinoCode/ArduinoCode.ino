//#include <Adafruit_LiquidCrystal.h>

// Adafruit_LiquidCrystal lcd_2(0);

int pin0 = 0;
int pin0digital = 0;
int showVolume = 0;
int a = 0;
int pin1 = 0;
int pin1digital = 0;

void setup()
{
  // lcd_2.begin(16, 2);
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(A2, INPUT);
  pinMode(3, OUTPUT);
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
}

void loop()
{
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);

  showVolume = analogRead(A2);
  analogWrite(3, showVolume); // si quieres: map(showVolume, 0, 1023, 0, 255)

  pin0 = analogRead(A0);
  pin0digital = map(pin0, 0, 1023, 100, 0);
  pin1 = analogRead(A1);
  pin1digital = map(pin1, 0, 1023, 100, 0);

  if (analogRead(A2) < 512) {
    Serial.print("pin0digital: ");
    Serial.println(pin0digital);
  } else {
    Serial.print("pin1digital: ");
    Serial.println(pin1digital);
  }
}