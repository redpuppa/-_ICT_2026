/*
  예제 17. FspTimer를 이용한 LED 밝기 조절
  - 12번 예제(analogWrite)와 같은 동작을 FspTimer로 직접 구현
  - LED: 5번 핀 (P107 = GPT 타이머 0번, 채널 A)
  - PWM 주파수 1kHz, 듀티 사이클(0~100%)로 밝기 조절
*/
#include "FspTimer.h"

FspTimer ledTimer;

// 밝기를 0~100(%)로 설정
void setBrightness(float percent) {
  uint32_t period = ledTimer.get_period_raw();               // 한 주기의 카운트 수
  uint32_t pulse  = (uint32_t)(period * percent / 100.0f);   // 듀티 → 카운트 변환
  ledTimer.set_duty_cycle(pulse, CHANNEL_A);
}

void setup() {
  // LED 모듈 전원 (12번 예제와 동일한 배선)
  pinMode(3, OUTPUT);
  digitalWrite(3, HIGH);

  // D5(P107) = GPT 타이머 0번, 채널 A (variant 핀맵에 고정)
  ledTimer.begin(TIMER_MODE_PWM, GPT_TIMER, 0, 1000.0f, 0.0f);  // 1kHz, 듀티 0%
  ledTimer.add_pwm_extended_cfg();
  ledTimer.enable_pwm_channel(CHANNEL_A);   // open() 전에 호출해야 적용됨
  ledTimer.open();
  ledTimer.start();

  // 5번 핀을 GPIO에서 GPT 출력 기능으로 전환
  R_IOPORT_PinCfg(&g_ioport_ctrl, BSP_IO_PORT_01_PIN_07,
                  (uint32_t)(IOPORT_CFG_PERIPHERAL_PIN | IOPORT_PERIPHERAL_GPT1));
}

void loop() {
  // 서서히 밝게 (0% → 100%)
  for (int i = 0; i <= 100; i++) {
    setBrightness(i);
    delay(10);
  }
  // 서서히 어둡게 (100% → 0%)
  for (int i = 100; i >= 0; i--) {
    setBrightness(i);
    delay(10);
  }
}
