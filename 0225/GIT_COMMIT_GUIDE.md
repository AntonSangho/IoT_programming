# Git Commit 가이드 - ESP8266 IoT 프로젝트

이 문서는 esp8266web 프로젝트를 **단계별로 코드를 작성**하면서, 각 단계마다 git commit으로 진행 상황을 저장하는 방법을 안내합니다.

> 코드를 잘못 수정했을 때 이전 상태로 쉽게 돌아갈 수 있습니다.

---

## 0. Git 초기 설정

### Git 설치 확인

```bash
git --version
```

버전 번호가 나오면 이미 설치되어 있습니다.

### 사용자 정보 설정 (처음 한 번만)

```bash
git config --global user.name "홍길동"
git config --global user.email "student@example.com"
```

### 프로젝트 폴더에서 Git 시작

```bash
cd esp8266web
git init
```

`git init`은 이 폴더를 Git이 관리하는 폴더로 만듭니다.

### 첫 번째 커밋 (빈 프로젝트 저장)

```bash
git add .
git commit -m "프로젝트 시작"
```

| 명령어 | 설명 |
|--------|------|
| `git add .` | 폴더 안의 모든 파일을 커밋 준비 상태로 만듦 |
| `git commit -m "메시지"` | 현재 상태를 저장 (스냅샷) |

---

## 1단계: Wi-Fi 연결 (esp8266web.ino)

### 작성할 코드

`esp8266web.ino` 파일을 만들고, ESP8266이 Wi-Fi에 접속하여 IP 주소를 출력하는 코드를 작성합니다.

```cpp
#include <ESP8266WiFi.h>

const char* ssid = "MySSID";        // 본인의 Wi-Fi 이름
const char* password = "MyPassword"; // 본인의 Wi-Fi 비밀번호

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println(WiFi.localIP());
}

void loop() {
}
```

### 확인 방법

1. Arduino IDE에서 업로드
2. **시리얼 모니터** (115200 baud)에서 IP 주소가 출력되면 성공

### 커밋

```bash
git add esp8266web.ino
git commit -m "Step 1: Wi-Fi 연결 코드 작성"
```

---

## 2단계: 웹서버 + LED 제어 추가 (esp8266web.ino)

### 작성할 코드

1단계 코드에 웹서버를 추가합니다. 브라우저에서 `/led`에 접속하면 LED가 켜지거나 꺼집니다.

**추가할 내용:**

```cpp
#include <ESP8266WebServer.h>

ESP8266WebServer server(80);  // 80번 포트에서 웹서버 시작

int led = 13;       // LED 핀 번호
int ledstate = 0;   // LED 상태 (0=꺼짐, 1=켜짐)
```

`setup()` 안에 추가:

```cpp
pinMode(led, OUTPUT);
server.on("/led", handleled);   // /led 요청이 오면 handleled 함수 실행
server.begin();                 // 웹서버 시작
```

`loop()` 안에 추가:

```cpp
server.handleClient();  // 클라이언트 요청을 계속 확인
```

새 함수 추가:

```cpp
void handleled() {
  ledstate = !ledstate;              // LED 상태 반전
  digitalWrite(led, ledstate);       // LED 켜기/끄기
  server.send(200, "text/plain", "OK");  // 브라우저에 "OK" 응답
}
```

### 확인 방법

1. 업로드 후 시리얼 모니터에서 IP 주소 확인
2. 브라우저에서 `http://<IP주소>/led` 접속
3. LED가 켜지고, 다시 접속하면 꺼지면 성공

### 커밋

```bash
git add esp8266web.ino
git commit -m "Step 2: 웹서버와 LED 제어 기능 추가"
```

---

## 3단계: DHT22 센서 + JSON 응답 추가 (esp8266web.ino)

### 작성할 코드

온습도 센서(DHT22)를 읽어서 JSON 형식으로 응답하는 기능을 추가합니다.

**추가할 내용:**

```cpp
#include <DHT.h>

DHT dht(2, DHT22, 12);  // GPIO 2번 핀에 DHT22 센서 연결

float humi, temp;
String webString = "";
unsigned long previousMillis = 0;
const long interval = 2000;  // 2초마다 센서 읽기
```

`setup()` 안에 추가:

```cpp
dht.begin();
server.on("/events", handleevents);  // /events 요청 처리
```

새 함수 추가:

```cpp
void handleevents() {
  gettemphumi();
  webString = "{\"temperature\": \"" + String(temp)
            + "\", \"humidity\": \"" + String(humi) + "\" }";
  server.send(200, "text/plain", webString);
}

void gettemphumi() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    humi = dht.readHumidity();
    temp = dht.readTemperature(false);  // false = 섭씨
    if (isnan(humi) || isnan(temp)) {
      Serial.println("Failed to read dht sensor.");
      return;
    }
  }
}
```

### 확인 방법

1. 업로드 후 브라우저에서 `http://<IP주소>/events` 접속
2. 다음과 같은 JSON이 보이면 성공:
   ```json
   {"temperature": "25.30", "humidity": "60.10"}
   ```

### 커밋

```bash
git add esp8266web.ino
git commit -m "Step 3: DHT22 센서 읽기와 JSON 응답 추가"
```

---

## 4단계-A: Python Flask 서버 (esp8266web.py)

### 작성할 코드

PC(또는 라즈베리파이)에서 실행할 Python 웹서버입니다. 브라우저와 ESP8266 사이의 **중계 서버** 역할을 합니다.

`esp8266web.py` 파일을 새로 만듭니다:

```python
#-*- coding: utf-8 -*-
from flask import Flask, render_template
try:
    from urllib.request import urlopen       # Python 3
    from urllib.error import HTTPError, URLError
except ImportError:
    from urllib2 import urlopen              # Python 2
    from urllib2 import HTTPError, URLError

deviceIp = "<장치 IP>"   # ESP8266의 IP 주소 (시리얼 모니터에서 확인)
portnum = "80"

base_url = "http://" + deviceIp + ":" + portnum
events_url = base_url + "/events"

app = Flask(__name__, template_folder=".")

@app.route('/events')
def getevents():
    u = urlopen(events_url)
    data = ""
    try:
        data = u.read()
    except HTTPError as e:
        print("HTTP error: %d" % e.code)
    except URLError as e:
        print("Network error: %s" % e.reason.args[1])
    return data

@app.route('/')
def dht22chart():
    return render_template("dhtchart.html")

if __name__ == '__main__':
    app.run(host="<서버 IP>", port=<PORT>)
```

> `<장치 IP>`, `<서버 IP>`, `<PORT>`를 본인의 환경에 맞게 수정하세요.

### 확인 방법

1. Flask 설치: `pip install flask`
2. 실행: `python esp8266web.py`
3. 브라우저에서 `http://<서버IP>:<PORT>/events`에 접속하여 JSON 데이터가 보이면 성공

### 커밋

```bash
git add esp8266web.py
git commit -m "Step 4-A: Python Flask 중계 서버 추가"
```

---

## 4단계-B: HTML 차트 페이지 (dhtchart.html)

### 작성할 코드

실시간 온도/습도 그래프를 보여주는 웹 페이지입니다. `dhtchart.html` 파일을 새로 만듭니다:

```html
<!DOCTYPE HTML>
<html lang="en">
<head>
  <meta charset="utf-8" />
  <meta HTTP-EQUIV="CACHE-CONTROL" CONTENT="NO-CACHE" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <title>실시간 온습도 모니터</title>
  <link rel="stylesheet"
    href="//code.jquery.com/mobile/1.4.3/jquery.mobile-1.4.3.min.css" />
  <script src="//code.jquery.com/jquery-1.11.1.min.js"></script>
  <script src="//code.jquery.com/mobile/1.4.3/jquery.mobile-1.4.3.min.js"></script>
  <script
    src="//cdnjs.cloudflare.com/ajax/libs/highcharts/4.0.4/highcharts.js"></script>

<script>
    var chart;   // 온도 차트
    var chart2;  // 습도 차트
    var pollUri = "/events";

    // 5초마다 서버에서 데이터를 가져오는 함수
    function pollEvent() {
        $.getJSON(pollUri,
            function(data) {
                var t = new Date();
                t.setHours(t.getHours() + 9);  // 한국 시간(KST) 보정
                chartAddPoint([t.getTime(), Number(data.temperature)]);
                chart2AddPoint([t.getTime(), Number(data.humidity)]);
            }
        );
        setTimeout('pollEvent()', 5000);  // 5초 후 다시 호출
    }

    pollEvent();

    function chartAddPoint(tval) {
        var series = chart.series[0],
        shift = series.data.length > 20;  // 20개 넘으면 오래된 데이터 삭제
        chart.series[0].addPoint(eval(tval), true, shift);
    }

    function chart2AddPoint(hval) {
        var series2 = chart2.series[0],
        shift2 = series2.data.length > 20;
        chart2.series[0].addPoint(eval(hval), true, shift2);
    }

    $(function() {
        // 온도 차트 설정
        chart = new Highcharts.Chart({
            chart: { renderTo: 'temp', defaultSeriesType: 'spline' },
            title: { text: '실시간 온도 데이터' },
            xAxis: { type: 'datetime', tickPixelInterval: 120,
                     maxZoom: 20 * 1000 },
            yAxis: { minPadding: 0.2, maxPadding: 0.2,
                     title: { text: 'Temperature (C)', margin: 20 } },
            series: [{ name: 'Temperature', data: [] }]
        });
        // 습도 차트 설정
        chart2 = new Highcharts.Chart({
            chart: { renderTo: 'humi', defaultSeriesType: 'spline' },
            title: { text: '실시간 습도 데이터' },
            xAxis: { type: 'datetime', tickPixelInterval: 120,
                     maxZoom: 20 * 1000 },
            yAxis: { minPadding: 0.2, maxPadding: 0.2,
                     title: { text: 'Humidity (%)', margin: 20 } },
            series: [{ name: 'Humidity', data: [] }]
        });
    });
</script>
</head>
<body>
  <div id="temp" style="width: 100%; height: 300px; margin-left:-5px;"></div>
  <div id="humi" style="width: 100%; height: 300px; margin-left:-5px;"></div>
</body>
</html>
```

### 확인 방법

1. Python 서버가 실행 중인 상태에서
2. 브라우저에서 `http://<서버IP>:<PORT>/` 접속
3. 실시간 온도/습도 그래프가 5초마다 업데이트되면 성공

### 커밋

```bash
git add dhtchart.html
git commit -m "Step 4-B: 실시간 온습도 차트 HTML 페이지 추가"
```

---

## 유용한 Git 명령어

### 현재 상태 확인

```bash
git status
```

수정된 파일, 커밋 안 된 변경 사항을 보여줍니다.

### 커밋 기록 보기

```bash
git log --oneline
```

지금까지 한 커밋 목록을 한 줄씩 보여줍니다. 출력 예시:

```
a1b2c3d Step 4-B: 실시간 온습도 차트 HTML 페이지 추가
d4e5f6g Step 4-A: Python Flask 중계 서버 추가
h7i8j9k Step 3: DHT22 센서 읽기와 JSON 응답 추가
l0m1n2o Step 2: 웹서버와 LED 제어 기능 추가
p3q4r5s Step 1: Wi-Fi 연결 코드 작성
t6u7v8w 프로젝트 시작
```

### 변경 내용 비교

```bash
git diff
```

마지막 커밋 이후 무엇이 바뀌었는지 보여줍니다. 코드가 어디서 잘못됐는지 찾을 때 유용합니다.

### 이전 단계로 돌아가기

코드가 망가졌을 때, 이전 커밋 상태로 돌아갈 수 있습니다.

```bash
# 1. 먼저 돌아갈 커밋의 ID를 확인
git log --oneline

# 2. 해당 커밋으로 돌아가기 (예: Step 2로 돌아가기)
git checkout l0m1n2o -- esp8266web.ino
```

> `l0m1n2o` 부분은 `git log`에서 확인한 실제 커밋 ID로 바꾸세요.

이 명령은 해당 파일만 이전 상태로 되돌립니다. 다른 파일은 영향 없습니다.

### 모든 변경 취소 (주의!)

현재 수정 중인 내용을 모두 버리고 마지막 커밋 상태로 돌아갑니다:

```bash
git checkout -- esp8266web.ino
```

> **주의:** 저장하지 않은 변경 사항이 모두 사라집니다.

---

## 전체 흐름 요약

| 단계 | 파일 | 핵심 내용 | 확인 방법 |
|------|------|----------|----------|
| 1 | esp8266web.ino | Wi-Fi 연결 | 시리얼 모니터에서 IP 확인 |
| 2 | esp8266web.ino | 웹서버 + LED 제어 | 브라우저에서 LED 토글 |
| 3 | esp8266web.ino | DHT22 센서 + JSON | 브라우저에서 JSON 확인 |
| 4-A | esp8266web.py | Flask 중계 서버 | 서버 실행 후 데이터 확인 |
| 4-B | dhtchart.html | 실시간 차트 | 그래프 업데이트 확인 |

각 단계를 완성할 때마다 `git add` → `git commit`으로 저장하세요!
