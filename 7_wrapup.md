# 7교시 — 전체 통합
**Arduino → pyserial → Flask → MariaDB 저장 → 웹 표시 완성**

> 📌 **교사 참고:** 이 문서는 정답 포함 교사용입니다. 학생 배포 시 `✅ 정답` 항목을 제거하세요.
> 이 교시는 1~6교시의 모든 내용이 합쳐지는 교시입니다. 학생이 스스로 연결하는 경험을 최대한 보장하세요.

---

### 🎯 학습 목표
- 1~6교시에서 만든 코드 조각들을 하나의 프로젝트로 통합할 수 있다.
- Arduino 센서 데이터가 DB에 저장되고 웹에 표시되는 전체 흐름을 설명할 수 있다.
- 오류가 생겼을 때 어느 단계에서 문제인지 스스로 찾을 수 있다.

### ⏱ 시간 계획 (50분)
| 시간 | 내용 |
|------|------|
| 0~5분 | 전체 흐름 다시 그려보기 |
| 5~20분 | 실습 1 — 코드 통합 |
| 20~35분 | 실습 2 — 자동 저장 구조 완성 |
| 35~45분 | 실습 3 — 웹 페이지 다듬기 |
| 45~50분 | 정리 및 트러블슈팅 |

---

## 1. 전체 흐름 다시 그려보기 (5분)

**학생들에게 빈 종이를 주고 스스로 흐름도를 그리게 합니다.**

> 지금까지 배운 것들이 어떻게 연결되나요? 화살표로 그려보세요.

정답 흐름도:

```
[Arduino + DHT11]
      |
      | Serial.println("온도,습도")
      ↓
[Python - pyserial]
      |
      | readline() → decode() → split()
      ↓
[Flask - app.py]
      |
      ├─→ INSERT → [MariaDB - sensor_data 테이블]
      |                        |
      |              SELECT ←──┘
      ↓
[templates/index.html - Jinja2]
      |
      ↓
[브라우저 - 웹 페이지]
```

---

## 2. 실습 1 — 코드 통합 (15분)

### 프로젝트 폴더 구조

```
sensor_project/
├── app.py
└── templates/
    └── index.html
```

### app.py — 완성본

지금까지 만든 코드를 하나로 합칩니다.

```python
import serial
import time
import pymysql
from flask import Flask, render_template

app = Flask(__name__)

# ── MariaDB 연결 ────────────────────────────
def get_connection():
    return pymysql.connect(
        host="localhost",
        user="pi",
        password="비밀번호",
        database="sensor_db",
        charset="utf8mb4"
    )

# ── Arduino 데이터 읽기 ─────────────────────
def read_sensor():
    try:
        ser = serial.Serial("COM3", 9600, timeout=2)
        time.sleep(2)
        line = ser.readline().decode("utf-8").strip()
        ser.close()
        parts = line.split(",")
        return {
            "temperature": float(parts[0]),
            "humidity":    float(parts[1])
        }
    except Exception as e:
        print("센서 오류:", e)
        return None

# ── 데이터 저장 ─────────────────────────────
def save_to_db(temperature, humidity):
    conn   = get_connection()
    cursor = conn.cursor()
    sql    = "INSERT INTO sensor_data (temperature, humidity) VALUES (%s, %s)"
    cursor.execute(sql, (temperature, humidity))
    conn.commit()
    cursor.close()
    conn.close()

# ── 데이터 조회 ─────────────────────────────
def get_records(limit=10):
    conn   = get_connection()
    cursor = conn.cursor(pymysql.cursors.DictCursor)
    cursor.execute(
        "SELECT * FROM sensor_data ORDER BY recorded_at DESC LIMIT %s",
        (limit,)
    )
    rows = cursor.fetchall()
    cursor.close()
    conn.close()
    return rows

# ── 라우트 ──────────────────────────────────
@app.route('/')
def index():
    records = get_records()
    return render_template("index.html", records=records)

@app.route('/collect')
def collect():
    data = read_sensor()
    if data:
        save_to_db(data["temperature"], data["humidity"])
        return f"저장 완료: 온도 {data['temperature']}°C, 습도 {data['humidity']}%"
    else:
        return "센서 데이터를 읽을 수 없습니다.", 500

if __name__ == '__main__':
    app.run(debug=True)
```

### templates/index.html — 완성본

```html
<!DOCTYPE html>
<html lang="ko">
<head>
    <meta charset="UTF-8">
    <title>센서 대시보드</title>
    <meta http-equiv="refresh" content="10">
    <style>
        body  { font-family: sans-serif; margin: 40px; }
        table { border-collapse: collapse; width: 100%; }
        th, td { border: 1px solid #ccc; padding: 8px 12px; text-align: center; }
        th    { background-color: #4472C4; color: white; }
        tr:nth-child(even) { background-color: #f2f2f2; }
    </style>
</head>
<body>
    <h1>🌡️ DHT11 센서 대시보드</h1>
    <p><a href="/collect">📥 지금 데이터 수집</a></p>

    {% if records %}
    <table>
        <tr>
            <th>ID</th>
            <th>온도 (°C)</th>
            <th>습도 (%)</th>
            <th>상태</th>
            <th>측정 시각</th>
        </tr>
        {% for row in records %}
        <tr>
            <td>{{ row.id }}</td>
            <td>{{ row.temperature }}</td>
            <td>{{ row.humidity }}</td>
            <td>
                {% if row.temperature >= 28 %}
                    🔴 더움
                {% elif row.temperature <= 20 %}
                    🔵 추움
                {% else %}
                    🟢 쾌적
                {% endif %}
            </td>
            <td>{{ row.recorded_at }}</td>
        </tr>
        {% endfor %}
    </table>
    {% else %}
        <p>저장된 데이터가 없습니다. 위 링크를 눌러 데이터를 수집하세요.</p>
    {% endif %}
</body>
</html>
```

---

**Q1.** `app.py`에서 함수를 `read_sensor()`, `save_to_db()`, `get_records()` 로 분리한 이유는 무엇인가요?

```
답:
```

> ✅ **정답:** 각 기능을 독립된 함수로 분리하면 코드를 읽기 쉽고 수정하기 편하다. 예를 들어 포트 번호만 바꾸고 싶으면 `read_sensor()` 안만 수정하면 되고, DB 비밀번호를 바꾸려면 `get_connection()` 만 수정하면 된다. 또한 같은 함수를 여러 라우트에서 재사용할 수 있다.

&nbsp;

**Q2.** `/collect` 라우트에서 센서 오류 시 `return "...", 500` 을 반환합니다. 숫자 `500` 은 무엇인가요?

```
답:
```

> ✅ **정답:** HTTP 상태 코드로, 500은 "서버 내부 오류(Internal Server Error)"를 의미한다. 브라우저나 다른 프로그램이 요청 결과가 성공인지 실패인지 판단할 때 사용한다. 정상은 200, 오류는 500이다.

---

## 3. 실습 2 — 자동 저장 구조 완성 (15분)

지금은 `/collect` 를 브라우저에서 직접 눌러야 데이터가 저장됩니다.
Flask 서버가 켜지면 **백그라운드에서 주기적으로 자동 저장**되도록 바꿔봅니다.

### app.py에 추가

```python
import threading   # 파일 맨 위 import에 추가

def auto_collect(interval=10):
    """interval초마다 센서 데이터를 자동으로 수집·저장"""
    while True:
        data = read_sensor()
        if data:
            save_to_db(data["temperature"], data["humidity"])
            print(f"저장됨: {data['temperature']}°C, {data['humidity']}%")
        time.sleep(interval)

# if __name__ == '__main__': 바로 위에 추가
thread = threading.Thread(target=auto_collect, args=(10,), daemon=True)
thread.start()

if __name__ == '__main__':
    app.run(host="0.0.0.0", debug=True, use_reloader=False)   # use_reloader=False 주의!
```

**Q3.** `daemon=True` 는 무엇을 의미하나요?

```
답:
```

> ✅ **정답:** 메인 프로그램(Flask 서버)이 종료되면 이 스레드도 함께 자동으로 종료된다는 뜻이다. `daemon=True` 가 없으면 Flask를 Ctrl+C로 종료해도 백그라운드 스레드가 계속 실행된다.

&nbsp;

**Q4.** `use_reloader=False` 를 쓰는 이유는 무엇인가요?

```
답:
```

> ✅ **정답:** Flask의 `debug=True` 모드에서는 코드 변경을 감지하면 서버를 자동으로 재시작(reloader)하는데, 이때 스레드가 중복으로 실행된다. `use_reloader=False`를 쓰면 자동 재시작을 끄고 스레드가 한 번만 실행되도록 한다.

&nbsp;

**Q5.** `interval=10` 을 5로 줄이면 어떤 변화가 생기나요? 실제로 바꿔서 확인해보세요.

```
답:
```

> ✅ **정답:** 5초마다 데이터가 저장되므로 DB에 더 빠르게 데이터가 쌓인다. 단, Arduino 측 `delay(2000)` 보다 짧게 설정하면 센서가 미처 새 데이터를 보내기 전에 Python이 읽으려 해서 이전 데이터나 오류가 생길 수 있다.

---

## 4. 실습 3 — 웹 페이지 다듬기 (10분)

데이터가 잘 쌓이는 것을 확인했으면 웹 페이지를 조금 더 유용하게 만들어봅니다.

### templates/index.html에 통계 추가

`<table>` 위에 아래 내용을 추가하세요:

```html
{% if records %}
<div style="display:flex; gap:40px; margin-bottom:20px;">
    <div>
        <strong>총 측정 횟수</strong><br>
        {{ records | length }} 회
    </div>
    <div>
        <strong>최근 온도</strong><br>
        {{ records[0].temperature }} °C
    </div>
    <div>
        <strong>최근 습도</strong><br>
        {{ records[0].humidity }} %
    </div>
</div>
{% endif %}
```

**Q6.** `records | length` 에서 `|` 는 Jinja2에서 무엇을 의미하나요?

```
답:
```

> ✅ **정답:** Jinja2의 필터(filter)를 적용하는 기호다. `records | length`는 `records` 리스트에 `length` 필터를 적용해서 리스트의 길이(개수)를 반환한다. Python의 `len(records)`와 같다.

&nbsp;

**Q7.** `records[0]` 이 가장 최근 데이터인 이유는 무엇인가요?

```
답:
```

> ✅ **정답:** `get_records()` 함수에서 `ORDER BY recorded_at DESC` 로 최신순 정렬했기 때문에 리스트의 첫 번째 항목(`[0]`)이 가장 최근 데이터이다.

---

## 5. 정리 및 트러블슈팅 (5분)

### 전체 흐름 최종 정리

```
Arduino (DHT11)
    ↓ Serial.println("온도,습도")
pyserial (read_sensor)
    ↓ readline → decode → split → float
save_to_db()
    ↓ INSERT INTO sensor_data
MariaDB (sensor_data 테이블)
    ↓ SELECT * ORDER BY DESC
get_records()
    ↓ fetchall → 딕셔너리 리스트
render_template("index.html", records=rows)
    ↓ Jinja2 {% for %}
브라우저 (웹 페이지)
```

### 자주 발생하는 오류

| 단계 | 오류 | 해결 |
|------|------|------|
| Serial | `SerialException` | 포트 번호 확인, Arduino IDE Serial Monitor 닫기 |
| 파싱 | `ValueError` | `serial_test.py` 로 raw 데이터 먼저 확인 |
| MariaDB 연결 | `Access denied` | user/password/database 이름 확인 |
| MariaDB 저장 | 데이터가 안 쌓임 | `conn.commit()` 확인 |
| 웹 표시 | 빈 페이지 | `get_records()` 리턴값 print로 확인 |

### 최종 확인 질문

**Q8.** 오늘 완성한 시스템에서 Flask 서버를 끄면 어떤 데이터가 사라지고 어떤 데이터가 남나요?

```
답:
```

> ✅ **정답:** Flask 서버를 껴도 MariaDB에 이미 저장된 데이터는 그대로 남는다. 사라지는 것은 메모리에만 있는 것들(예: Python 변수)이다. 이것이 3교시에서 "왜 DB가 필요한가"에 대한 실질적인 답이다.

---

## 📎 참고 자료

| 자료 | 링크 | 내용 |
|------|------|------|
| Python threading 공식 문서 | https://docs.python.org/ko/3/library/threading.html | Thread, daemon 개념 |
| Flask — 실행 옵션 | https://flask.palletsprojects.com/en/stable/api/#flask.Flask.run | use_reloader, debug 옵션 |
| Jinja2 — 필터 목록 | https://jinja.palletsprojects.com/en/stable/templates/#builtin-filters | length, round, upper 등 |
| HTTP 상태 코드 (MDN) | https://developer.mozilla.org/ko/docs/Web/HTTP/Status | 200, 404, 500 의미 |
| pyserial 공식 문서 | https://pyserial.readthedocs.io/en/latest/ | Serial 연결 전체 API |

---

> 💡 **다음 시간 예고:** 8교시는 자유 심화 시간입니다. 오늘 만든 시스템을 바탕으로 본인이 원하는 기능을 추가해봅니다.
