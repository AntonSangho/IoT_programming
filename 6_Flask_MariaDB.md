# 6교시 — Flask와 MariaDB 연결하기
**Raspberry Pi의 MariaDB에 접속하고 데이터를 저장·조회하기**

> 📌 **교사 참고:** 이 문서는 정답 포함 교사용입니다. 학생 배포 시 `✅ 정답` 항목을 제거하세요.

---

### 🎯 학습 목표
- Python에서 `PyMySQL`로 Raspberry Pi의 MariaDB에 접속할 수 있다.
- Flask 라우트에서 INSERT와 SELECT를 코드로 실행할 수 있다.
- 조회한 데이터를 Jinja2 템플릿에 전달해 웹에 표시할 수 있다.

### ⏱ 시간 계획 (50분)
| 시간 | 내용 |
|------|------|
| 0~5분 | 지난 시간 복습 |
| 5~15분 | 개념 — MariaDB와 PyMySQL |
| 15~25분 | 실습 1 — 연결 테스트 및 SELECT |
| 25~35분 | 실습 2 — Flask 라우트에서 INSERT |
| 35~45분 | 실습 3 — 저장된 데이터를 웹에 표시 |
| 45~50분 | 정리 |

---

## 1. 지난 시간 복습 (5분)

**칠판에 질문 적기:**

> 5교시에서 배운 SQL 4가지를 말해보세요.

```
INSERT INTO → 데이터 추가
SELECT      → 데이터 조회
WHERE       → 조건 필터
ORDER BY    → 정렬
```

> 오늘은 이 SQL을 직접 치는 게 아니라
> **Python 코드 안에서** 실행합니다.

---

## 2. 개념 — MariaDB와 PyMySQL (10분)

### MySQL vs MariaDB

| | MySQL | MariaDB |
|--|-------|---------|
| 주로 쓰는 환경 | 일반 PC, 서버 | Raspberry Pi, Linux |
| SQL 문법 | 같음 | 같음 |
| Python 라이브러리 | `mysql-connector-python` | `PyMySQL` |

> 💡 MariaDB는 MySQL에서 파생된 데이터베이스입니다.
> SQL 문법은 거의 동일하지만 Raspberry Pi에서는 MariaDB가 기본으로 설치됩니다.

### Raspberry Pi에서 MariaDB 준비

**터미널에서 실행:**
```bash
# MariaDB 서버 설치 (이미 설치된 경우 생략)
sudo apt install mariadb-server -y

# MariaDB 접속
sudo mariadb

# DB와 사용자 생성 (MariaDB 콘솔 안에서)
CREATE DATABASE sensor_db;
CREATE USER 'pi'@'localhost' IDENTIFIED BY '비밀번호';
GRANT ALL PRIVILEGES ON sensor_db.* TO 'pi'@'localhost';
FLUSH PRIVILEGES;
EXIT;
```

### PyMySQL 설치

```bash
pip install pymysql
```

### 전체 연결 흐름

```
Python (PyMySQL)          MariaDB 서버 (Raspberry Pi)
  |                               |
  |-- pymysql.connect() -------> |  ← 접속
  |<-- connection 객체 ----------|
  |                               |
  |-- cursor() ----------------> |  ← SQL 실행 도구 생성
  |-- execute("SQL") ----------> |  ← SQL 전송
  |<-- 결과 --------------------|
  |                               |
  |-- close() ----------------> |  ← 연결 종료
```

### 기본 구조

```python
import pymysql

# 1. 연결
conn = pymysql.connect(
    host="localhost",
    user="pi",
    password="비밀번호",
    database="sensor_db",
    charset="utf8mb4"
)

# 2. 커서 생성 (딕셔너리 형태로 받기)
cursor = conn.cursor(pymysql.cursors.DictCursor)

# 3. SQL 실행
cursor.execute("SELECT * FROM sensor_data")

# 4. 결과 가져오기
rows = cursor.fetchall()
print(rows)

# 5. 닫기
cursor.close()
conn.close()
```

> 💡 **`charset="utf8mb4"`** 를 꼭 넣어주세요. 한글 데이터나 특수문자가 깨지는 것을 방지합니다.

---

## 3. 실습 1 — 연결 테스트 및 SELECT (10분)

### db_test.py 작성

```python
import pymysql

conn = pymysql.connect(
    host="localhost",
    user="pi",
    password="비밀번호",
    database="sensor_db",
    charset="utf8mb4"
)

cursor = conn.cursor(pymysql.cursors.DictCursor)
cursor.execute("SELECT * FROM sensor_data ORDER BY recorded_at DESC LIMIT 5")
rows = cursor.fetchall()

for row in rows:
    print(row)

cursor.close()
conn.close()
```

**Q1.** `fetchall()` 의 결과는 어떤 형태인가요? 실행해서 확인해보세요.

```
답:
```

> ✅ **정답:** `DictCursor`를 사용했기 때문에 딕셔너리의 리스트 형태로 반환된다.
> 예: `[{"id": 1, "temperature": 25.3, "humidity": 60.5, "recorded_at": datetime(...)}, ...]`
> 각 행이 딕셔너리이고, 컬럼 이름으로 값에 접근할 수 있다.

&nbsp;

**Q2.** `DictCursor` 를 쓰지 않고 일반 `cursor()`를 쓰면 결과가 어떻게 달라지나요?

```
답:
```

> ✅ **정답:** 딕셔너리가 아닌 튜플의 리스트로 반환된다.
> 예: `[(1, 25.3, 60.5, datetime(...)), ...]`
> 이 경우 `row["temperature"]` 대신 `row[1]` 처럼 인덱스로 접근해야 한다.

&nbsp;

**Q3.** `charset="utf8mb4"` 를 빠뜨리면 어떤 상황에서 문제가 생길 수 있나요?

```
답:
```

> ✅ **정답:** 한글, 이모지, 특수문자가 포함된 데이터를 저장하거나 조회할 때 인코딩 오류가 발생하거나 글자가 깨질 수 있다. `utf8mb4`는 이모지를 포함한 모든 유니코드 문자를 지원한다.

---

## 4. 실습 2 — Flask 라우트에서 INSERT (10분)

### app.py

```python
import pymysql
from flask import Flask, render_template

app = Flask(__name__)

def get_connection():
    """MariaDB 연결 객체를 반환하는 함수"""
    return pymysql.connect(
        host="localhost",
        user="pi",
        password="비밀번호",
        database="sensor_db",
        charset="utf8mb4"
    )

@app.route('/save')
def save():
    """가짜 센서 데이터를 DB에 저장 (나중에 실제 Serial 데이터로 교체)"""
    temperature = 25.3
    humidity    = 60.5

    conn   = get_connection()
    cursor = conn.cursor()

    sql = "INSERT INTO sensor_data (temperature, humidity) VALUES (%s, %s)"
    cursor.execute(sql, (temperature, humidity))
    conn.commit()   # ← INSERT/UPDATE/DELETE 후 반드시 필요

    cursor.close()
    conn.close()

    return "저장 완료!"

if __name__ == '__main__':
    app.run(host="0.0.0.0", debug=True)   # host="0.0.0.0" → 같은 네트워크에서 접속 가능
```

> 💡 **`host="0.0.0.0"`** 을 쓰면 같은 Wi-Fi에 연결된 다른 기기(노트북, 스마트폰)에서도
> `http://라즈베리파이IP:5000` 으로 접속할 수 있어요.

**Q4.** `%s` 는 무엇인가요? 왜 직접 숫자를 넣지 않나요?

```python
# 이렇게 쓰지 않는 이유:
sql = f"INSERT INTO sensor_data (temperature, humidity) VALUES ({temperature}, {humidity})"
```

```
답:
```

> ✅ **정답:** `%s`는 파라미터 바인딩(Parameterized Query)이다. SQL 문자열에 값을 직접 넣으면 SQL Injection 공격에 취약해진다. `%s`와 `execute(sql, (값,))` 방식을 쓰면 라이브러리가 값을 안전하게 처리해준다.

&nbsp;

**Q5.** `conn.commit()` 을 빠뜨리면 어떻게 되나요?

```
답:
```

> ✅ **정답:** 코드는 오류 없이 실행되지만 데이터가 실제로 DB에 저장되지 않는다. MariaDB는 기본적으로 트랜잭션을 사용하기 때문에 `commit()`을 해야 변경사항이 확정된다.

&nbsp;

**Q6.** Raspberry Pi의 IP 주소는 어떻게 확인하나요?

```
답:
```

> ✅ **정답:** Raspberry Pi 터미널에서 `hostname -I` 명령을 실행하면 IP 주소를 확인할 수 있다. 예: `192.168.0.15` 형태로 출력된다.

---

## 5. 실습 3 — 저장된 데이터를 웹에 표시 (10분)

이제 저장된 데이터를 조회해서 웹에 표시합니다.

### app.py에 라우트 추가

```python
@app.route('/')
def index():
    conn   = get_connection()
    cursor = conn.cursor(pymysql.cursors.DictCursor)

    cursor.execute("SELECT * FROM sensor_data ORDER BY recorded_at DESC LIMIT 10")
    rows = cursor.fetchall()

    cursor.close()
    conn.close()

    return render_template("index.html", records=rows)
```

### templates/index.html

```html
<!DOCTYPE html>
<html lang="ko">
<head>
    <meta charset="UTF-8">
    <title>센서 데이터</title>
    <meta http-equiv="refresh" content="5">
</head>
<body>
    <h1>센서 기록 (최근 10개)</h1>
    <table border="1">
        <tr>
            <th>ID</th>
            <th>온도 (°C)</th>
            <th>습도 (%)</th>
            <th>측정 시각</th>
        </tr>
        {% for row in records %}
        <tr>
            <td>{{ row.id }}</td>
            <td>{{ row.temperature }}</td>
            <td>{{ row.humidity }}</td>
            <td>{{ row.recorded_at }}</td>
        </tr>
        {% endfor %}
    </table>
    <p><a href="/save">테스트 데이터 저장</a></p>
</body>
</html>
```

**Q7.** `pymysql.cursors.DictCursor` 덕분에 템플릿에서 `row.temperature` 처럼 쓸 수 있었습니다.
일반 커서를 쓸 경우 템플릿에서 어떻게 바꿔야 하나요?

```html
<!-- DictCursor 사용 시 -->
<td>{{ row.temperature }}</td>

<!-- 일반 커서 사용 시 -->
<td>{{ __________ }}</td>
```

> ✅ **정답:** `{{ row[1] }}` — 튜플은 인덱스로만 접근할 수 있다. temperature가 두 번째 컬럼이므로 `row[1]`이 된다. DictCursor를 쓰는 이유가 바로 이 불편함을 없애기 위해서다.

&nbsp;

**Q8.** `LIMIT 10` 을 없애면 어떤 문제가 생길 수 있나요?

```
답:
```

> ✅ **정답:** 데이터가 수천, 수만 개 쌓이면 모두 불러오기 때문에 속도가 느려지고 Raspberry Pi처럼 메모리가 제한된 환경에서는 특히 문제가 된다. 항상 LIMIT으로 개수를 제한해야 한다.

---

## 6. 정리 (5분)

```
설치    → pip install pymysql
연결    → pymysql.connect(host, user, password, database, charset="utf8mb4")
커서    → conn.cursor(pymysql.cursors.DictCursor)
조회    → cursor.execute(SQL) → cursor.fetchall()
저장    → cursor.execute(SQL, (값,)) → conn.commit()
닫기    → cursor.close() → conn.close()
```

### mysql-connector-python vs PyMySQL 비교

| | mysql-connector-python | PyMySQL |
|--|------------------------|---------|
| 설치 | `pip install mysql-connector-python` | `pip install pymysql` |
| import | `import mysql.connector` | `import pymysql` |
| 연결 | `mysql.connector.connect(...)` | `pymysql.connect(...)` |
| DictCursor | `conn.cursor(dictionary=True)` | `conn.cursor(pymysql.cursors.DictCursor)` |
| Raspberry Pi 호환 | 설치 오류 발생할 수 있음 | ✅ 안정적 |

### 최종 확인 질문

**Q9.** 아래 코드에서 잘못된 부분 2곳을 찾아보세요.

```python
cursor.execute("INSERT INTO sensor_data (temperature, humidity) VALUES (%s, %s)", (25.3, 60.5))
cursor.close()
conn.close()
```

```
잘못된 부분:
```

> ✅ **정답:**
> 1. `conn.commit()` 이 빠져있다 → 데이터가 실제로 저장되지 않는다.
> 2. `cursor.close()` 전에 `conn.commit()`을 해야 한다.
>
> 올바른 코드:
> ```python
> cursor.execute("INSERT INTO sensor_data (temperature, humidity) VALUES (%s, %s)", (25.3, 60.5))
> conn.commit()
> cursor.close()
> conn.close()
> ```

---

## 📎 참고 자료

| 자료 | 링크 | 내용 |
|------|------|------|
| PyMySQL 공식 문서 | https://pymysql.readthedocs.io/en/latest/ | PyMySQL 전체 API |
| PyMySQL GitHub | https://github.com/PyMySQL/PyMySQL | 설치 및 예제 |
| MariaDB 공식 문서 | https://mariadb.com/kb/en/documentation/ | MariaDB 전체 가이드 |
| Raspberry Pi — MariaDB 설치 가이드 | https://raspberrytips.com/install-mariadb-raspberry-pi/ | Pi에서 MariaDB 설치 |
| SQL Injection 설명 (OWASP) | https://owasp.org/www-community/attacks/SQL_Injection | %s 파라미터 바인딩이 필요한 이유 |
| 점프 투 플라스크 | https://wikidocs.net/book/4542 | Flask 한국어 입문서 |

---

> 💡 **다음 시간 예고:** 7교시에서는 지금까지 만든 모든 것을 하나로 연결합니다.
> Arduino → pyserial → Flask → MariaDB 저장 → 웹 표시까지 전체 흐름을 완성합니다.
