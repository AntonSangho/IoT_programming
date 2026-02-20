# 8교시 — 심화 자율 확장
**내가 만든 시스템에 기능을 추가해보자**

> 📌 **교사 참고:** 이 교시는 학생 자율 탐구 시간입니다.
> 정해진 순서 없이 학생 수준에 따라 아래 미션 카드 중 하나를 골라 진행합니다.
> 교사는 순회하며 힌트를 제공하는 역할을 합니다.

---

### 🎯 이 시간의 목표
- 지금까지 만든 시스템을 스스로 확장할 수 있다.
- 코드를 수정하고 결과를 확인하는 반복 과정을 경험한다.
- 오류가 생겼을 때 스스로 원인을 찾아 해결하려 시도한다.

### ⏱ 시간 계획 (50분)
| 시간 | 내용 |
|------|------|
| 0~5분 | 미션 카드 선택 |
| 5~40분 | 자율 확장 실습 |
| 40~50분 | 발표 및 공유 |

---

## 미션 카드 선택 안내

> 아래 3단계 중 본인의 수준에 맞는 카드를 **1개 이상** 골라서 도전하세요.
> 빨리 끝낸 학생은 다음 단계 카드에 도전합니다.

| 단계 | 대상 | 미션 |
|------|------|------|
| ⭐ 기본 | 7교시까지 잘 따라온 학생 | 웹 페이지 꾸미기, 통계 표시 |
| ⭐⭐ 도전 | 코드 구조가 이해되는 학생 | 경보 기능, 데이터 삭제 |
| ⭐⭐⭐ 심화 | 빠르게 이해한 학생 | 그래프, 시간대별 분석 |

---

## ⭐ 기본 미션

### 미션 1 — 통계 카드 추가

웹 페이지 상단에 평균 온도, 최고 온도, 최저 온도를 표시해 보세요.

**힌트 — app.py:**
```python
@app.route('/')
def index():
    conn   = get_connection()
    cursor = conn.cursor(pymysql.cursors.DictCursor)

    cursor.execute("SELECT * FROM sensor_data ORDER BY recorded_at DESC LIMIT 10")
    records = cursor.fetchall()

    # 아래 쿼리를 직접 작성해보세요
    cursor.execute("SELECT AVG(temperature) AS avg_temp, MAX(temperature) AS max_temp, MIN(temperature) AS min_temp FROM sensor_data")
    stats = cursor.fetchone()   # fetchall() 말고 fetchone() — 결과가 1행일 때

    cursor.close()
    conn.close()
    return render_template("index.html", records=records, stats=stats)
```

**힌트 — index.html:**
```html
<div>
    <span>평균: {{ stats.avg_temp | round(1) }}°C</span>
    <span>최고: {{ stats.max_temp }}°C</span>
    <span>최저: {{ stats.min_temp }}°C</span>
</div>
```

**도전 질문:** `| round(1)` 은 무엇을 하는 필터인가요?

```
답:
```

> ✅ **정답:** 소수점 첫째 자리까지 반올림한다. AVG()의 결과는 소수점이 매우 길게 나올 수 있기 때문에 보기 좋게 정리하는 데 쓴다.

---

### 미션 2 — 상태 색상 바꾸기

온도 상태(쾌적/더움/추움)에 따라 행 배경색이 달라지도록 수정해보세요.

**힌트 — index.html:**
```html
{% for row in records %}
<tr style="background-color:
    {% if row.temperature >= 28 %}
        #FFCCCC
    {% elif row.temperature <= 20 %}
        #CCE5FF
    {% else %}
        #CCFFCC
    {% endif %}
">
    <td>{{ row.temperature }}</td>
    ...
</tr>
{% endfor %}
```

**도전 질문:** `#FFCCCC`, `#CCE5FF`, `#CCFFCC` 는 각각 어떤 색인가요?

```
답:
```

> ✅ **정답:** 순서대로 연한 빨강(더움), 연한 파랑(추움), 연한 초록(쾌적)이다. HTML 색상 코드는 `#RRGGBB` 형식으로, 각각 빨강/초록/파랑의 강도를 16진수로 표현한다.

---

## ⭐⭐ 도전 미션

### 미션 3 — 온도 경보 기능

온도가 설정값을 넘으면 웹 페이지 상단에 경고 메시지를 표시해 보세요.

**app.py:**
```python
TEMP_LIMIT = 30   # 경보 기준 온도 (원하는 값으로 바꿔보세요)

@app.route('/')
def index():
    conn   = get_connection()
    cursor = conn.cursor(pymysql.cursors.DictCursor)
    cursor.execute("SELECT * FROM sensor_data ORDER BY recorded_at DESC LIMIT 10")
    records = cursor.fetchall()
    cursor.close()
    conn.close()

    # 가장 최근 온도가 기준을 넘었는지 확인
    alert = False
    if records and records[0]["temperature"] >= TEMP_LIMIT:
        alert = True

    return render_template("index.html", records=records, alert=alert, limit=TEMP_LIMIT)
```

**index.html:**
```html
{% if alert %}
<div style="background:#FF4444; color:white; padding:12px; font-size:1.2em;">
    ⚠️ 경보! 현재 온도가 {{ limit }}°C를 초과했습니다!
</div>
{% endif %}
```

**도전 질문:** `TEMP_LIMIT` 을 app.py 코드 안에 직접 쓰는 것과 변수로 따로 빼는 것의 차이는 무엇인가요?

```
답:
```

> ✅ **정답:** 변수로 빼두면 기준값을 바꿀 때 코드 한 곳만 수정하면 된다. 코드 여러 곳에 `30`이 직접 들어가 있으면 하나라도 빠뜨리면 버그가 생긴다. 이런 값을 "매직 넘버"라고 하며, 변수로 이름을 붙여두는 것이 좋은 습관이다.

---

### 미션 4 — 오래된 데이터 삭제

데이터가 너무 많이 쌓이면 Raspberry Pi가 느려질 수 있어요.
일정 개수 이상이면 오래된 데이터를 자동으로 삭제하는 기능을 추가해 보세요.

**app.py에 함수 추가:**
```python
MAX_RECORDS = 100   # 최대 보관 개수

def cleanup_old_records():
    conn   = get_connection()
    cursor = conn.cursor()

    # 전체 개수 확인
    cursor.execute("SELECT COUNT(*) FROM sensor_data")
    count = cursor.fetchone()[0]

    if count > MAX_RECORDS:
        # 오래된 것부터 초과분만큼 삭제
        delete_count = count - MAX_RECORDS
        cursor.execute(
            "DELETE FROM sensor_data ORDER BY recorded_at ASC LIMIT %s",
            (delete_count,)
        )
        conn.commit()
        print(f"{delete_count}개 삭제됨 (현재 {MAX_RECORDS}개 유지)")

    cursor.close()
    conn.close()
```

**save_to_db() 함수 마지막에 추가:**
```python
def save_to_db(temperature, humidity):
    ...
    conn.commit()
    cursor.close()
    conn.close()
    cleanup_old_records()   # 저장 후 정리
```

**도전 질문:** `DELETE FROM ... ORDER BY recorded_at ASC` 에서 `ASC` 를 `DESC` 로 바꾸면 어떻게 되나요?

```
답:
```

> ✅ **정답:** 오래된 데이터가 아닌 최신 데이터부터 삭제된다. 데이터 관리 목적과 반대가 되므로 반드시 `ASC`(오름차순, 오래된 순)를 써야 한다.

---

## ⭐⭐⭐ 심화 미션

### 미션 5 — Chart.js로 온도 그래프 그리기

저장된 데이터를 꺾은선 그래프로 시각화해 보세요.

**app.py에 API 라우트 추가:**
```python
from flask import jsonify

@app.route('/api/chart')
def chart_data():
    conn   = get_connection()
    cursor = conn.cursor(pymysql.cursors.DictCursor)
    cursor.execute(
        "SELECT temperature, humidity, recorded_at FROM sensor_data ORDER BY recorded_at ASC LIMIT 20"
    )
    rows = cursor.fetchall()
    cursor.close()
    conn.close()

    # JavaScript가 쓸 수 있는 형태로 변환
    labels = [str(row["recorded_at"]) for row in rows]
    temps  = [row["temperature"] for row in rows]
    hums   = [row["humidity"]    for row in rows]

    return jsonify({"labels": labels, "temperatures": temps, "humidities": hums})
```

**index.html — `</body>` 위에 추가:**
```html
<canvas id="myChart" width="800" height="300"></canvas>

<script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
<script>
fetch('/api/chart')
    .then(res => res.json())
    .then(data => {
        new Chart(document.getElementById('myChart'), {
            type: 'line',
            data: {
                labels: data.labels,
                datasets: [
                    {
                        label: '온도 (°C)',
                        data: data.temperatures,
                        borderColor: 'red',
                        fill: false
                    },
                    {
                        label: '습도 (%)',
                        data: data.humidities,
                        borderColor: 'blue',
                        fill: false
                    }
                ]
            }
        });
    });
</script>
```

**도전 질문:** `/api/chart` 라우트가 HTML이 아닌 JSON을 반환하는 이유는 무엇인가요?

```
답:
```

> ✅ **정답:** JavaScript의 `fetch()`는 HTML 페이지 전체가 아니라 데이터만 필요하다. JSON 형태로 데이터를 주면 JavaScript가 받아서 그래프를 그릴 수 있다. 이처럼 데이터를 제공하는 라우트를 API 라우트라고 하며, 1교시에서 배운 `jsonify()`가 쓰이는 대표적인 사례다.

---

### 미션 6 — 시간대별 평균 분석

시간대(0시~23시)별 평균 온도를 조회해서 "몇 시에 가장 더웠는지" 분석해 보세요.

**app.py에 라우트 추가:**
```python
@app.route('/analysis')
def analysis():
    conn   = get_connection()
    cursor = conn.cursor(pymysql.cursors.DictCursor)

    cursor.execute("""
        SELECT
            HOUR(recorded_at)    AS hour,
            AVG(temperature)     AS avg_temp,
            COUNT(*)             AS count
        FROM sensor_data
        GROUP BY HOUR(recorded_at)
        ORDER BY hour ASC
    """)
    rows = cursor.fetchall()
    cursor.close()
    conn.close()

    return render_template("analysis.html", hourly=rows)
```

**templates/analysis.html:**
```html
<!DOCTYPE html>
<html lang="ko">
<head><meta charset="UTF-8"><title>시간대별 분석</title></head>
<body>
    <h1>시간대별 평균 온도</h1>
    <table border="1">
        <tr><th>시간</th><th>평균 온도</th><th>측정 횟수</th></tr>
        {% for row in hourly %}
        <tr>
            <td>{{ row.hour }}시</td>
            <td>{{ row.avg_temp | round(1) }}°C</td>
            <td>{{ row.count }}회</td>
        </tr>
        {% endfor %}
    </table>
    <p><a href="/">← 돌아가기</a></p>
</body>
</html>
```

**도전 질문:** `GROUP BY HOUR(recorded_at)` 은 무엇을 하는 SQL인가요?

```
답:
```

> ✅ **정답:** `recorded_at`에서 시(hour) 부분만 추출해서 같은 시간대의 데이터를 하나로 묶는다. 예를 들어 9시에 수집된 데이터가 여러 개 있으면 모두 묶어서 AVG(), COUNT()를 계산한다.

---

## 발표 및 공유 (10분)

수업 마지막 10분, 각자 추가한 기능을 간단히 소개합니다.

**발표 내용 (1~2분):**

```
1. 어떤 미션을 선택했나요?
2. 어떤 부분이 제일 어려웠나요?
3. 어떻게 해결했나요?
4. 시간이 더 있다면 어떤 기능을 추가하고 싶나요?
```

---

## 전체 수업 마무리 — 우리가 만든 것

```
1교시  Flask return 이해
   ↓
2교시  Jinja2 템플릿으로 데이터 표시
   ↓
3교시  Arduino → pyserial → Flask 연결
   ↓
4교시  MariaDB 테이블 설계 (스스로!)
   ↓
5교시  SQL CRUD 직접 작성
   ↓
6교시  Flask ↔ MariaDB 연결
   ↓
7교시  전체 통합 (Arduino → DB → 웹)
   ↓
8교시  심화 확장 (그래프, 경보, 분석...)
```

> 여러분은 IoT 센서 데이터를 수집하고, 저장하고, 웹으로 보여주는
> **작은 IoT 대시보드 시스템**을 처음부터 끝까지 직접 만들었습니다. 🎉

---

## 📎 참고 자료

| 자료 | 링크 | 내용 |
|------|------|------|
| Chart.js 공식 문서 | https://www.chartjs.org/docs/latest/ | 그래프 종류 및 옵션 |
| Chart.js — Line Chart | https://www.chartjs.org/docs/latest/charts/line.html | 꺾은선 그래프 설정 |
| MySQL HOUR() 함수 | https://dev.mysql.com/doc/refman/8.0/en/date-and-time-functions.html#function_hour | 날짜/시간 함수 목록 |
| JavaScript fetch API (MDN) | https://developer.mozilla.org/ko/docs/Web/API/Fetch_API/Using_Fetch | fetch() 사용법 |
| Jinja2 — 내장 필터 | https://jinja.palletsprojects.com/en/stable/templates/#builtin-filters | round, length 등 |
| Flask jsonify | https://flask.palletsprojects.com/en/stable/api/#flask.json.jsonify | API 라우트 만들기 |
