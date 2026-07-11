#include "FspTimer.h"

FspTimer servoTimer;

void setup() {
  // D5(P107) = GPT 타이머 0번, 채널 A (variant 핀맵에 고정)
  servoTimer.begin(TIMER_MODE_PWM, GPT_TIMER, 0, 1000.0f, 0.0f);  // 50Hz, 1.5ms
  servoTimer.add_pwm_extended_cfg();
  servoTimer.enable_pwm_channel(CHANNEL_A);   // open() 전에 호출해야 적용됨
  servoTimer.open();
  servoTimer.start();

  // 9번 핀을 GPIO에서 GPT 출력 기능으로 전환
  R_IOPORT_PinCfg(&g_ioport_ctrl, BSP_IO_PORT_01_PIN_07,
                  (uint32_t)(IOPORT_CFG_PERIPHERAL_PIN | IOPORT_PERIPHERAL_GPT1));
}

void writeMicroseconds(uint32_t us) {
  servoTimer.set_pulse_us((double)us, CHANNEL_A);
}

void loop() {
  for(int i=0; i<100; i++){
    writeMicroseconds((1000/100)*i); 
    delay(10);
  }
}
