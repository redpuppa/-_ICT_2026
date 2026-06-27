/*
  예제 10.2.1
  LED 매트릭스에 하트를 표시
*/
#include "Arduino_LED_Matrix.h"

ArduinoLEDMatrix inha;   // 내장 LED 매트릭스 객체

void setup() {
  inha.begin();          // 매트릭스 시작
}

void loop() {
  inha.loadFrame(LEDMATRIX_CHIP);
  delay(500);

  inha.loadFrame(LEDMATRIX_DANGER);
  delay(500);

  inha.loadFrame(LEDMATRIX_EMOJI_HAPPY);
  delay(500);

  inha.loadFrame(LEDMATRIX_HEART_BIG);
  delay(500);

  inha.clear();
  delay(1000);
}
