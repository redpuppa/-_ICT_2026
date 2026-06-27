#include "FspTimer.h"

FspTimer servoTimer;
const int SERVO_PIN = 9;

uint32_t periodCounts = 0;   // 한 주기(20ms)에 해당하는 카운트 수

void setup() {
  uint8_t type;
  int8_t ch = FspTimer::get_available_timer(type);

  servoTimer.begin(TIMER_MODE_PWM, type, ch, 50.0f, 50.0f);  // 50Hz = 20ms 주기
  servoTimer.setup_overflow_irq();
  servoTimer.open();

  periodCounts = servoTimer.get_period_counts();  // 주기 전체 카운트 획득

  servoTimer.start();

  writeMicroseconds(1500);   // 시작은 중앙(90°)
}

void writeMicroseconds(uint32_t us) {
  // 20000µs 주기 대비 펄스 폭 비율 → 카운트로 환산
  uint32_t counts = (uint64_t)periodCounts * us / 20000UL;
  // 두 번째 인자는 채널: CHANNEL_A 또는 CHANNEL_B
  servoTimer.set_duty_cycle(counts, CHANNEL_A);
}

void loop() {
  writeMicroseconds(1000); delay(1000);   // 약 0°
  writeMicroseconds(1500); delay(1000);   // 약 90°
  writeMicroseconds(2000); delay(1000);   // 약 180°
}