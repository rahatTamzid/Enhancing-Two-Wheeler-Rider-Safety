#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <U8g2lib.h>
#include "Wire.h"
#include "OLED.h"
#include <TinyGPS++.h>
#include <MPU6050_light.h>
#define gps_rxPin D5
#define gps_txPin D6
#define buzzer D7
#define BLUE D4
#define GREEN D3
OLED display(4, 5);
SoftwareSerial neogps(gps_rxPin, gps_txPin);
TinyGPSPlus gps;
MPU6050 mpu(Wire);
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/5, /* data=*/4);
const char *ssid = "Rahat";
const char *password = "bubt1234";
ESP8266WebServer server(80);
const int analogInPin = A0;
int bikeSpeed = 0;
void handleSentVar() {
  Serial.println("handleSentVar function called...");
}
void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println();
  neogps.begin(9600);
  display.begin();
  Wire.begin();
  pinMode(buzzer, OUTPUT);
  pinMode(BLUE, OUTPUT);
  pinMode(GREEN, OUTPUT);
  byte status = mpu.begin();
  Serial.print(F("MPU6050 status: "));
  Serial.println(status);
  while (status != 0) {}  // stop everything if could not connect to MPU6050
  Serial.println(F("Calculating offsets, do not move MPU6050"));
  delay(1000);
  mpu.calcOffsets();  // gyro and accelero
  Serial.println("Done!\n");
  Serial.print("Configuring access point...");
  u8g2.begin();
  u8g2.setFont(u8g2_font_logisoso62_tn);
  u8g2.setFontMode(0);  // enable transparent mode, which is faster
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.on("/data/", HTTP_GET, handleSentVar);
  server.begin();
  Serial.println("HTTP server started");
}
void loop() {
  //Angle Receiving Section from Bike unit MPU6050 & Helmet Unit MPU6050
  server.handleClient();
  mpu.update();
  //Converting Text message into mutable string for print
  // Convert constant string literals to mutable character arrays
  const char *string1 = "Warning";
  const char *string2 = "Align Your Head";
  const char *string3 = "Connected";
  const char *string4 = "Bike Speed";
  const char *string5 = "Angle Diff.";
  const char *string6 = "Safe Angle";
  const char *string7 = "Helmet Not";
  const char *string8 = "Connected";

  char mutable_string1[20];  // Create a mutable character array
  char mutable_string2[20];
  char mutable_string3[20];
  char mutable_string4[20];
  char mutable_string5[20];
  char mutable_string6[20];
  char mutable_string7[20];
  char mutable_string8[20];

  strcpy(mutable_string1, string1);
  strcpy(mutable_string2, string2);
  strcpy(mutable_string3, string3);
  strcpy(mutable_string4, string4);
  strcpy(mutable_string5, string5);
  strcpy(mutable_string6, string6);
  strcpy(mutable_string7, string7);
  strcpy(mutable_string8, string8);
  // End of Character conversion
  if (server.hasArg("sensor_reading")) {  // this is the variable sent from the client
                                          //Serial.println("Sensor reading received...")
    digitalWrite(GREEN, HIGH);
    delay(15);
    digitalWrite(GREEN, LOW);
    int readingInt = server.arg("sensor_reading").toInt();
    char readingToPrint[5];
    itoa(readingInt, readingToPrint, 10);  //integer to string conversion for OLED library
    u8g2.firstPage();
    u8g2.drawUTF8(0, 64, readingToPrint);
    u8g2.nextPage();
    server.send(200, "text/html", "Data received");
    Serial.print(" Helmet Angle: ");
    Serial.print(readingToPrint);
    Serial.print(" Bike Angle: ");
    Serial.print(mpu.getAngleZ());
    // End of Angle Receiving Section from Bike unit MPU6050 & Helmet Unit MPU6050
    int angleDifference = abs((readingInt) - (mpu.getAngleZ()));
    //Speed Collection Section
    boolean newData = false;
    for (unsigned long start = millis(); millis() - start < 1000;) {
      while (neogps.available()) {
        if (gps.encode(neogps.read())) {
          newData = true;
        }
      }
    }
    if (newData == true) {
      newData = false;
      if (gps.location.isValid() == 1) {
        String gps_speed = String(gps.speed.kmph());
        bikeSpeed = gps_speed.toInt();
        //display.print(gps.speed.kmph());
      }
    } else {
      Serial.print("No Speed Data");
    }
    //End of Speed collection
    Serial.print(bikeSpeed);
    while (bikeSpeed > 110) {
      bikeSpeed = 110;  //Setting max speed 110 KM/H
    }
    float safeAngle = (70 - (0.636 * bikeSpeed));
    //Converting integer value Char*
    char printableSafeangle[10];
    char printableBikespeed[10];
    char printableAngleDifference[10];
    sprintf(printableSafeangle, "%d", safeAngle);
    sprintf(printableBikespeed, "%d", bikeSpeed);
    sprintf(printableAngleDifference, "%d", angleDifference);
    //End of conversion
    if (angleDifference > safeAngle) {
      display.clear();
      display.print(mutable_string1, 3, 5);
      display.print(mutable_string2, 5, 1);
      digitalWrite(buzzer, HIGH);
      delay(50);
      digitalWrite(buzzer, LOW);
      display.print(mutable_string1, 3, 5);
      display.print(mutable_string2, 5, 1);
    } else {
      display.clear();
      display.print(mutable_string3, 0, 1);
      display.print(mutable_string4, 1, 1);
      display.print(printableBikespeed, 1, 14);
      display.print(mutable_string5, 2, 1);
      display.print(printableAngleDifference, 2, 14);
      display.print(mutable_string6, 3, 1);
      display.print(printableSafeangle, 3, 14);
      digitalWrite(buzzer, LOW);
    }
  } else {
    display.clear();
    display.print(mutable_string7, 3, 3);
    display.print(mutable_string8, 5, 3);
    digitalWrite(BLUE, HIGH);
    delay(20);
    digitalWrite(BLUE, LOW);
  }
}