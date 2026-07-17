/*
  [방법 B] 보드가 PC로 값을 "밀어보낸다" (TCP 클라이언트 → PC 서버)
  - PC  : TCP 서버를 열고 대기 (pc_collect_B.py)
  - 보드: PC의 IP:포트로 접속해 "adc,volt" 한 줄씩 전송
  - 연속 수집·CSV 저장에 가장 적합한 방식
  - 센서: 가변저항(가운데 핀 → A0), 양끝 → 5V / GND

  보드: Arduino UNO R4 WiFi
  필요 라이브러리: WiFiS3 (UNO R4 보드 패키지에 기본 포함)
*/
#include "WiFiS3.h"
#include "arduino_secrets.h"       // SECRET_SSID / SECRET_PASS

#define ADC_BIT    12
#define RESOLUTION (1 << ADC_BIT)  // 4096 (12-bit)
#define VA_REF     5.0

const char*    PC_IP   = "192.168.0.10";  // ★ PC(수집기)의 IP로 변경
const uint16_t PC_PORT = 9000;            // pc_collect_B.py의 포트와 동일

WiFiClient client;

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
}

void loop() {
  // 연결이 끊겨 있으면 PC 서버에 (재)접속
  if (!client.connected()) {
    client.stop();
    Serial.print("PC 서버에 연결 시도...");
    if (client.connect(PC_IP, PC_PORT)) {
      Serial.println(" 성공");
    } else {
      Serial.println(" 실패 (PC 수집기를 먼저 실행했는지 확인). 3초 후 재시도");
      delay(3000);
      return;
    }
  }

  // 센서 읽고 CSV 한 줄 전송
  int   adc  = analogRead(A0);
  float volt = (VA_REF / RESOLUTION) * adc;

  client.print(adc);
  client.print(",");
  client.println(volt, 3);         // println → 줄바꿈으로 한 줄 구분

  Serial.print("전송: ");
  Serial.print(adc);
  Serial.print(",");
  Serial.println(volt, 3);

  delay(1000);                     // 1초 주기
}
