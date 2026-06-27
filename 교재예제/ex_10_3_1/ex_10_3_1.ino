/*
  예제 10.3.1
  BLE로 문자열을 받아 LED 매트릭스에 출력
*/

#include <ArduinoBLE.h>
#include "ArduinoGraphics.h"
#include "Arduino_LED_Matrix.h"

ArduinoLEDMatrix matrix;   // 내장 LED 매트릭스 객체

// BLE 서비스 정의
BLEService textService("19B10000-E8F2-537E-4F6C-D104768A1214");

// 문자열을 받을 특성 — 최대 32바이트, 읽기/쓰기 가능
BLEStringCharacteristic textChar("19B10001-E8F2-537E-4F6C-D104768A1214",
                                 BLERead | BLEWrite, 32);

// 현재 매트릭스에 표시할 문자열 (기본값)
String message = "UNO R4";

// 문자열을 매트릭스에 스크롤 출력하는 함수
void scrollText(String text) {
  matrix.beginDraw();
  matrix.stroke(0xFFFFFFFF);
  matrix.textScrollSpeed(60);
  matrix.textFont(Font_5x7);
  matrix.beginText(0, 1, 0xFFFFFF);
  matrix.println(text);
  matrix.endText(SCROLL_LEFT);
  matrix.endDraw();
}

void setup() {
  Serial.begin(9600);
  while (!Serial) { }

  matrix.begin();                 // 매트릭스 시작

  // ----- BLE 초기화 -----
  if (!BLE.begin()) {
    Serial.println("BLE 시작 실패!");
    while (1);
  }

  BLE.setLocalName("UNO R4 Matrix");      // 스마트폰에 표시될 이름
  BLE.setAdvertisedService(textService);
  textService.addCharacteristic(textChar);
  BLE.addService(textService);

  textChar.writeValue(message);   // 초기값 설정

  BLE.advertise();                // 광고 시작 (연결 대기)
  Serial.println("BLE 대기 중... 'UNO R4 Matrix'로 연결하세요");
}

void loop() {
  BLEDevice central = BLE.central();    // 연결된 장치 확인

  if (central) {
    Serial.print("연결됨: ");
    Serial.println(central.address());

    while (central.connected()) {
      // 스마트폰이 새 문자열을 보내면
      if (textChar.written()) {
        message = textChar.value();      // 받은 문자열 저장
        Serial.print("수신: ");
        Serial.println(message);
      }
      // 받은(또는 기본) 문자열을 계속 스크롤
      scrollText(message);
    }

    Serial.print("연결 해제: ");
    Serial.println("BLE 대기 중... 'UNO R4 Matrix'로 연결하세요");
    Serial.println(central.address());
  } else {
    // 연결이 없을 때도 기본 문자열 스크롤
    scrollText(message);
  }
}