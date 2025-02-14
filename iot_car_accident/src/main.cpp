#include <Arduino.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
#include <MPU6050.h>
#include <OneWire.h>
#include <SoftwareSerial.h>
#include <TinyGPSPlus.h>
#include <Wire.h>
#include <avr/wdt.h>

#define NUMBER "01719706234"

#define GPS_RX 11
#define GPS_TX 12
#define SIM800L_RX 3
#define SIM800L_TX 2
#define DS18B20_PIN A0
#define VIBRATOR_SENSOR_PIN A1
#define BUZZER_PIN 10

SoftwareSerial gpsSerial(GPS_RX, GPS_TX);
SoftwareSerial gsm(SIM800L_RX, SIM800L_TX);

LiquidCrystal_I2C lcd(0x27, 16, 2);
OneWire oneWire(DS18B20_PIN);
DallasTemperature ds18b20(&oneWire);
TinyGPSPlus gps;
MPU6050 mpu;

void checkGPS();
void monitoring();
void showLCD();
String intStr(int);
void detectAccident();
void GSMinit();
void activateGPRS();
void sendSMS();
void updateServer();
void clearLcd();

bool smsFlag;
bool isFlip, accidentDetected;
int sec, vibration;
int16_t ax, ay, az;
int16_t gx, gy, gz;
double lati, longi;
float temperature;
long pMs;

void setup() {
  wdt_disable();
  Serial.begin(9600);
  gsm.begin(9600);
  gpsSerial.begin(9600);

  lcd.init();
  lcd.backlight();

  ds18b20.begin();
  Wire.begin();
  mpu.initialize();

  pinMode(VIBRATOR_SENSOR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, 0);

  GSMinit();
  activateGPRS();
}

void loop() {
  checkGPS();
  long cMs = millis();
  int vibrator = analogRead(VIBRATOR_SENSOR_PIN);
  if (vibrator < 512) vibration++;

  if (cMs - pMs > 1000) {
    ds18b20.requestTemperatures();
    temperature = ds18b20.getTempCByIndex(0);
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    isFlip = az < 0;
    detectAccident();

    monitoring();
    showLCD();

    pMs = cMs;
    vibration = 0;
  }
}

void detectAccident() {
  if (isFlip)
    sec++;
  else if (vibration > 3000)
    sec++;
  else if (temperature > 35)
    sec++;

  if (sec > 3) {
    sec = 0;
    accidentDetected = 1;
    digitalWrite(BUZZER_PIN, 1);
    clearLcd();
    showLCD();
    updateServer();
    updateServer();
    sendSMS();
    clearLcd();
  }

  if (accidentDetected && !isFlip && vibration < 1000 && temperature < 32) {
    sec = 0;
    accidentDetected = 0;
    digitalWrite(BUZZER_PIN, 0);
    smsFlag = 0;
    clearLcd();
    showLCD();
    updateServer();
    updateServer();
  }
}

void checkGPS() {
  gpsSerial.listen();
  while (gpsSerial.available() > 0) {
    if (gps.encode(gpsSerial.read())) {
      if (gps.location.isValid()) {
        lati = gps.location.lat();
        longi = gps.location.lng();
      }
    }
  }
}

void showLCD() {
  if (accidentDetected) {
    lcd.setCursor(0, 0);
    lcd.print("ACCIDENT DETECT!");
    lcd.setCursor(0, 1);
    lcd.print("BY: ");
    if (isFlip)
      lcd.print("GYRO");
    else if (vibration > 3000)
      lcd.print("HIT ");
    else if (temperature > 35)
      lcd.print("FIRE");
    return;
  }

  lcd.setCursor(0, 0);
  lcd.print(gps.location.isValid() ? "-GPS LOCK-" : "--NO GPS--");
  lcd.setCursor(12, 0);
  lcd.print(temperature, 0);
  lcd.print((char)223);
  lcd.print("C");
  lcd.setCursor(0, 1);
  lcd.print("V:" + intStr(vibration) + " FLIP: ");
  lcd.print(isFlip ? "YES" : "NO ");
}

String intStr(int value) {
  if (value < 10)
    return "000" + (String)value;
  else if (value < 100)
    return "00" + (String)value;
  else if (value < 1000)
    return "0" + (String)value;
  else
    return (String)value;
}

void monitoring() {
  Serial.print(ax);
  Serial.print("\t|\t");
  Serial.print(ay);
  Serial.print("\t|\t");
  Serial.print(az);
  Serial.print("\t|\t");
  Serial.print(temperature);
  Serial.print("\t|\t");
  Serial.print(vibration);
  Serial.print("\t|\t");
  Serial.print(lati);
  Serial.print("\t|\t");
  Serial.print(longi);
  Serial.println();
}

void GSMinit() {
  clearLcd();
  lcd.print("GSM Powering UP");
  for (byte i = 0; i < 15; i++) {
    lcd.setCursor(i, 1);
    lcd.print("-");
    delay(1000);
  }

  gsm.print("AT\r\n");
  delay(1000);
  gsm.print("ATE0\r\n");
  delay(1000);
  gsm.print("AT+CMGF=1\r\n");
  delay(1000);
  gsm.print("AT+CNMI=1,2,0,0,0\r\n");
  delay(1000);
  clearLcd();
}

void activateGPRS() {
  clearLcd();
  lcd.print("GPRS Init...");

  gsm.println("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"");
  delay(1000);
  gsm.println("AT+SAPBR=3,1,\"APN\",\"ttinternet\"");
  delay(1000);
  gsm.println("AT+CGATT=1");
  delay(1000);
  gsm.println("AT+SAPBR=1,1");
  delay(6000);
  gsm.println("AT+HTTPINIT");
  delay(1000);

  clearLcd();
}

void sendSMS() {
  if (smsFlag) return;
  gsm.print("AT+CMGF=1\r\n");
  delay(200);
  gsm.print((String) "AT+CMGS=\"" + NUMBER + "\"\r\n");
  delay(200);
  gsm.print("CAR ACCIDENT DETECTED!!\nLocation: https://www.google.com/maps/place/");
  gsm.print(lati, 6);
  gsm.print(",");
  gsm.print(longi, 6);
  gsm.write(0x1A);
  gsm.print("\r\n");
  delay(3000);

  smsFlag = 1;
}

void updateServer() {
  gsm.print("AT+HTTPPARA=\"URL\",\"http://esinebd.com/projects/iot-car-accident/api.php?la=");
  gsm.print(lati, 6);
  gsm.print("&lo=");
  gsm.print(longi, 6);
  gsm.print("&st=");
  gsm.print(accidentDetected);
  gsm.println("\"");
  delay(2000);
  gsm.println("AT+HTTPACTION=0");
  delay(3000);
}

void clearLcd() {
  lcd.clear();
  lcd.backlight();
}