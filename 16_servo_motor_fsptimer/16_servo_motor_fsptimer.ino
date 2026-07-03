#include "FspTimer.h"

FspTimer servoTimer;
const int SERVO_PIN = 9;

void setup() {
  uint8_t type;
  int8_t ch = FspTimer::get_available_timer(type);

  // 반드시 begin → open → start 순서를 지켜야 합니다.
  servoTimer.begin(TIMER_MODE_PWM, type, ch, 50.0f, 50.0f);  // 50Hz = 20ms, 50%
  // servoTimer.setup_overflow_irq();
  servoTimer.open();
  servoTimer.start();

  writeMicroseconds(1500);   // 시작은 중앙(90°)
}

void writeMicroseconds(uint32_t us) {
  // set_pulse_us()가 내부에서 µs → 카운트 변환을 처리해 준다.
  // 두 번째 인자는 채널: CHANNEL_A 또는 CHANNEL_B
  servoTimer.set_pulse_us((double)us, CHANNEL_A);
}

void loop() {
  writeMicroseconds(1000); delay(1000);   // 약 0°
  writeMicroseconds(1500); delay(1000);   // 약 90°
  writeMicroseconds(2000); delay(1000);   // 약 180°
}