/*
  [방법 C] MQTT 양방향 통신
  UNO R4 WiFi <-> MQTT Broker <-> PC

  UNO R4
    - Publish : /data       -> ADC, 전압
    - Subscribe: /led/set   <- PC의 LED 명령
    - Publish : /led/state  -> 실제 적용된 LED 상태

  PC
    - Subscribe: /data, /led/state
    - Publish  : /led/set

  필요한 라이브러리:
    - WiFiS3
    - ArduinoMqttClient
*/

#include "WiFiS3.h"
#include <ArduinoMqttClient.h>
#include "arduino_secrets.h"       // SECRET_SSID / SECRET_PASS

#define ADC_BIT    12
#define RESOLUTION (1 << ADC_BIT)
#define VA_REF     5.0

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

const char broker[] = "test.mosquitto.org";
const int  port = 1883;

// ★ PC의 BASE_TOPIC과 반드시 동일해야 함
const char BASE_TOPIC[]      = "utech/uno_r4/soony";

const char DATA_TOPIC[]      = "utech/uno_r4/soony/data";
const char LED_SET_TOPIC[]   = "utech/uno_r4/soony/led/set";
const char LED_STATE_TOPIC[] = "utech/uno_r4/soony/led/state";

unsigned long prevMillis = 0;
const unsigned long interval = 1000;

void connectMQTT();
void onMqttMessage(int messageSize);
void publishLedState();

void setup() {
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  analogReadResolution(ADC_BIT);

  // ── WiFi 연결 ─────────────────────────────────────
  Serial.print("WiFi 연결 중");

  WiFi.begin(SECRET_SSID, SECRET_PASS);

  while (WiFi.status() != WL_CONNECTED ||
         WiFi.localIP() == IPAddress(0, 0, 0, 0)) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("보드 IP: ");
  Serial.println(WiFi.localIP());

  // ── MQTT 연결 ─────────────────────────────────────
  connectMQTT();

  // PC가 보내는 LED 명령을 구독
  mqttClient.onMessage(onMqttMessage);
  mqttClient.subscribe(LED_SET_TOPIC);

  Serial.print("구독: ");
  Serial.println(LED_SET_TOPIC);

  // 현재 LED 상태를 PC에 알려준다.
  publishLedState();
}

void loop() {
  // MQTT 수신 처리
  mqttClient.poll();

  // 연결이 끊어진 경우 재연결
  if (!mqttClient.connected()) {
    Serial.println("MQTT 연결 끊김. 재연결 중...");
    connectMQTT();

    mqttClient.onMessage(onMqttMessage);
    mqttClient.subscribe(LED_SET_TOPIC);

    publishLedState();
  }

  // ── 1초마다 센서값 Publish ─────────────────────────
  unsigned long now = millis();

  if (now - prevMillis >= interval) {
    prevMillis = now;

    int adc = analogRead(A0);
    float volt = (VA_REF / RESOLUTION) * adc;

    mqttClient.beginMessage(DATA_TOPIC);
    mqttClient.print(adc);
    mqttClient.print(",");
    mqttClient.print(volt, 3);
    mqttClient.endMessage();

    Serial.print("발행[");
    Serial.print(DATA_TOPIC);
    Serial.print("] : ");
    Serial.print(adc);
    Serial.print(",");
    Serial.println(volt, 3);
  }
}

// ── MQTT 브로커 연결 ─────────────────────────────────
void connectMQTT() {
  Serial.print("MQTT 브로커 연결...");

  while (!mqttClient.connect(broker, port)) {
    Serial.print(" 실패 rc=");
    Serial.println(mqttClient.connectError());
    delay(2000);
  }

  Serial.println(" 성공");
}

// ── PC가 /led/set으로 보낸 명령 처리 ─────────────────
void onMqttMessage(int messageSize) {
  String command = "";

  while (mqttClient.available()) {
    command += (char)mqttClient.read();
  }

  command.trim();

  Serial.print("수신[");
  Serial.print(LED_SET_TOPIC);
  Serial.print("] : ");
  Serial.println(command);

  bool changed = false;

  if (command == "1" || command == "ON" || command == "on") {
    digitalWrite(LED_BUILTIN, HIGH);
    changed = true;
  }
  else if (command == "0" || command == "OFF" || command == "off") {
    digitalWrite(LED_BUILTIN, LOW);
    changed = true;
  }

  if (changed) {
    Serial.println(
      digitalRead(LED_BUILTIN) ? "LED -> ON" : "LED -> OFF"
    );

    // 실제 적용 상태를 PC에 다시 Publish
    publishLedState();
  }
}

// ── 현재 LED 상태를 PC에 Publish ──────────────────────
void publishLedState() {
  const char *state =
    digitalRead(LED_BUILTIN) ? "1" : "0";

  mqttClient.beginMessage(LED_STATE_TOPIC);
  mqttClient.print(state);
  mqttClient.endMessage();

  Serial.print("발행[");
  Serial.print(LED_STATE_TOPIC);
  Serial.print("] : ");
  Serial.println(state);
}
