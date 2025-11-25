
#include <LiquidCrystal.h>

int pin0 = 0;
int pin0digital = 0;
int a = 0;
int pin1 = 0;
int pin1digital = 0;

LiquidCrystal lcd_2(7, 6, 5, 4, 3, 2);

void setup(){
  lcd_2.begin(16, 2);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
}

void loop(){
  
  //digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  //digitalWrite(LED_BUILTIN, LOW);
  //delay(500);

  pin0 = analogRead(A0);
  pin0digital = map(pin0, 0, 1023, 100, 0);

  pin1 = analogRead(A1);
  pin1digital = map(pin1, 0, 1023, 100, 0);

  lcd_2.clear();

  if (analogRead(A2) < 512) {
    lcd_2.print(pin0digital);
  } else {
    lcd_2.print(pin1digital);
  }
}