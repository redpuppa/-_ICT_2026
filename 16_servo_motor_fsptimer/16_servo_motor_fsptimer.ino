#include "FspTimer.h"

FspTimer servoTimer;

void setup() {
  // D9(P303) = GPT 타이머 7번, 채널 B (variant 핀맵에 고정)
  servoTimer.begin(TIMER_MODE_PWM, GPT_TIMER, 7, 50.0f, 7.5f);  // 50Hz, 1.5ms
  servoTimer.add_pwm_extended_cfg();
  servoTimer.enable_pwm_channel(CHANNEL_B);   // open() 전에 호출해야 적용됨
  servoTimer.open();
  servoTimer.start();

  // 9번 핀을 GPIO에서 GPT 출력 기능으로 전환
  R_IOPORT_PinCfg(&g_ioport_ctrl, BSP_IO_PORT_03_PIN_03,
                  (uint32_t)(IOPORT_CFG_PERIPHERAL_PIN | IOPORT_PERIPHERAL_GPT1));
}

void writeMicroseconds(uint32_t us) {
  servoTimer.set_pulse_us((double)us, CHANNEL_B);
}

void loop() {
  writeMicroseconds(1000); delay(1000);
  writeMicroseconds(1500); delay(1000);
  writeMicroseconds(2000); delay(1000);
}
