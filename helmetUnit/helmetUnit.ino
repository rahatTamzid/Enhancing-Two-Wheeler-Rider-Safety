#include <ESP8266WiFi.h>
#include <OneWire.h>
#include "Wire.h"
#include <MPU6050_light.h>
MPU6050 mpu(Wire);
const char *ssid = "Rahat";
const char *password = "bubt1234";
int temp = 0; 
void setup() {
  Serial.begin(115200);
  delay(10);
  Wire.begin();
  byte status = mpu.begin();
  Serial.print(F("MPU6050 status: "));
  Serial.println(status);
  while (status != 0) {}  // stop everything if could not connect to MPU6050
  Serial.println(F("Calculating offsets, do not move MPU6050"));
  delay(1000);
  // mpu.upsideDownMounting = true; // uncomment this line if the MPU6050 is mounted upside-down
  mpu.calcOffsets();  // gyro and accelero
  Serial.println("Done!\n");
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
void loop() {
  mpu.update();
  temp = (mpu.getAngleZ());
  Serial.println(temp);
  Serial.print("X : ");
  Serial.print(mpu.getAngleX());
  Serial.print("\tY : ");
  Serial.print(mpu.getAngleY());
  Serial.print("\tZ : ");
  Serial.println(mpu.getAngleZ());
  char intToPrint[5];
  itoa(temp, intToPrint, 10);  //integer to string conversion for OLED library
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const char *host = "192.168.4.1";
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
  // We now create a URI for the request
  String url = "/data/";
  url += "?sensor_reading=";
  url += intToPrint;
  Serial.print("Requesting URL: ");
  Serial.println(url);
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }
}