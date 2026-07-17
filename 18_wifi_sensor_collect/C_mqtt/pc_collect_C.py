"""
[방법 C] PC 수집기 — PC가 MQTT 브로커를 구독해 보드가 발행한 값을 받는다.
          받은 값을 CSV로 저장하고 실시간 그래프로 보여준다.

실행 전 준비:
    pip install paho-mqtt matplotlib
    보드 스케치와 TOPIC 을 동일하게 맞춘다.

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
PORT     = 1883
TOPIC    = "utech/uno_r4/adc"      # ★ 보드 스케치의 topic과 동일하게
CSV_PATH = "adc_log_C.csv"
WINDOW   = 100
# ────────────────────────────────────────────────────

buf = deque()   # 콜백(네트워크 스레드)이 받은 (adc, volt)을 잠시 담는 큐


def on_connect(client, userdata, flags, reason_code, *args):
    print("브로커 연결됨 (rc=%s), 구독: %s" % (reason_code, TOPIC))
    client.subscribe(TOPIC)


def on_message(client, userdata, msg):
    try:
        adc_s, volt_s = msg.payload.decode().strip().split(",")
        buf.append((int(adc_s), float(volt_s)))
    except ValueError:
        pass


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
(line,) = ax.plot([], [], marker=".")
ax.set_title("UNO R4 WiFi - A0 (Method C: MQTT)")
ax.set_xlabel("elapsed (s)")
ax.set_ylabel("ADC (0-4095)")
ax.set_ylim(0, 4095)

t0 = time.time()
with open(CSV_PATH, "w", newline="", encoding="utf-8") as f:
    w = csv.writer(f)
    w.writerow(["elapsed_s", "adc", "volt"])
    print(f"수집 시작 → {CSV_PATH}   (Ctrl+C로 종료)")
    try:
        while True:
            while buf:                       # 큐에 쌓인 값을 모두 처리
                adc, volt = buf.popleft()
                t = time.time() - t0
                w.writerow([f"{t:.2f}", adc, f"{volt:.3f}"])
                f.flush()
                xs.append(t)
                ys.append(adc)
                print(f"{t:6.1f}s  ADC={adc:4d}  V={volt:.3f}")
            line.set_data(xs, ys)
            ax.relim()
            ax.autoscale_view(scaley=False)
            plt.pause(0.2)
    except KeyboardInterrupt:
        print("\n종료. 저장 파일:", CSV_PATH)
    finally:
        client.loop_stop()
