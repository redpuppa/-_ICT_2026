/*
  [방법 A] 보드가 "웹서버"가 되고, PC가 값을 가져간다 (HTTP GET)
  - 보드: WiFi(STA)로 공유기에 접속 → 80번 포트로 웹서버 개설
  - PC  : 브라우저나 파이썬으로 http://보드IP 에 접속하면 "adc,volt" 한 줄을 응답
  - 센서: 가변저항(가운데 핀 → A0), 양끝 → 5V / GND

  보드: Arduino UNO R4 WiFi
  필요 라이브러리: WiFiS3 (UNO R4 보드 패키지에 기본 포함)
*/
#include "WiFiS3.h"
#include "arduino_secrets.h"       // SECRET_SSID / SECRET_PASS

#define ADC_BIT    12
#define RESOLUTION (1 << ADC_BIT)  // 4096 (12-bit)
#define VA_REF     5.0             // 기준 전압 5V

WiFiServer server(80);             // 80번 포트(HTTP) 웹서버

void setup() {
  Serial.begin(115200);
  analogReadResolution(ADC_BIT);   // ADC를 12비트로 (11_ADC_VR과 동일)

  Serial.print("WiFi 연결 중");
  WiFi.begin(SECRET_SSID, SECRET_PASS);
  while (WiFi.status() != WL_CONNECTED) {   // 연결될 때까지 대기
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  server.begin();
  Serial.print("연결 완료! PC/브라우저에서 접속할 주소 →  http://");
  Serial.println(WiFi.localIP());  // ★ 이 IP를 PC 수집기(pc_collect_A.py)에 입력
}

void loop() {
  WiFiClient client = server.available();   // 접속한 PC가 있는가?
  if (!client) return;

  // HTTP 요청 헤더를 빈 줄이 나올 때까지 읽어서 버린다.
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") break;                // 헤더의 끝(빈 줄)
  }

  // 센서 읽기
  int   adc  = analogRead(A0);
  float volt = (VA_REF / RESOLUTION) * adc;

  // 응답 전송 (본문은 "adc,volt" CSV 한 줄)
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/plain");
  client.println("Access-Control-Allow-Origin: *");
  client.println("Connection: close");
  client.println();                          // 헤더와 본문 사이 빈 줄
  client.print(adc);
  client.print(",");
  client.println(volt, 3);

  client.stop();                             // 연결 종료
}
