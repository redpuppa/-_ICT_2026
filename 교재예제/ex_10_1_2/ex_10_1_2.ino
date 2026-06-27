/*
  예제 10.1.2
  NTP를 이용한 내장 RTC 설정
*/
#include <RTC.h>
#include <NTPClient.h>
#include <WiFiS3.h>
#include <WiFiUdp.h>

// ----- WiFi 접속 정보 -----
const char* ssid = "galaxy-lmh";
const char* pass = "abcdefgh";

// 한국 표준시(KST) = UTC + 9시간 = 32400초
const long KST_OFFSET = 32400;

// 요일(DayOfWeek) 값을 한글 문자열로 변환
const char* dayName(DayOfWeek d) {
  switch (d) {
    case DayOfWeek::MONDAY:    return "월요일";
    case DayOfWeek::TUESDAY:   return "화요일";
    case DayOfWeek::WEDNESDAY: return "수요일";
    case DayOfWeek::THURSDAY:  return "목요일";
    case DayOfWeek::FRIDAY:    return "금요일";
    case DayOfWeek::SATURDAY:  return "토요일";
    case DayOfWeek::SUNDAY:    return "일요일";
  }
  return "?";
}

// 한 자리 숫자 앞에 0을 붙여 두 자리로 출력 (예: 9 -> 09)
void print2(int v) {
  if (v < 10) Serial.print("0");
  Serial.print(v);
}

void setup() {
  Serial.begin(9600);
  while (!Serial) { }          // 시리얼 모니터가 열릴 때까지 대기

  RTC.begin();                 // 내장 RTC 시작

  // ----- 1. WiFi 연결 -----
  Serial.print("WiFi 연결 중");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" 완료!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  // ----- 2. NTP에서 현재 시각 읽기 (WiFiS3 내장) -----
  Serial.print("NTP 시각 동기화 중");
  unsigned long utc = WiFi.getTime();
  // 2001-01-01(978307200) 이전 값이면 아직 동기화 안 된 것 → 재시도
  while (utc < 978307200UL) {
    delay(1000);
    Serial.print(".");
    utc = WiFi.getTime();
  }
  Serial.println(" 완료!");

  unsigned long epoch = utc + KST_OFFSET;   // UTC -> KST 보정

  // ----- 3. NTP 시각을 내장 RTC에 저장 -----
  RTCTime startTime(epoch);    // epoch 초로 RTCTime 생성 (날짜/시간/요일 자동 계산)
  RTC.setTime(startTime);      // RTC에 시간 저장
  Serial.println("NTP -> RTC 동기화 완료");
}

void loop() {
  RTCTime currentTime;

  // 현재시간 가져오기
  RTC.getTime(currentTime);

  // 시리얼 모니터 출력
  Serial.print("현재시간: ");
  Serial.print(currentTime.getYear());              Serial.print("년 ");
  Serial.print(Month2int(currentTime.getMonth()));  Serial.print("월 ");
  Serial.print(currentTime.getDayOfMonth());        Serial.print("일 ");
  Serial.print(dayName(currentTime.getDayOfWeek()));
  Serial.print("  ");

  // ----- 시:분:초 (두 자리 맞춤) -----
  print2(currentTime.getHour());    Serial.print(":");
  print2(currentTime.getMinutes()); Serial.print(":");
  print2(currentTime.getSeconds());
  Serial.println();

  delay(1000);                 // 1초마다 갱신
}