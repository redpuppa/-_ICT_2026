/*
  예제 10.1.1
  내장 RTC를 이용한 날짜, 시간, 요일 표시
*/
#include <RTC.h>

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

  // RTC 초기화
  RTC.begin();                 // 내장 RTC 시작

  // 저장할 날짜/시간 구성: 2026년 6월 21일(일) 14:30:00
  RTCTime startTime(21, Month::JUNE, 2026, 14, 30, 0,
                    DayOfWeek::SUNDAY, SaveLight::SAVING_TIME_INACTIVE);

  RTC.setTime(startTime);      // RTC에 시간 저장
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