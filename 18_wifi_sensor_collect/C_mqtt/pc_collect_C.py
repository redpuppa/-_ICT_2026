"""
[방법 C] PC 수집기 — 보드와 PC가 MQTT 브로커를 통해 서로 발행/구독한다.
    - 구독: 보드가 발행하는 센서값(.../data)              → CSV 저장 + 실시간 그래프
    - 구독: 보드가 발행하는 LED 적용 상태(.../led/state)   → 콘솔에 표시
    - 발행: 그래프 창에서 키보드로 LED 켜기/끄기 명령(.../led/set) → 보드가 구독해서 반영

  그래프 창에 포커스를 둔 상태에서:
    o 키 → LED ON  명령 발행
    f 키 → LED OFF 명령 발행

실행 전 준비:
    pip install paho-mqtt matplotlib
    보드 스케치와 BASE_TOPIC 을 동일하게 맞춘다.

실행:
    python pc_collect_C.py
종료: Ctrl+C

참고: paho-mqtt 2.x / 1.x 모두 동작하도록 작성했습니다.
"""
import csv
import time
from collections import deque

import matplotlib.pyplot as plt
import paho.mqtt.client as mqtt

# ── 설정 ─────────────────────────────────────────────
# [방식 1] 공개 브로커
BROKER   = "test.mosquitto.org"
# [방식 2] 로컬 브로커(WSL): 이 수집기를 Windows에서 실행하면 보통 "localhost"
#          (미러 네트워킹) 또는 포트프록시 설정 시에도 "localhost". 자세한 건 README 2.4.
# BROKER   = "localhost"
PORT       = 1883
BASE_TOPIC = "ict2026/uno_r4/mhlee"     # ★ 보드 스케치의 BASE_TOPIC과 동일하게

DATA_TOPIC      = BASE_TOPIC + "/data"        # 구독(보드→PC): "adc,volt"
LED_SET_TOPIC   = BASE_TOPIC + "/led/set"     # 발행(PC→보드): "1"(ON) / "0"(OFF)
LED_STATE_TOPIC = BASE_TOPIC + "/led/state"   # 구독(보드→PC): 적용된 LED 상태

CSV_PATH = "adc_log_C.csv"
WINDOW   = 100
# ────────────────────────────────────────────────────

data_buf = deque()   # 콜백(네트워크 스레드)이 받은 (adc, volt)을 잠시 담는 큐
led_buf  = deque()   # 콜백이 받은 LED 상태("1"/"0")를 잠시 담는 큐


def on_connect(client, userdata, flags, reason_code, *args):
    print("브로커 연결됨 (rc=%s)" % reason_code)
    client.subscribe(DATA_TOPIC)
    client.subscribe(LED_STATE_TOPIC)
    print("구독:", DATA_TOPIC, "/", LED_STATE_TOPIC)


def on_message(client, userdata, msg):
    if msg.topic == DATA_TOPIC:
        try:
            adc_s, volt_s = msg.payload.decode().strip().split(",")
            data_buf.append((int(adc_s), float(volt_s)))
        except ValueError:
            pass
    elif msg.topic == LED_STATE_TOPIC:
        led_buf.append(msg.payload.decode().strip())


def set_led(client, on: bool):
    """LED 켜기/끄기 명령을 발행한다 (보드가 LED_SET_TOPIC을 구독 중)."""
    client.publish(LED_SET_TOPIC, "1" if on else "0")
    print(f"발행[led/set]: {'1 (ON)' if on else '0 (OFF)'}")


def on_key(event):
    if event.key == "o":
        set_led(client, True)
    elif event.key == "f":
        set_led(client, False)


# paho-mqtt 2.x는 CallbackAPIVersion 지정을 요구, 1.x는 인자 없이 생성
try:
    client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
except (AttributeError, TypeError):
    client = mqtt.Client()

client.on_connect = on_connect
client.on_message = on_message
client.connect(BROKER, PORT, 60)
client.loop_start()                # 네트워크 처리는 백그라운드 스레드에서

xs, ys = deque(maxlen=WINDOW), deque(maxlen=WINDOW)
plt.ion()
fig, ax = plt.subplots()
fig.canvas.mpl_connect("key_press_event", on_key)
(line,) = ax.plot([], [], marker=".")
ax.set_title("UNO R4 WiFi - A0 (Method C: MQTT)  [o]=LED ON  [f]=LED OFF")
ax.set_xlabel("elapsed (s)")
ax.set_ylabel("ADC (0-4095)")
ax.set_ylim(0, 4095)

t0 = time.time()
with open(CSV_PATH, "w", newline="", encoding="utf-8") as f:
    w = csv.writer(f)
    w.writerow(["elapsed_s", "adc", "volt"])
    print(f"수집 시작 → {CSV_PATH}   (그래프 창에서 o/f 키로 LED 제어, Ctrl+C로 종료)")
    try:
        while True:
            while data_buf:                  # 큐에 쌓인 센서값을 모두 처리
                adc, volt = data_buf.popleft()
                t = time.time() - t0
                w.writerow([f"{t:.2f}", adc, f"{volt:.3f}"])
                f.flush()
                xs.append(t)
                ys.append(adc)
                print(f"{t:6.1f}s  ADC={adc:4d}  V={volt:.3f}")
            while led_buf:                    # 큐에 쌓인 LED 상태를 모두 처리
                state = led_buf.popleft()
                print(f"         LED 상태(보드 확인): {'ON' if state == '1' else 'OFF'}")
            line.set_data(xs, ys)
            ax.relim()
            ax.autoscale_view(scaley=False)
            plt.pause(0.2)
    except KeyboardInterrupt:
        print("\n종료. 저장 파일:", CSV_PATH)
    finally:
        client.loop_stop()
