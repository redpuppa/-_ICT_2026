/*
  예제 20. 초음파 거리계 + 4자리 FND 표시 (FspTimer 주기 인터럽트)
  - 거리 측정(pulseIn)은 블로킹이라 loop()에서 수행
  - FND 멀티플렉싱은 일정 주기가 필요하므로 타이머 인터럽트에서 수행
    → loop가 측정으로 바빠도 FND가 깜빡이지 않는다
  - 표시 형식: "100.0" (XXX.X, 0.1cm 단위)
  참조: ex_9_5(초음파), 10_fnd_digit4_CA(FND), 19_timer_interrupt_fsptimer(타이머)
*/
#include "FspTimer.h"

// ===== 초음파 센서 (HC-SR04) =====
const int trigPin = 13;
const int echoPin = 12;

// ===== 4자리 공통 애노드 FND =====
                          // a, b, c, d, e, f, g, dp
uint8_t fnd_data_pin[8] = { 4, 5, 6, 7, 8, 9, 10, 11};
uint8_t fnd_data[10]    = {0x3F, 0x06, 0x5B, 0x4F, 0x66,
                           0x6D, 0x7D, 0x27, 0x7F, 0x6F};
                          // D1, D2, D3, D4
uint8_t fnd_digit_pin[4] = {14, 15, 16, 17};

// ===== 타이머 =====
FspTimer fndTimer;

// loop()가 쓰고 ISR이 읽는 표시값 (0.1cm 단위, 예: 1000 = 100.0cm)
// 32비트 정렬 int는 M4에서 읽기/쓰기가 원자적이라 별도 보호 불필요
volatile int dispValue = 0;

// 한 자리에 세그먼트 패턴 출력 (공통 애노드: 세그먼트 핀 LOW일 때 켜짐)
void fndDisplay(uint8_t num, bool dp) {
  uint8_t pattern = fnd_data[num];
  if (dp) pattern |= 0x80;                 // 소수점(bit7) 켜기
  for (int i = 0; i < 8; i++) {
    digitalWrite(fnd_data_pin[i], !bitRead(pattern, i));
  }
}

// 타이머 인터럽트: 한 번 호출될 때마다 한 자리씩 스캔 (멀티플렉싱)
void onFndScan(timer_callback_args_t *args) {
  (void)args;
  static uint8_t cur = 0;          // 현재 스캔 중인 자리 (0~3)

  int v = dispValue;
  int digit;
  switch (cur) {
    case 0:  digit = (v / 1000) % 10; break;  // 백의 자리
    case 1:  digit = (v / 100)  % 10; break;  // 십의 자리
    case 2:  digit = (v / 10)   % 10; break;  // 일의 자리 (소수점 위치)
    default: digit =  v         % 10; break;  // 소수 첫째 자리
  }

  // 잔상(ghosting) 방지: 세그먼트를 바꾸기 전에 모든 자리를 먼저 끈다
  for (int j = 0; j < 4; j++) digitalWrite(fnd_digit_pin[j], LOW);

  fndDisplay(digit, (cur == 2));   // 세 번째 자리에만 소수점 표시
  digitalWrite(fnd_digit_pin[cur], HIGH);   // 현재 자리만 켜기

  cur = (cur + 1) & 0x03;          // 다음 자리로 (0→1→2→3→0)
}

void setup() {
  Serial.begin(9600);

  // 초음파 핀 설정
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  digitalWrite(trigPin, LOW);

  // FND 핀 설정
  for (int i = 0; i < 8; i++) {
    pinMode(fnd_data_pin[i], OUTPUT);
    digitalWrite(fnd_data_pin[i], HIGH);   // 세그먼트 OFF
  }
  for (int i = 0; i < 4; i++) {
    pinMode(fnd_digit_pin[i], OUTPUT);
    digitalWrite(fnd_digit_pin[i], LOW);   // 자리 OFF
  }

  // 타이머 확보 후 500Hz 주기 인터럽트 설정
  // 자리당 500Hz → 전체 프레임 125Hz (깜빡임 없음)
  uint8_t type;
  int8_t ch = FspTimer::get_available_timer(type);
  if (ch < 0) ch = FspTimer::get_available_timer(type, true);

  fndTimer.begin(TIMER_MODE_PERIODIC, type, ch, 500.0f, 0.0f, onFndScan);
  fndTimer.setup_overflow_irq();   // open() 전에 호출
  fndTimer.open();
  fndTimer.start();
}

void loop() {
  // 10us 트리거 펄스 송출
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Echo 펄스 폭 측정 → 거리 계산
  long pulseWidth = pulseIn(echoPin, HIGH);
  float distance = pulseWidth / 58.0;      // cm 단위

  // 감지 범위 2~200cm의 값만 표시 (원본의 || 오류를 && 로 수정)
  if (distance >= 2 && distance <= 200) {
    dispValue = (int)(distance * 10 + 0.5);  // 0.1cm 단위로 반올림
    Serial.print(distance, 1);
    Serial.println(" cm");
  }

  delay(100);
}
