"""
[방법 A] PC 수집기 — 보드(웹서버)에 주기적으로 HTTP 요청해 값을 받는다.
          받은 값을 CSV로 저장하고 실시간 그래프로 보여준다.

실행 전 준비:
    pip install requests matplotlib
    아래 BOARD_URL 을 보드 시리얼 모니터에 찍힌 IP로 바꾼다.

실행:
    python pc_collect_A.py
종료: 그래프 창에서 Ctrl+C (또는 창 닫기)
"""
import csv
import time
from collections import deque

import requests
import matplotlib.pyplot as plt

# ── 설정 ─────────────────────────────────────────────
BOARD_URL = "http://192.168.0.50"   # ★ 보드 IP로 변경
INTERVAL  = 1.0                     # 수집 주기(초)
CSV_PATH  = "adc_log_A.csv"         # 저장 파일
WINDOW    = 100                     # 그래프에 표시할 최근 표본 수
# ────────────────────────────────────────────────────

xs, ys = deque(maxlen=WINDOW), deque(maxlen=WINDOW)
plt.ion()
fig, ax = plt.subplots()
(line,) = ax.plot([], [], marker=".")
ax.set_title("UNO R4 WiFi - A0 (Method A: Web Server)")
ax.set_xlabel("elapsed (s)")
ax.set_ylabel("ADC (0-4095)")
ax.set_ylim(0, 4095)

t0 = time.time()
with open(CSV_PATH, "w", newline="", encoding="utf-8") as f:
    w = csv.writer(f)
    w.writerow(["elapsed_s", "adc", "volt"])
    print(f"수집 시작 → {CSV_PATH}   (창을 닫거나 Ctrl+C로 종료)")
    try:
        while True:
            try:
                text = requests.get(BOARD_URL, timeout=2).text.strip()
                adc_s, volt_s = text.split(",")
                adc, volt = int(adc_s), float(volt_s)
            except Exception as e:
                print("요청 실패:", e)
                time.sleep(INTERVAL)
                continue

            t = time.time() - t0
            w.writerow([f"{t:.2f}", adc, f"{volt:.3f}"])
            f.flush()

            xs.append(t)
            ys.append(adc)
            line.set_data(xs, ys)
            ax.relim()
            ax.autoscale_view(scaley=False)
            print(f"{t:6.1f}s  ADC={adc:4d}  V={volt:.3f}")
            plt.pause(INTERVAL)
    except KeyboardInterrupt:
        print("\n종료. 저장 파일:", CSV_PATH)
