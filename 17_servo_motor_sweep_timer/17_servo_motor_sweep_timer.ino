// 15_servo_motor 예제(Servo.h 사용, 0→180° 스윕)를
// Servo.h 없이 하드웨어 타이머 PWM으로 다시 작성한 버전 — "핀 고정" 방식.
// 보드: Arduino UNO R4 WiFi (RA4M1)
//
// [핀 고정 원리]
//   앞선 FspTimer::get_available_timer() 방식은 "빈 타이머"를 자동으로 잡기 때문에
//   그 출력이 반드시 9번 핀으로 나온다는 보장이 없다.
//   여기서는 코어의 PwmOut 클래스를 쓴다. 생성자에 핀 번호(9)를 주면,
//   내부에서 그 핀에 연결된 타이머를 찾아준다.
//     D9 = P303 = GPT 채널 7 / PWM_CHANNEL_B  ← 코어 핀 매핑 테이블 기준
//   즉 "9번 핀 전용 타이머"를 명시적으로 고정해 사용한다.
//
// [서보 제어 원리]
//   50Hz(20ms 주기) PWM에서 펄스 폭(HIGH 시간)으로 각도를 지정한다.
//   0°=570µs, 180°=2400µs (기존 attach(pin, 570, 2400)과 동일한 범위).

#include "pwm.h"

const int  SERVO_PIN    = 9;      // D9 = P303 = GPT ch7 / PWM_CHANNEL_B
const long SERVO_MIN_US = 570;    // 0°   에 해당하는 펄스 폭
const long SERVO_MAX_US = 2400;   // 180° 에 해당하는 펄스 폭

PwmOut servo(SERVO_PIN);          // 핀 고정: 생성자에 핀 번호를 지정

void setup() {
  // 주기 20000µs(=20ms, 50Hz), 초기 펄스 570µs(0°).
  // 세 번째 인자 false = period/pulse를 µs 단위로 해석(분주비 자동 선택).
  servo.begin(20000, (uint32_t)SERVO_MIN_US, false);
}

// 각도(0~180°)를 펄스 폭(µs)으로 환산해 출력
void writeAngle(int angle) {
  if (angle < 0)   angle = 0;
  if (angle > 180) angle = 180;

  // 각도 → 펄스 폭(µs). Servo.h의 write()와 동일한 매핑.
  long us = map(angle, 0, 180, SERVO_MIN_US, SERVO_MAX_US);

  // 펄스 폭만 갱신하면 각도가 바뀐다. (내부에서 µs→카운트 변환 처리)
  servo.pulseWidth_us(us);
}

void loop() {
  // 0 → 180° 스윕 (기존 15_servo_motor와 동일한 동작)
  for (int i = 0; i <= 180; i++) {
    writeAngle(i);
    delay(10);
  }
}
