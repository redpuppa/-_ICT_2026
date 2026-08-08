/*
  [방법 C] 보드와 PC가 MQTT 브로커를 통해 "서로" 발행(publish)/구독(subscribe)한다.
  - 보드 → 브로커 → PC : 센서값(adc,volt) 발행/구독   (topic: .../data)
  - PC   → 브로커 → 보드 : 내장 LED 켜기/끄기 명령 발행/구독 (topic: .../led/set)
  - 보드 → 브로커 → PC : LED 적용 결과(상태) 발행/구독      (topic: .../led/state)
  - 이 예제는 설치 없이 쓰는 공개 테스트 브로커(test.mosquitto.org)를 사용.
    (수업에서 여러 반이 동시에 쓰면 topic이 겹칠 수 있으니 BASE_TOPIC을 반별로 바꿔 쓰세요.)
  - 센서: 가변저항(가운데 핀 → A0), 양끝 → 5V / GND

  보드: Arduino UNO R4 WiFi
  필요 라이브러리:
    - WiFiS3 (보드 패키지 기본 포함)
    - ArduinoMqttClient  (라이브러리 매니저에서 설치)
*/
#include "WiFiS3.h"
#include <ArduinoMqttClient.h>
#include "arduino_secrets.h"       // SECRET_SSID / SECRET_PASS

#define ADC_BIT    12
#define RESOLUTION (1 << ADC_BIT)  // 4096 (12-bit)
#define VA_REF     5.0

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

// ── MQTT 브로커 설정: 아래 둘 중 하나만 사용 ───────────────────────
// [방식 1] 공개 테스트 브로커 — 설치 불필요, 인터넷만 되면 됨(값이 외부로 나감)
const char broker[] = "test.mosquitto.org";
// [방식 2] 로컬 브로커(WSL의 mosquitto) — 아래 줄의 주석을 풀고 위 줄을 주석 처리.
//          주소는 "Windows PC의 LAN IP"(ipconfig의 IPv4). 설정법은 README 2.4 참고.
// const char broker[] = "192.168.0.10";
// ──────────────────────────────────────────────────────────────
const int  port      = 1883;                          // MQTT 기본 포트
const char BASE_TOPIC[] = "ict2026/uno_r4/mhlee";      // ★ 수업/반별로 고유하게 변경 권장

// 실제 사용할 3개의 세부 토픽 (BASE_TOPIC 뒤에 이어붙임)
String dataTopic;       // 보드 → PC : "adc,volt"
String ledSetTopic;     // PC → 보드 : "1"(ON) / "0"(OFF)
String ledStateTopic;   // 보드 → PC : 적용된 LED 상태 "1"/"0"

void onMqttMessage(int messageSize);   // 구독 메시지 수신 콜백(선언)

void setup() {
  Serial.begin(115200);
  analogReadResolution(ADC_BIT);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  dataTopic     = String(BASE_TOPIC) + "/data";
  ledSetTopic   = String(BASE_TOPIC) + "/led/set";
  ledStateTopic = String(BASE_TOPIC) + "/led/state";

  Serial.print("WiFi 연결 중");
  WiFi.begin(SECRET_SSID, SECRET_PASS);
  // WL_CONNECTED 이면서 실제 IP(DHCP)를 받을 때까지 대기
  // (실패 시 0.0.0.0으로 넘어가는 것을 방지)
  while (WiFi.status() != WL_CONNECTED || WiFi.localIP() == IPAddress(0, 0, 0, 0)) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("보드 IP: ");
  Serial.println(WiFi.localIP());

  Serial.print("MQTT 브로커 연결...");
  while (!mqttClient.connect(broker, port)) {
    Serial.print(" 실패 rc=");
    Serial.println(mqttClient.connectError());
    delay(2000);
  }
  Serial.println(" 성공");

  // ----- 구독: PC가 보내는 LED 명령을 받기 위해 콜백 등록 후 subscribe -----
  mqttClient.onMessage(onMqttMessage);
  mqttClient.subscribe(ledSetTopic);
  Serial.print("구독 시작: ");
  Serial.println(ledSetTopic);
}

void loop() {
  mqttClient.poll();               // 브로커 연결(keep-alive) 유지 + 구독 메시지 수신 처리

  int   adc  = analogRead(A0);
  float volt = (VA_REF / RESOLUTION) * adc;

  // ----- 발행: 센서값(adc,volt) 한 줄 -----
  mqttClient.beginMessage(dataTopic);
  mqttClient.print(adc);
  mqttClient.print(",");
  mqttClient.print(volt, 3);
  mqttClient.endMessage();

  Serial.print("발행[data]: ");
  Serial.print(adc);
  Serial.print(",");
  Serial.println(volt, 3);

  delay(1000);
}

// ----- 구독 메시지 수신 콜백: PC → 보드 LED 명령 처리 -----
void onMqttMessage(int messageSize) {
  String topic = mqttClient.messageTopic();

  String payload;
  while (mqttClient.available()) {
    payload += (char)mqttClient.read();
  }
  payload.trim();

  if (topic == ledSetTopic) {
    bool on = (payload == "1" || payload.equalsIgnoreCase("on"));
    digitalWrite(LED_BUILTIN, on ? HIGH : LOW);

    Serial.print("수신[led/set]: ");
    Serial.println(payload);

    // ----- 발행: 적용된 LED 상태를 다시 알려줌 -----
    mqttClient.beginMessage(ledStateTopic);
    mqttClient.print(on ? "1" : "0");
    mqttClient.endMessage();

    Serial.print("발행[led/state]: ");
    Serial.println(on ? "1" : "0");
  }
}
