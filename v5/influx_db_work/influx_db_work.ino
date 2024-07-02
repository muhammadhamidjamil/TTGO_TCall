#include "arduino_secrets.h"

#if defined(ESP32)
#include <WiFiMulti.h>
WiFiMulti wifiMulti;
#define DEVICE "ESP32"
#elif defined(ESP8266)
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti wifiMulti;
#define DEVICE "ESP8266"
#endif

#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

// WiFi AP SSID
#define WIFI_SSID MY_SSID
// WiFi password
#define WIFI_PASSWORD MY_PASSWORD

#define INFLUXDB_URL _INFLUXDB_URL
#define INFLUXDB_TOKEN _INFLUXDB_TOKEN
#define INFLUXDB_ORG _INFLUXDB_ORG
#define INFLUXDB_BUCKET _INFLUXDB_BUCKET

// Time zone info
#define TZ_INFO "UTC5"

// Declare InfluxDB client instance with pre configured InfluxCloud certificate
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET,
                      INFLUXDB_TOKEN, InfluxDbCloud2CACert);

// Declare Data point
Point sensor("Sensors");

void setup() {
  Serial.begin(115200);

  // Setup wifi
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to wifi");
  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.println();

  // Accurate time is necessary for certificate validation and writing in
  // batches We use the NTP servers in your area as provided by:
  // https://www.pool.ntp.org/zone/ Syncing progress and the time will be
  // printed to Serial.
  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");

  // Check server connection
  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }

  sensor.addTag("room_temperature", DEVICE);
}
void loop() {
  // Clear fields for reusing the point. Tags will remain the same as set above.
  sensor.clearFields();

  sensor.addField("Temperature", WiFi.RSSI());
  sensor.addField("Humidity", WiFi.RSSI());
  // Print what are we exactly writing
  Serial.print("Writing: ");
  Serial.println(sensor.toLineProtocol());

  // Check WiFi connection and reconnect if needed
  if (wifiMulti.run() != WL_CONNECTED) {
    Serial.println("Wifi connection lost");
  }

  // Write point
  if (!client.writePoint(sensor)) {
    Serial.print("InfluxDB write failed: ");
    Serial.println(client.getLastErrorMessage());
  }

  Serial.println("Waiting 1 second");
  delay(1000);
}
