# 1교시 — Flask `return`이 하는 일
**Flask가 브라우저에게 무엇을 돌려주는가**

> 📌 **교사 참고:** 이 문서는 정답 포함 교사용입니다. 학생 배포 시 `✅ 정답` 항목을 제거하세요.

---

### 🎯 학습 목표
- `return`이 브라우저에게 응답을 보내는 역할임을 이해한다.
- `return "문자열"`, `return render_template()`, `return jsonify()` 세 가지 차이를 설명할 수 있다.
- 상황에 따라 적절한 return 방식을 선택할 수 있다.

### ⏱ 시간 계획 (50분)
| 시간 | 내용 |
|------|------|
| 0~5분 | 도입 — 브라우저가 URL을 치면 무슨 일이? |
| 5~15분 | 개념 — Flask return의 역할 |
| 15~25분 | 실습 1 — 문자열 vs HTML |
| 25~35분 | 실습 2 — render_template으로 데이터 전달 |
| 35~45분 | 실습 3 — jsonify로 JSON 반환 |
| 45~50분 | 정리 — 언제 뭘 써야 하나? |

---

## 1. 도입 (5분)

**칠판에 질문 적기:**

> 브라우저에서 `http://127.0.0.1:5000/` 을 치면 무슨 일이 벌어질까요?

학생 답변을 들은 뒤 이 그림으로 정리:

```
브라우저          Flask 서버
  |                  |
  |-- 요청 (Request) -->|
  |                  | ← 여기서 함수가 실행됨
  |<- 응답 (Response)--|
  |                  |
  화면에 표시
```

**핵심 한 마디:**
> `return`이 바로 Flask가 브라우저에게 돌려주는 **응답**입니다.

---

## 2. 개념 — return의 세 가지 형태

```python
from flask import Flask, render_template, jsonify

app = Flask(__name__)

# 형태 1: 문자열 반환
@app.route('/a')
def route_a():
    return "Hello, Flask!"

# 형태 2: HTML 템플릿 반환
@app.route('/b')
def route_b():
    data = {"temperature": 25.3, "humidity": 60.5}
    return render_template("index.html", sensor=data)

# 형태 3: JSON 반환
@app.route('/c')
def route_c():
    data = {"temperature": 25.3, "humidity": 60.5}
    return jsonify(data)

if __name__ == '__main__':
    app.run(debug=True)
```

**브라우저에서 각각 열면:**

| 라우트 | 브라우저에 보이는 것 | 용도 |
|--------|---------------------|------|
| `/a` | `Hello, Flask!` (텍스트) | 간단한 확인용 |
| `/b` | 꾸며진 HTML 페이지 | 사람이 보는 웹페이지 |
| `/c` | `{"humidity": 60.5, "temperature": 25.3}` | 다른 프로그램과 데이터 교환 |

---

## 3. 실습 1 — 문자열 vs HTML (10분)

### 준비

`app.py` 파일 생성:

```python
from flask import Flask

app = Flask(__name__)

@app.route('/')
def index():
    return "온도: 25.3, 습도: 60.5"

if __name__ == '__main__':
    app.run(debug=True)
```

### 실습 질문

**Q1.** 위 코드를 실행하고 브라우저에서 확인했을 때 어떻게 보이나요?

```
답:
```

> ✅ **정답:** 브라우저에 `온도: 25.3, 습도: 60.5` 라는 텍스트가 그대로 출력된다.

&nbsp;

**Q2.** return 값을 아래처럼 바꾸면 어떻게 달라지나요? 직접 바꿔서 확인해 보세요.

```python
return "<h1>온도: 25.3°C</h1><p>습도: 60.5%</p>"
```

```
달라진 점:
```

> ✅ **정답:** 브라우저가 HTML을 해석해서 "온도: 25.3°C"가 큰 제목처럼 표시되고, 습도는 작은 글씨로 나온다. return에 HTML 태그를 넣으면 브라우저가 그것을 페이지로 렌더링한다.

&nbsp;

**Q3.** 문자열 안에 HTML을 직접 쓰는 방식의 단점은 무엇일까요?

```
답:
```

> ✅ **정답:** 코드가 길어지면 Python 코드 안에 HTML이 섞여서 읽기 어렵고 유지보수가 힘들다. 디자인을 바꾸려면 Python 파일을 열어야 한다.

---

## 4. 실습 2 — render_template으로 데이터 전달 (10분)

### 폴더 구조 만들기

```
project/
├── app.py
└── templates/
    └── index.html
```

### templates/index.html

```html
<!DOCTYPE html>
<html>
<head>
    <title>센서 데이터</title>
</head>
<body>
    <h1>실시간 센서 데이터</h1>
    <p>온도: {{ sensor.temperature }} °C</p>
    <p>습도: {{ sensor.humidity }} %</p>
</body>
</html>
```

### app.py 수정

```python
from flask import Flask, render_template

app = Flask(__name__)

@app.route('/')
def index():
    data = {"temperature": 25.3, "humidity": 60.5}
    return render_template("index.html", sensor=data)

if __name__ == '__main__':
    app.run(debug=True)
```

### 실습 질문

**Q4.** `render_template("index.html", sensor=data)` 에서 `sensor=data`는 무슨 역할을 하나요?

```
답:
```

> ✅ **정답:** Python의 `data` 딕셔너리를 HTML 템플릿에 `sensor`라는 이름으로 전달한다. HTML에서 `{{ sensor.temperature }}` 처럼 사용할 수 있게 된다.

&nbsp;

**Q5.** HTML의 `{{ sensor.temperature }}` 에서 `{{ }}` 는 무슨 의미인가요?

```
답:
```

> ✅ **정답:** Jinja2 템플릿 문법으로, `{{ }}` 안에 있는 변수를 Python에서 전달받은 값으로 바꿔서 출력한다.

&nbsp;

**Q6.** `data` 딕셔너리의 온도를 `28.1`로 바꾸면 브라우저에서 어떻게 바뀌나요? 직접 확인해 보세요.

```
답:
```

> ✅ **정답:** 브라우저를 새로고침하면 온도가 28.1°C로 바뀐다. Python 코드만 바꿔도 HTML 내용이 동적으로 변한다는 것을 확인할 수 있다.

---

## 5. 실습 3 — jsonify로 JSON 반환 (10분)

### app.py에 라우트 추가

```python
from flask import Flask, render_template, jsonify

app = Flask(__name__)

@app.route('/')
def index():
    data = {"temperature": 25.3, "humidity": 60.5}
    return render_template("index.html", sensor=data)

@app.route('/api/sensor')
def api_sensor():
    data = {"temperature": 25.3, "humidity": 60.5}
    return jsonify(data)

if __name__ == '__main__':
    app.run(debug=True)
```

### 실습 질문

**Q7.** 브라우저에서 `/api/sensor` 에 접속하면 무엇이 보이나요?

```
답:
```

> ✅ **정답:** `{"humidity": 60.5, "temperature": 25.3}` 형태의 JSON 텍스트가 보인다.

&nbsp;

**Q8.** `/` 와 `/api/sensor` 의 결과물이 다른 이유가 무엇인가요?

```
답:
```

> ✅ **정답:** `/`는 `render_template()`으로 HTML을 반환하기 때문에 꾸며진 웹페이지가 보이고, `/api/sensor`는 `jsonify()`로 JSON 데이터를 반환하기 때문에 데이터만 텍스트로 보인다.

&nbsp;

**Q9.** JSON 형태로 반환하는 게 언제 유용할까요?

```
답:
```

> ✅ **정답:** JavaScript에서 fetch로 데이터를 가져올 때, 또는 나중에 앱이나 다른 프로그램이 이 데이터를 사용할 때 유용하다. 사람이 보는 화면과 데이터를 분리할 수 있다.

---

## 6. 정리 — 언제 뭘 써야 하나? (5분)

```
사용자가 브라우저로 페이지를 보는 경우
→ render_template()

데이터를 다른 프로그램/JavaScript에 전달하는 경우
→ jsonify()

간단히 확인하거나 테스트할 때
→ return "문자열"
```

### 최종 확인 질문

**Q10.** 아래 상황에서 어떤 return 방식을 쓰겠나요?

| 상황 | 사용할 방식 |
|------|-------------|
| 학생들이 보는 센서 대시보드 페이지 | |
| Arduino가 데이터를 서버로 보낼 때 응답 | |
| 온도 데이터를 JavaScript 차트에 전달 | |

> ✅ **정답:**
> - 대시보드 페이지 → `render_template()`
> - Arduino 응답 → `return "OK"` 또는 `jsonify({"status": "ok"})`
> - JavaScript 차트 → `jsonify()`

---

## 📎 참고 자료

| 자료 | 링크 | 내용 |
|------|------|------|
| Flask 공식 문서 — Quickstart | https://flask.palletsprojects.com/en/stable/quickstart/ | 라우팅, return, 템플릿 기초 |
| Flask 공식 문서 — render_template | https://flask.palletsprojects.com/en/stable/api/#flask.render_template | render_template 사용법 |
| Flask 공식 문서 — jsonify | https://flask.palletsprojects.com/en/stable/api/#flask.json.jsonify | jsonify 사용법 |
| Jinja2 템플릿 문법 | https://jinja.palletsprojects.com/en/stable/templates/ | `{{ }}`, `{% %}` 문법 전체 |
| HTTP 요청/응답 개념 (MDN) | https://developer.mozilla.org/ko/docs/Web/HTTP/Overview | 브라우저-서버 통신 원리 |
| 점프 투 플라스크 (한국어) | https://wikidocs.net/book/4542 | Flask 한국어 입문서 |

---

> 💡 **다음 시간 예고:** 2교시에서는 Jinja2 템플릿의 `{% for %}` 반복문을 사용해서 여러 개의 센서 데이터를 한 번에 웹에 표시하는 방법을 배웁니다.
