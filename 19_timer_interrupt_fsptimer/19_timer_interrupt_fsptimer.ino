/*
  예제 19. FspTimer를 이용한 주기 인터럽트
  - 타이머를 PERIODIC 모드로 열고 콜백을 등록하면
    설정한 주기마다 하드웨어가 콜백(ISR)을 자동 호출한다.
  - loop()가 무엇을 하든 정확한 주기로 실행된다. (핀 불필요)
  - 이 예제: 0.5초마다 인터럽트 → 내장 LED 토글(1Hz 점멸) + 초 카운트
*/
#include "FspTimer.h"

FspTimer intervalTimer;

// ISR과 loop()가 함께 쓰는 변수는 반드시 volatile
volatile uint32_t tickCount = 0;
volatile bool tickFlag = false;

// 주기(0.5초)마다 하드웨어가 직접 호출하는 콜백 (인터럽트 컨텍스트)
void onTimerTick(timer_callback_args_t *args) {
  (void)args;                                  // 이번 예제에선 인자 미사용
  tickCount++;
  digitalWrite(LED_BUILTIN, tickCount & 1);    // 호출마다 LED 토글
  tickFlag = true;                             // 무거운 작업은 loop()에 위임
}

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);

  // 비어 있는 타이머를 아무거나 확보 (인터럽트는 핀과 무관)
  uint8_t type;
  int8_t ch = FspTimer::get_available_timer(type);
  if (ch < 0) {
    // 전부 사용 중이면 PWM 예약 채널까지 강제로 동원
    ch = FspTimer::get_available_timer(type, true);
  }

  // 2Hz = 0.5초 주기. PERIODIC 모드라 듀티(다섯 번째 인자)는 의미 없음.
  // 여섯 번째 인자로 콜백 함수를 넘긴다.
  intervalTimer.begin(TIMER_MODE_PERIODIC, type, ch, 2.0f, 0.0f, onTimerTick);
  intervalTimer.setup_overflow_irq();   // 오버플로 인터럽트를 NVIC에 등록 (open 전에!)
  intervalTimer.open();
  intervalTimer.start();

  Serial.println("타이머 인터럽트 시작 (0.5초 주기)");
}

void loop() {
  // ISR이 세워준 플래그를 loop에서 처리 (Serial 출력은 ISR에서 하면 안 됨)
  if (tickFlag) {
    tickFlag = false;
    Serial.print("tick #");
    Serial.print(tickCount);
    Serial.print("  경과: ");
    Serial.print(tickCount * 0.5, 1);
    Serial.println("초");
  }

  // loop는 다른 일을 자유롭게 해도 LED 점멸 주기는 흔들리지 않는다
}
