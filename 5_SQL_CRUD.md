# 5교시 — SQL CRUD 직접 작성하기
**INSERT, SELECT, WHERE, ORDER BY 를 손으로 써보기**

> 📌 **교사 참고:** 이 문서는 정답 포함 교사용입니다. 학생 배포 시 `✅ 정답` 항목을 제거하세요.
> 막히는 학생에게는 정답을 바로 주지 말고 **힌트 카드** (문서 하단) 를 먼저 건네세요.

---

### 🎯 학습 목표
- 4교시에서 설계한 테이블에 데이터를 직접 `INSERT` 할 수 있다.
- `SELECT` 로 데이터를 조회하고 `WHERE`, `ORDER BY` 로 필터/정렬할 수 있다.
- 집계 함수 `COUNT`, `AVG`, `MAX`, `MIN` 을 활용할 수 있다.

### ⏱ 시간 계획 (50분)
| 시간 | 내용 |
|------|------|
| 0~5분 | 지난 시간 복습 — 4교시에서 만든 테이블 확인 |
| 5~20분 | 실습 1 — INSERT로 데이터 넣기 |
| 20~35분 | 실습 2 — SELECT / WHERE / ORDER BY |
| 35~45분 | 실습 3 — 집계 함수 |
| 45~50분 | 정리 |

---

## 1. 지난 시간 복습 (5분)

4교시에서 완성한 테이블이 있는지 확인합니다.

```sql
-- 테이블이 만들어졌는지 확인
SHOW TABLES;

-- 테이블 구조 확인
DESC sensor_data;
```

예상 결과:

```
+-------------+----------+------+-----+-------------------+
| Field       | Type     | Null | Key | Default           |
+-------------+----------+------+-----+-------------------+
| id          | int      | NO   | PRI | NULL              |
| temperature | float    | YES  |     | NULL              |
| humidity    | float    | YES  |     | NULL              |
| recorded_at | datetime | YES  |     | CURRENT_TIMESTAMP |
+-------------+----------+------+-----+-------------------+
```

> ⚠️ 테이블이 없는 학생은 먼저 4교시 실습지의 CREATE TABLE을 실행하고 시작합니다.

---

## 2. 실습 1 — INSERT로 데이터 넣기 (15분)

### 기본 문법

```sql
INSERT INTO 테이블명 (컬럼1, 컬럼2, ...) VALUES (값1, 값2, ...);
```

### 예시

```sql
INSERT INTO sensor_data (temperature, humidity)
VALUES (25.3, 60.5);
```

> 💡 `id`와 `recorded_at`은 자동으로 채워지므로 생략합니다.

---

**Q1.** 위 INSERT 문을 실행한 뒤 아래 SELECT로 확인해 보세요. 어떤 결과가 나오나요?

```sql
SELECT * FROM sensor_data;
```

```
결과:
```

> ✅ **정답:** id가 1이고, temperature 25.3, humidity 60.5, recorded_at에 현재 시각이 자동으로 들어간 행 1개가 출력된다.

&nbsp;

**Q2.** 아래 데이터를 모두 INSERT 해보세요. (직접 SQL 작성)

| temperature | humidity |
|-------------|----------|
| 27.1 | 58.2 |
| 29.4 | 55.0 |
| 23.8 | 65.3 |
| 31.2 | 50.1 |
| 22.5 | 70.8 |

```sql
-- 여기에 작성:




```

> ✅ **정답:**
> ```sql
> INSERT INTO sensor_data (temperature, humidity) VALUES (27.1, 58.2);
> INSERT INTO sensor_data (temperature, humidity) VALUES (29.4, 55.0);
> INSERT INTO sensor_data (temperature, humidity) VALUES (23.8, 65.3);
> INSERT INTO sensor_data (temperature, humidity) VALUES (31.2, 50.1);
> INSERT INTO sensor_data (temperature, humidity) VALUES (22.5, 70.8);
> ```

&nbsp;

**Q3.** INSERT 문을 한 번에 여러 행 넣는 방식으로 바꿔보세요.

```sql
INSERT INTO sensor_data (temperature, humidity)
VALUES
    (_____, _____),
    (_____, _____);
```

> ✅ **정답:**
> ```sql
> INSERT INTO sensor_data (temperature, humidity)
> VALUES
>     (27.1, 58.2),
>     (29.4, 55.0);
> ```

---

## 3. 실습 2 — SELECT / WHERE / ORDER BY (15분)

### 기본 문법

```sql
SELECT 컬럼명 FROM 테이블명 WHERE 조건 ORDER BY 컬럼명 ASC|DESC;
```

---

**Q4.** 모든 데이터를 조회하되, temperature와 humidity 컬럼만 가져오는 SQL을 작성해보세요.

```sql
-- 여기에 작성:
```

> ✅ **정답:**
> ```sql
> SELECT temperature, humidity FROM sensor_data;
> ```

&nbsp;

**Q5.** 온도가 28도 이상인 데이터만 조회하는 SQL을 작성해보세요.

```sql
-- 여기에 작성:
```

> ✅ **정답:**
> ```sql
> SELECT * FROM sensor_data WHERE temperature >= 28;
> ```

&nbsp;

**Q6.** 온도가 높은 순서대로 정렬해서 조회하는 SQL을 작성해보세요.

```sql
-- 여기에 작성:
```

> ✅ **정답:**
> ```sql
> SELECT * FROM sensor_data ORDER BY temperature DESC;
> ```

&nbsp;

**Q7.** 온도가 25도 이상이면서 습도가 60% 이하인 데이터만 조회해보세요.

```sql
-- 여기에 작성:
```

> ✅ **정답:**
> ```sql
> SELECT * FROM sensor_data WHERE temperature >= 25 AND humidity <= 60;
> ```

&nbsp;

**Q8.** 가장 최근에 입력된 데이터 3개만 가져오는 SQL을 작성해보세요.

```sql
-- 여기에 작성:
```

> ✅ **정답:**
> ```sql
> SELECT * FROM sensor_data ORDER BY recorded_at DESC LIMIT 3;
> ```

---

## 4. 실습 3 — 집계 함수 (10분)

데이터를 요약해서 하나의 숫자로 만들어주는 함수들입니다.

| 함수 | 의미 | 예시 |
|------|------|------|
| `COUNT(*)` | 행의 개수 | 총 측정 횟수 |
| `AVG(컬럼)` | 평균값 | 평균 온도 |
| `MAX(컬럼)` | 최댓값 | 최고 온도 |
| `MIN(컬럼)` | 최솟값 | 최저 온도 |

---

**Q9.** 지금까지 저장된 데이터가 총 몇 개인지 조회해보세요.

```sql
-- 여기에 작성:
```

> ✅ **정답:**
> ```sql
> SELECT COUNT(*) FROM sensor_data;
> ```

&nbsp;

**Q10.** 온도의 평균, 최댓값, 최솟값을 한 번에 조회해보세요.

```sql
-- 여기에 작성:
```

> ✅ **정답:**
> ```sql
> SELECT AVG(temperature), MAX(temperature), MIN(temperature) FROM sensor_data;
> ```

&nbsp;

**Q11.** 컬럼 이름을 읽기 좋게 별칭(alias)으로 바꿔보세요. `AS` 키워드를 사용합니다.

```sql
SELECT
    AVG(temperature) AS ________,
    MAX(temperature) AS ________,
    MIN(temperature) AS ________
FROM sensor_data;
```

> ✅ **정답 예시:**
> ```sql
> SELECT
>     AVG(temperature) AS 평균온도,
>     MAX(temperature) AS 최고온도,
>     MIN(temperature) AS 최저온도
> FROM sensor_data;
> ```

&nbsp;

**Q12.** (심화) 온도가 28도 이상인 데이터의 평균 습도는 얼마인가요?

```sql
-- 여기에 작성:
```

> ✅ **정답:**
> ```sql
> SELECT AVG(humidity) FROM sensor_data WHERE temperature >= 28;
> ```

---

## 5. 정리 (5분)

```
INSERT INTO 테이블 (컬럼) VALUES (값)  → 데이터 추가
SELECT 컬럼 FROM 테이블                → 조회
WHERE 조건                             → 필터
ORDER BY 컬럼 DESC/ASC                 → 정렬
LIMIT 숫자                             → 개수 제한
COUNT / AVG / MAX / MIN                → 집계
```

### 최종 확인 — 스스로 만들어보기

**Q13.** 습도가 낮은 순서대로 상위 3개 데이터를 가져오는 SQL을 작성해보세요.

```sql
-- 여기에 작성:
```

> ✅ **정답:**
> ```sql
> SELECT * FROM sensor_data ORDER BY humidity ASC LIMIT 3;
> ```

---

## 📌 힌트 카드 (막히는 학생에게 순서대로 건네주세요)

> **교사 안내:** 학생이 막힐 때 정답을 바로 주지 말고 아래 카드를 순서대로 활용하세요.

---

### 힌트 카드 A — INSERT 막힐 때

```
힌트 1: INSERT INTO 다음에 테이블 이름을 쓰고,
        괄호 안에 넣고 싶은 컬럼 이름을 씁니다.

힌트 2: VALUES 다음 괄호 안에 각 컬럼에 넣을 값을
        같은 순서로 씁니다.

힌트 3: INSERT INTO sensor_data (temperature, humidity)
        VALUES ( ___ , ___ );
        빈칸에 숫자만 채우면 됩니다.
```

---

### 힌트 카드 B — WHERE 막힐 때

```
힌트 1: WHERE은 SELECT 뒤, FROM 테이블명 뒤에 씁니다.

힌트 2: 조건은 수학 부등호와 같습니다.
        >= (이상),  <= (이하),  = (같음),  != (다름)

힌트 3: SELECT * FROM sensor_data WHERE temperature ___ 28;
        빈칸에 부등호 기호만 채우면 됩니다.
```

---

### 힌트 카드 C — ORDER BY 막힐 때

```
힌트 1: ORDER BY는 SELECT 문 맨 뒤에 씁니다.

힌트 2: 높은 순서 = DESC (내림차순)
        낮은 순서 = ASC  (오름차순)

힌트 3: SELECT * FROM sensor_data ORDER BY temperature ___;
        빈칸에 DESC 또는 ASC를 쓰면 됩니다.
```

---

### 힌트 카드 D — 집계 함수 막힐 때

```
힌트 1: AVG(), MAX(), MIN() 은 SELECT 바로 뒤에 씁니다.

힌트 2: SELECT AVG(___) FROM sensor_data;
        괄호 안에 컬럼 이름을 씁니다.

힌트 3: SELECT AVG(temperature) FROM sensor_data;
        이미 완성된 예시입니다. 다른 함수도 같은 방식입니다.
```

---

## 📎 참고 자료

| 자료 | 링크 | 내용 |
|------|------|------|
| MySQL 공식 문서 — SELECT | https://dev.mysql.com/doc/refman/8.0/en/select.html | SELECT 전체 문법 |
| MySQL 공식 문서 — INSERT | https://dev.mysql.com/doc/refman/8.0/en/insert.html | INSERT 전체 문법 |
| MySQL 집계 함수 | https://dev.mysql.com/doc/refman/8.0/en/aggregate-functions.html | COUNT, AVG, MAX, MIN |
| W3Schools MySQL (한국어 친화적) | https://www.w3schools.com/mysql/ | SQL 문법 예제 모음 |
| SQL Zoo (실습 사이트) | https://sqlzoo.net/ | 브라우저에서 SQL 직접 실습 |

---

> 💡 **다음 시간 예고:** 6교시에서는 Python에서 MySQL에 직접 연결해 지금까지 배운 SQL을 코드로 실행합니다. Flask와 MySQL을 연결하는 핵심 교시예요.
