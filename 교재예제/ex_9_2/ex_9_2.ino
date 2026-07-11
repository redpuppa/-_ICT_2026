/*
 * 예제 9.2 수정본 (Uno R4 WiFi 대응)
 * 라이브러리: DHT sensor library by Adafruit
 */

#include "DHT.h"

#define DHTPIN 2     // DHT11이 연결된 핀
#define DHTTYPE DHT11   // DHT 11 센서 사용

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  Serial.println("EX 9.2 DHT11 Test for Uno R4 WiFi");
  
  dht.begin();
}

void loop() {
  // 측정 간격 (최소 2초 권장)
  delay(2000);

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // 센서 읽기 실패 시 처리
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("Temperature is ");
  Serial.print(t, 0);
  Serial.print("oC\t");
  Serial.print("Humidity is ");
  Serial.print(h, 0);
  Serial.println("%");
}