/*
  [방법 C] 보드가 MQTT "브로커"로 값을 발행(publish)하고, PC가 구독(subscribe)한다.
  - 보드→브로커→PC 구조. 여러 대의 기기/원격 수집으로 확장하기 쉽다.
  - 이 예제는 설치 없이 쓰는 공개 테스트 브로커(test.mosquitto.org)를 사용.
    (수업에서 여러 반이 동시에 쓰면 topic이 겹칠 수 있으니 TOPIC을 반별로 바꿔 쓰세요.)
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
const int  port     = 1883;                  // MQTT 기본 포트
const char topic[]  = "utech/uno_r4/adc";    // ★ 수업/반별로 고유하게 변경 권장

void setup() {
  Serial.begin(115200);
  analogReadResolution(ADC_BIT);

  Serial.print("WiFi 연결 중");
  WiFi.begin(SECRET_SSID, SECRET_PASS);
  while (WiFi.status() != WL_CONNECTED) {
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
}

void loop() {
  mqttClient.poll();               // 브로커 연결(keep-alive) 유지

  int   adc  = analogRead(A0);
  float volt = (VA_REF / RESOLUTION) * adc;

  // 토픽에 "adc,volt" 한 줄을 발행
  mqttClient.beginMessage(topic);
  mqttClient.print(adc);
  mqttClient.print(",");
  mqttClient.print(volt, 3);
  mqttClient.endMessage();

  Serial.print("발행: ");
  Serial.print(adc);
  Serial.print(",");
  Serial.println(volt, 3);

  delay(1000);
}
