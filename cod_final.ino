#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define SENSOR_INTERIOR A0
#define SENSOR_EXTERIOR A1
#define SERVO_PIN 9
#define BUTTON_PIN 2
#define LED_PWM_PIN 5
#define LUM_MIN 600 

Servo blindServo;
LiquidCrystal_I2C lcd(0x27, 16, 2);

bool modManual = false;
bool modAnterior = false;
bool jaluzeleDeschise = false;
unsigned long buttonPressTime = 0;
bool buttonPrevState = LOW;
int unghiCurent = 0;

unsigned long lastToggleTime = 0;
bool afisamInterior = true;

void setup() {
  pinMode(BUTTON_PIN, INPUT);  
  pinMode(LED_PWM_PIN, OUTPUT);

  blindServo.attach(SERVO_PIN);
  lcd.begin(16, 2);
  lcd.backlight();
  lcd.clear();
}

void loop() {
  bool buttonState = digitalRead(BUTTON_PIN);

  if (buttonPrevState == LOW && buttonState == HIGH) {
    buttonPressTime = millis();
  }

  if (buttonPrevState == HIGH && buttonState == LOW) {
    unsigned long durataApasare = millis() - buttonPressTime;

    if (!modManual) {
      modManual = true;
      jaluzeleDeschise = false;
      unghiCurent = 0;
      blindServo.write(unghiCurent);
      analogWrite(LED_PWM_PIN, 255); 
    } else {
      if (durataApasare >= 3000) {
        modManual = false;
        analogWrite(LED_PWM_PIN, 0);  
        lastToggleTime = 0;
      } else {
        jaluzeleDeschise = !jaluzeleDeschise;
        unghiCurent = jaluzeleDeschise ? 90 : 0;
        blindServo.write(unghiCurent);
        analogWrite(LED_PWM_PIN, 255);
      }
    }
  }

  buttonPrevState = buttonState;

  if (modManual != modAnterior) {
    lcd.clear();
    modAnterior = modManual;
  }

  if (modManual) {
    lcd.setCursor(0, 0);
    lcd.print("Jaluzele        ");
    lcd.setCursor(0, 1);
    lcd.print(jaluzeleDeschise ? "deschise        " : "inchise         ");
  } else {

    int luminaExterior = analogRead(SENSOR_EXTERIOR);
    int luminaInterior = analogRead(SENSOR_INTERIOR);

    if (luminaExterior > 200) {
      analogWrite(LED_PWM_PIN, 0);

      int unghi = map(luminaInterior, 0, 1023, 90, 10); 
      unghiCurent = constrain(unghi, 10, 90);
      blindServo.write(unghiCurent);
    } else {
      unghiCurent = 0;
      blindServo.write(unghiCurent);

      int eroare = LUM_MIN - luminaInterior;
      int pwmVal = map(eroare, 0, LUM_MIN, 0, 255);
      pwmVal = constrain(pwmVal, 0, 255);
      analogWrite(LED_PWM_PIN, pwmVal);
    }

    if (millis() - lastToggleTime > 3000) {
      afisamInterior = !afisamInterior;
      lastToggleTime = millis();
      lcd.setCursor(0, 0);
      lcd.print("                ");
      lcd.setCursor(0, 0);
      if (afisamInterior) {
        lcd.print("L.Int: ");
        lcd.print(luminaInterior);
      } else {
        lcd.print("L.Ext: ");
        lcd.print(luminaExterior);
      }
    }

    lcd.setCursor(0, 1);
    lcd.print("Unghi: ");
    lcd.print(unghiCurent);
    lcd.print((char)223); 
    lcd.print(" ");

    lcd.setCursor(11, 1);
    if (unghiCurent <= 5) {
      lcd.print("Inc   ");
    } else if (unghiCurent >= 85) {
      lcd.print("Desch ");
    } else {
      lcd.print("      ");
    }
  }

  delay(100);
}
