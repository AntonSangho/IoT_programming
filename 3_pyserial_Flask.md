# 3교시 — Arduino 데이터를 Flask에 연결하기
**pyserial로 Serial 데이터를 읽어 웹 페이지에 실시간 표시**

> 📌 **교사 참고:** 이 문서는 정답 포함 교사용입니다. 학생 배포 시 `✅ 정답` 항목을 제거하세요.

---

### 🎯 학습 목표
- `pyserial`로 Arduino의 Serial 데이터를 Python에서 읽을 수 있다.
- 읽은 데이터를 파싱해서 Flask 템플릿에 전달할 수 있다.
- 2교시에서 만든 가짜 데이터를 실제 센서 데이터로 교체할 수 있다.

### ⏱ 시간 계획 (50분)
| 시간 | 내용 |
|------|------|
| 0~5분 | 지난 시간 복습 |
| 5~10분 | 개념 — Serial 통신과 pyserial |
| 10~20분 | 실습 1 — pyserial로 데이터 읽기 |
| 20~35분 | 실습 2 — 읽은 데이터 파싱하기 |
| 35~45분 | 실습 3 — Flask에 연결해서 웹에 표시 |
| 45~50분 | 정리 및 트러블슈팅 |

---

## 1. 지난 시간 복습 (5분)

**칠판에 질문 적기:**

> 2교시에서 가짜 데이터를 넣었던 이 변수, 오늘은 어디서 채울 건가요?

```python
records = [
    {"temperature": 25.3, "humidity": 60.5},
    ...
]
```

학생 답변 후 정리:

> 오늘은 이 부분을 **Arduino 실제 데이터**로 바꿉니다.
> 그러려면 Python이 Arduino를 읽을 수 있어야 해요 → `pyserial`

---

## 2. 개념 — Serial 통신과 pyserial (5분)

```
Arduino                Python (pyserial)          Flask
  |                         |                       |
  | -- Serial.print() -->   |                       |
  |   "25.3,60.5\n"         | -- 파싱 -->  dict    |
  |                         |              data  --> | render_template()
  |                         |                       |
```

- Arduino는 이미 `Serial.print("25.3,60.5")` 형태로 데이터를 보내고 있어요.
- Python의 `pyserial` 라이브러리가 그 데이터를 읽습니다.
- 읽은 문자열을 파싱해서 Flask가 웹에 표시합니다.

**pyserial 설치 확인:**
```bash
pip install pyserial
```

---

## 3. 실습 1 — pyserial로 데이터 읽기 (10분)

### Arduino 코드 확인

학생들이 이미 만든 Arduino 코드가 이 형태인지 확인하세요:

```cpp
#include <DHT.h>

#define DHTPIN 2
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

void setup() {
    Serial.begin(9600);
    dht.begin();
}

void loop() {
    float temp = dht.readTemperature();
    float hum  = dht.readHumidity();

    Serial.print(temp);
    Serial.print(",");
    Serial.println(hum);   // \n 포함

    delay(2000);
}
```

> ⚠️ **중요:** `Serial.println()` 이 줄바꿈(`\n`)을 자동으로 붙여줍니다. Python에서 한 줄씩 읽을 때 이 줄바꿈을 기준으로 잘라냅니다.

### Python 파일 — serial_test.py

```python
import serial
import time

# 포트 이름은 본인 환경에 맞게 변경
# Windows: "COM3", "COM4" 등
# Mac/Linux: "/dev/ttyUSB0", "/dev/ttyACM0" 등

ser = serial.Serial("COM3", 9600, timeout=1)
time.sleep(2)  # Arduino 리셋 대기

for _ in range(5):
    line = ser.readline()          # 한 줄 읽기 (bytes)
    print("원본:", line)
    decoded = line.decode("utf-8").strip()  # 문자열로 변환, 공백 제거
    print("변환:", decoded)

ser.close()
```

**Q1.** `line.decode("utf-8")` 가 없으면 어떤 형태로 출력되나요?

```
답:
```

> ✅ **정답:** `b'25.3,60.5\r\n'` 처럼 앞에 `b`가 붙은 bytes 형태로 출력된다. Python에서 Serial 데이터는 기본적으로 bytes이기 때문에 문자열로 쓰려면 `decode()`가 필요하다.

&nbsp;

**Q2.** `.strip()` 을 쓰지 않으면 문자열 끝에 무엇이 남나요?

```
답:
```

> ✅ **정답:** `\r\n` (줄바꿈 문자)가 남는다. `.strip()`은 문자열 앞뒤의 공백과 줄바꿈을 제거한다.

&nbsp;

**Q3.** `timeout=1` 의 의미는 무엇인가요?

```
답:
```

> ✅ **정답:** Arduino에서 데이터가 오지 않을 때 최대 1초까지 기다린다는 뜻이다. timeout이 없으면 데이터가 올 때까지 무한 대기할 수 있다.

---

## 4. 실습 2 — 읽은 데이터 파싱하기 (15분)

Serial에서 읽은 문자열은 `"25.3,60.5"` 형태입니다.
이것을 온도와 습도로 분리해야 합니다.

### 파싱 코드

```python
raw = "25.3,60.5"          # Serial에서 받은 문자열 예시

parts = raw.split(",")      # 쉼표 기준으로 분리
print(parts)                # ['25.3', '60.5']

temperature = float(parts[0])
humidity    = float(parts[1])

print(f"온도: {temperature}, 습도: {humidity}")
```

**Q4.** `"25.3,60.5".split(",")` 의 결과는 무엇인가요?

```
답:
```

> ✅ **정답:** `['25.3', '60.5']` — 쉼표를 기준으로 나뉜 문자열 리스트가 반환된다.

&nbsp;

**Q5.** `parts[0]`은 문자열 `"25.3"` 인데, 왜 `float()`로 변환해야 하나요?

```
답:
```

> ✅ **정답:** Serial에서 읽은 데이터는 모두 문자열(str)이다. 숫자로 계산하거나 비교하려면 `float()`로 숫자 타입으로 변환해야 한다. 변환하지 않으면 `"25.3" > 28` 같은 비교가 오류를 낸다.

&nbsp;

**Q6.** Arduino가 잘못된 값을 보낼 때(`nan`, 빈 문자열 등) `float()` 변환이 실패할 수 있습니다.
이것을 방지하는 코드를 작성해 보세요.

```python
raw = ser.readline().decode("utf-8").strip()

# 여기에 예외 처리 추가:


```

> ✅ **정답:**
> ```python
> raw = ser.readline().decode("utf-8").strip()
> try:
>     parts = raw.split(",")
>     temperature = float(parts[0])
>     humidity    = float(parts[1])
> except (ValueError, IndexError):
>     print("잘못된 데이터:", raw)
>     temperature = None
>     humidity    = None
> ```

---

## 5. 실습 3 — Flask에 연결해서 웹에 표시 (10분)

이제 pyserial과 Flask를 합칩니다.
2교시의 가짜 데이터를 실제 Arduino 데이터로 교체합니다.

### app.py

```python
import serial
import time
from flask import Flask, render_template

app = Flask(__name__)

def read_sensor():
    """Arduino에서 센서 데이터 1개를 읽어 딕셔너리로 반환"""
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
        return {"temperature": None, "humidity": None}

@app.route('/')
def index():
    data = read_sensor()
    return render_template("index.html", sensor=data)

if __name__ == '__main__':
    app.run(debug=True)
```

### templates/index.html

```html
<!DOCTYPE html>
<html>
<head>
    <title>DHT11 센서</title>
    <meta http-equiv="refresh" content="5">  <!-- 5초마다 자동 새로고침 -->
</head>
<body>
    <h1>실시간 센서 데이터</h1>
    {% if sensor.temperature %}
        <p>🌡️ 온도: {{ sensor.temperature }} °C</p>
        <p>💧 습도: {{ sensor.humidity }} %</p>
    {% else %}
        <p>⚠️ 센서 데이터를 읽을 수 없습니다.</p>
    {% endif %}
</body>
</html>
```

**Q7.** `<meta http-equiv="refresh" content="5">` 는 무슨 역할을 하나요?

```
답:
```

> ✅ **정답:** 5초마다 페이지를 자동으로 새로고침한다. 브라우저가 자동으로 서버에 재요청을 보내기 때문에 Flask가 `read_sensor()`를 다시 실행해 최신 데이터를 보여준다.

&nbsp;

**Q8.** `read_sensor()` 함수가 라우트 함수 밖에 따로 있는 이유는 무엇일까요?

```
답:
```

> ✅ **정답:** 코드를 역할별로 분리하기 위해서다. 센서를 읽는 기능과 웹 응답을 만드는 기능을 분리하면 나중에 수정하거나 재사용하기 쉽다. 예를 들어 나중에 `/api/sensor` 라우트를 추가할 때도 `read_sensor()`를 그대로 쓸 수 있다.

&nbsp;

**Q9.** Arduino를 연결하지 않은 상태에서 실행하면 어떻게 되나요?

```
답:
```

> ✅ **정답:** `except` 블록이 실행되어 `{"temperature": None, "humidity": None}` 이 반환되고, 웹에서 "⚠️ 센서 데이터를 읽을 수 없습니다."가 표시된다. 예외 처리 덕분에 서버가 죽지 않는다.

---

## 6. 정리 및 트러블슈팅 (5분)

### 핵심 흐름 정리

```
Arduino           Python (pyserial)              Flask
Serial.println()  readline() → decode() → split()  render_template()
"25.3,60.5\n"  →  ["25.3", "60.5"]          →  {"temperature": 25.3}
```

### 자주 발생하는 오류

| 오류 메시지 | 원인 | 해결 방법 |
|-------------|------|-----------|
| `serial.SerialException: could not open port` | 포트 이름이 틀렸거나 이미 사용 중 | 장치 관리자에서 포트 번호 확인, Arduino IDE Serial Monitor 닫기 |
| `ValueError: could not convert string to float` | Arduino 데이터 형식이 예상과 다름 | `serial_test.py`로 raw 데이터 먼저 확인 |
| 데이터가 안 들어옴 | `time.sleep(2)` 부족 | 대기 시간을 3초로 늘리기 |
| 한글 깨짐 | 인코딩 문제 | `decode("utf-8")` 대신 `decode("cp949")` 시도 |

### 최종 확인 질문

**Q10.** 오늘 실습 전체 흐름을 한 줄로 설명해 보세요.

```
답:
```

> ✅ **정답 예시:** Arduino가 Serial로 보낸 `"온도,습도"` 문자열을 pyserial이 읽고, Python이 파싱해서 딕셔너리로 만든 뒤, Flask가 그것을 HTML 템플릿에 전달해 웹 브라우저에 표시한다.

---

## 📎 참고 자료

| 자료 | 링크 | 내용 |
|------|------|------|
| pyserial 공식 문서 | https://pyserial.readthedocs.io/en/latest/ | Serial 연결, read/write 전체 API |
| pyserial — readline() | https://pyserial.readthedocs.io/en/latest/pyserial_api.html#serial.Serial.readline | readline 사용법 및 timeout |
| Python str.split() | https://docs.python.org/ko/3/library/stdtypes.html#str.split | 문자열 분리 방법 |
| Python 예외 처리 (try/except) | https://docs.python.org/ko/3/tutorial/errors.html | try/except 기본 문법 |
| Arduino Serial.println() | https://www.arduino.cc/reference/en/language/functions/communication/serial/println/ | Arduino Serial 출력 형식 |
| 점프 투 플라스크 | https://wikidocs.net/book/4542 | Flask 한국어 입문서 |

---

> 💡 **다음 시간 예고:** 4교시에서는 지금까지 읽어온 데이터를 "어떻게 저장할 것인가"를 고민합니다. MySQL 테이블 구조를 스스로 설계해보는 시간이에요.
