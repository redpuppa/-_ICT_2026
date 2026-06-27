/*
  예제 10.3
  BLE를 이용한 내장 LED 제어
*/

#include <ArduinoBLE.h>

// LED 서비스와 특성(Characteristic) 정의
// UUID는 BLE에서 서비스/특성을 구분하는 고유 식별자, [guid]::NewGuid(), uuidgen 
BLEService ledService("19B10000-E8F2-537E-4F6C-D104768A1214");

// 읽기/쓰기 가능한 1바이트 특성 — 스마트폰이 이 값에 0 또는 1을 씀
BLEByteCharacteristic switchChar("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);

void setup() {
  Serial.begin(9600);
  while (!Serial) { }

  pinMode(LED_BUILTIN, OUTPUT);     // 내장 LED 출력 설정

  // ----- BLE 초기화 -----
  if (!BLE.begin()) {
    Serial.println("BLE 시작 실패!");
    while (1);                      // 실패 시 정지
  }

  BLE.setLocalName("UNO R4 LED Control");     // 스마트폰에 표시될 장치 이름(서로 다르게)
  BLE.setAdvertisedService(ledService);
  ledService.addCharacteristic(switchChar);   // 서비스에 특성 추가
  BLE.addService(ledService);                 // 서비스 등록

  switchChar.writeValue(0);         // 초기값: LED 꺼짐

  BLE.advertise();                  // 광고 시작 (연결 대기)
  Serial.println("BLE 대기 중... 'UNO R4 LED'로 연결하세요");
}

void loop() {
  // 연결된 중앙 장치(스마트폰 등)를 찾음
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("연결됨: ");
    Serial.println(central.address());

    // 연결이 유지되는 동안 반복
    while (central.connected()) {
      // 특성 값이 새로 써지면(스마트폰이 값을 보내면)
      if (switchChar.written()) {
        if (switchChar.value()) {          // 값이 1이면
          digitalWrite(LED_BUILTIN, HIGH); // LED 켜기
          Serial.println("LED ON");
        } else {                           // 값이 0이면
          digitalWrite(LED_BUILTIN, LOW);  // LED 끄기
          Serial.println("LED OFF");
        }
      }
    }

    Serial.print("연결 해제: ");
    Serial.println(central.address());
  }
}