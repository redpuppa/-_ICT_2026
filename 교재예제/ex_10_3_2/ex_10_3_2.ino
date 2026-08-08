/*
  예제 10.3.2
  BLE로 RTC 시간 보내기 — 스마트폰 앱이 "read"하면 현재 시각(문자열)을 돌려준다.
  (ex_10_3의 write 방향[스마트폰→보드]에 대응하는 read 방향[보드→스마트폰] 예제)

  동작:
   - 스마트폰(nRF Connect 등)이 시간 특성을 읽으면 "2026-07-25 14:30:05" 형태의 문자열을 받는다.
   - BLERead 이벤트 핸들러가 읽는 순간 RTC 값을 갱신하므로 항상 현재 시각이 나간다.
   - 구독(Notify)하면 1초마다 자동으로 갱신 값을 밀어준다.
*/

#include <ArduinoBLE.h>
#include <RTC.h>

// 시간 서비스와 특성 정의
BLEService timeService("19B10000-E8F2-537E-4F6C-D104768A1214");

// 스마트폰이 읽으면(read)/구독하면(notify) 시각 문자열을 받는 특성 (최대 25바이트)
BLEStringCharacteristic timeChar("19B10002-E8F2-537E-4F6C-D104768A1214",
                                 BLERead | BLENotify, 25);

// 현재 RTC 시각을 "YYYY-MM-DD HH:MM:SS" 문자열로 만든다
String currentTimeString() {
  RTCTime now;
  RTC.getTime(now);

  char buf[25];
  sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d",
          now.getYear(),
          Month2int(now.getMonth()),
          now.getDayOfMonth(),
          now.getHour(),
          now.getMinutes(),
          now.getSeconds());
  return String(buf);
}

// 스마트폰이 이 특성을 read하는 순간 호출되는 콜백
// → 읽기 직전에 최신 RTC 값으로 갱신하므로 항상 현재 시각이 전송된다
void onTimeRead(BLEDevice central, BLECharacteristic characteristic) {
  timeChar.writeValue(currentTimeString());
  Serial.print("read 요청 → 전송: ");
  Serial.println(currentTimeString());
}

void setup() {
  Serial.begin(9600);
  while (!Serial) { }

  // ----- RTC 초기화 및 시각 설정 -----
  RTC.begin();
  // 최초 1회 현재 시각으로 설정 (VRTC 백업이 있으면 이후 주석 처리 가능)
  RTCTime startTime(25, Month::JULY, 2026, 14, 30, 0,
                    DayOfWeek::SATURDAY, SaveLight::SAVING_TIME_INACTIVE);
  RTC.setTime(startTime);

  // ----- BLE 초기화 -----
  if (!BLE.begin()) {
    Serial.println("BLE 시작 실패!");
    while (1);
  }

  BLE.setLocalName("UNO R4 TIME mhlee");        // 스마트폰에 표시될 이름(서로 다르게)
  BLE.setAdvertisedService(timeService);
  timeService.addCharacteristic(timeChar);
  BLE.addService(timeService);

  // read 이벤트 핸들러 등록 (읽는 순간 RTC 값 갱신)
  timeChar.setEventHandler(BLERead, onTimeRead);

  timeChar.writeValue(currentTimeString());     // 초기값
  BLE.advertise();
  Serial.println("BLE 대기 중... 'UNO R4 TIME mhlee'로 연결 후 시간 특성을 read 하세요");
}

void loop() {
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("연결됨: ");
    Serial.println(central.address());

    unsigned long lastUpdate = 0;
    while (central.connected()) {
      BLE.poll();                       // BLE 이벤트 처리 (read 콜백 포함)

      // 구독(Notify) 중인 앱을 위해 1초마다 최신 값 갱신
      if (millis() - lastUpdate >= 1000) {
        lastUpdate = millis();
        timeChar.writeValue(currentTimeString());
      }
    }

    Serial.print("연결 해제: ");
    Serial.println(central.address());
  }
}
