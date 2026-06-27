/*
  예제 10.2.3
  LED 매트릭스에 문자열 스크롤 하기
*/

#include "ArduinoGraphics.h"      // 텍스트/도형 그리기 기능 (beginText, stroke 등)
#include "Arduino_LED_Matrix.h"   // UNO R4 내장 12x8 LED 매트릭스 제어

ArduinoLEDMatrix inha;            // 내장 LED 매트릭스 객체 (이름은 자유롭게 지정)

void setup() {
  inha.begin();                   // 매트릭스 초기화

  inha.beginDraw();               // 그리기 시작 (이후 그리기 명령을 묶음)
  inha.stroke(0xFFFFFFFF);        // 픽셀 색상(밝기) 설정 — 흰색(켜짐)

  // ----- 정지된 텍스트 표시 -----
  const char text[] = "UNO R4";   // 출력할 문자열
  inha.textFont(Font_4x6);        // 작은 폰트(4x6) 선택 — 한 화면에 더 많이 들어감
  inha.beginText(0, 1, 0xFFFFFF); // 텍스트 시작: x=0, y=1 위치, 글자색 흰색
  inha.println(text);             // 문자열을 그리기 버퍼에 출력
  inha.endText();                 // 텍스트 출력 종료 (스크롤 없이 정지 표시)

  inha.endDraw();                 // 그리기 종료 → 매트릭스에 실제 반영

  delay(2000);                    // 2초간 정지 화면 유지
}

void loop() {
  // ----- 스크롤 효과 시작 -----
  inha.beginDraw();               // 그리기 시작

  inha.stroke(0xFFFFFFFF);        // 픽셀 색상(밝기) — 흰색
  inha.textScrollSpeed(50);       // 스크롤 속도 설정 (값이 작을수록 빠름, 단위 ms)

  const char text[] = "  Hello World..!!  ";   // 흐르게 할 문자열
  inha.textFont(Font_5x7);        // 큰 폰트(5x7) 선택 — 정지 화면보다 또렷
  inha.beginText(0, 1, 0xFFFFFF); // 텍스트 시작 위치/색상 지정
  inha.println(text);             // 문자열을 버퍼에 출력

  // SCROLL_LEFT를 사용하여 왼쪽으로 흐르게 설정
  inha.endText(SCROLL_LEFT);      // 텍스트 종료 + 왼쪽 스크롤 실행

  inha.endDraw();                 // 그리기 종료 → 매트릭스에 반영
}