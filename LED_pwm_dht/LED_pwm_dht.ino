#include <DHT.h>

#define DHTPIN 7
#define DHTTYPE DHT22
#define LED 11

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  dht.begin();
  Serial.println("DHT22 Sensor Test");
}

void loop() {
  delay(2000);
  

  static float baseline_temp = dht.readTemperature();
  static float baseline_hum = dht.readHumidity();


  if (isnan(baseline_temp) || isnan(baseline_hum)) {
    Serial.println("Failed to read from sensor!");
    return;
  }
  delay(2000);
  float new_temp = dht.readTemperature();
  float new_hum = dht.readHumidity();
  if (isnan(baseline_temp) || isnan(baseline_hum)) {
    Serial.println("Failed to read from sensor!");
    return;
  }
  if (new_hum > baseline_hum) {
    float dh = new_hum - baseline_hum;
    float power = map(dh, 0, 30, 0, 255);
    analogWrite(LED, power);
  }
  else {
    analogWrite(LED, LOW);
  }

}
