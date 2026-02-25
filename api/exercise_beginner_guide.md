# 초급 응용 과제: 기존 API 확장하기

## 목표

4단계까지 완성한 미니터 API에 새로운 엔드포인트를 직접 추가합니다.
기존 코드의 패턴(`insert_user`, `get_user` 등)을 참고하여 스스로 작성합니다.

## 사전 준비

### Git 초기 설정

| 명령어 |
|--------|
| <pre>cd ~/projects/IoT/api<br>git init<br>git add app.py config.py<br>git commit -m "feat: 기본 미니터 API (ping, sign-up, tweet, follow, unfollow, timeline)"</pre> |

### Git 커밋 규칙 (필수)

기능 하나를 완성할 때마다 반드시 커밋합니다. 커밋 이력이 본인의 작업 증거이며, 문제가 생겼을 때 되돌릴 수 있는 안전장치입니다.

**커밋 흐름:**

| 순서 | 내용 |
|------|------|
| 1 | 코드 작성 |
| 2 | flask run 서버 확인 |
| 3 | HTTPie 테스트 성공 |
| 4 | git add app.py |
| 5 | git commit -m "메시지" |

**테스트가 성공한 코드만 커밋합니다. 에러 나는 상태로 커밋하지 마세요.**

**커밋 메시지 형식:**

| 접두어 | 의미 | 예시 |
|--------|------|------|
| feat | 새 기능 추가 | `feat: GET /user 유저 조회 API 추가` |
| fix | 버그 수정 | `fix: tweet 300자 검증 오류 수정` |
| refactor | 코드 개선 | `refactor: follow 중복 코드 정리` |

**과제 하나 = 커밋 하나 (여러 과제를 한꺼번에 커밋하지 않습니다)**

---

## 과제 1: 유저 정보 조회 API

### 요구사항

| 항목 | 내용 |
|------|------|
| 엔드포인트 | `GET /user/<int:user_id>` |
| 기능 | user_id로 유저 정보를 조회합니다 |
| 응답 | id, name, email, profile을 JSON으로 반환합니다 |
| 에러 | 존재하지 않는 유저면 404를 반환합니다 |

### 힌트

| 참고할 기존 코드 |
|----------------|
| <pre># 이미 만들어둔 get_user 함수를 활용합니다<br>def get_user(user_id):<br>    with current_app.database.connect() as conn:<br>        user = conn.execute(text("""<br>            SELECT id, name, email, profile<br>            FROM users<br>            WHERE id = :user_id<br>        """), {'user_id': user_id}).fetchone()<br>    ...</pre> |

### 작성할 코드

`create_app` 안에 새 라우트를 추가합니다:

| 작성할 라우트 |
|-------------|
| <pre>@app.route('/user/&lt;int:user_id&gt;', methods=['GET'])<br>def get_user_info(user_id):<br>    user = get_user(user_id)<br>    if user is None:<br>        return '사용자가 존재하지 않습니다.', 404<br>    return jsonify(user)</pre> |

### httpie 테스트

| 순서 | 테스트 | 명령어 | 예상 응답 |
|------|--------|--------|-----------|
| 1 | 존재하는 유저 조회 | <pre>http GET http://localhost:5000/user/4</pre> | `{"id":4,"name":"sangho","email":"sangho@test.com","profile":"hello"}` |
| 2 | 존재하지 않는 유저 조회 | <pre>http GET http://localhost:5000/user/999</pre> | `404` 사용자가 존재하지 않습니다. |

### Git 커밋

| 명령어 |
|--------|
| <pre>git add app.py<br>git commit -m "feat: GET /user/&lt;user_id&gt; 유저 정보 조회 API 추가"</pre> |

---

## 과제 2: 전체 유저 목록 조회 API

### 요구사항

| 항목 | 내용 |
|------|------|
| 엔드포인트 | `GET /users` |
| 기능 | 모든 유저 목록을 조회합니다 |
| 응답 | id, name, email, profile 목록을 JSON으로 반환합니다 |
| 주의 | **password(hashed_password)는 절대 포함하지 않습니다** |

### 힌트

| 참고할 기존 코드 |
|----------------|
| <pre># get_user는 fetchone()으로 한 명만 가져옵니다<br># 전체 목록은 fetchall()을 사용합니다<br># get_timeline의 fetchall() + 리스트 컴프리헨션 패턴을 참고하세요</pre> |

### 작성할 코드

1. `create_app` **위에** DB 헬퍼 함수를 추가합니다:

| 작성할 함수 |
|------------|
| <pre>def get_all_users():<br>    with current_app.database.connect() as conn:<br>        users = conn.execute(text("""<br>            SELECT<br>                id,<br>                name,<br>                email,<br>                profile<br>            FROM users<br>        """)).fetchall()<br><br>    return [{<br>        'id'      : user[0],<br>        'name'    : user[1],<br>        'email'   : user[2],<br>        'profile' : user[3]<br>    } for user in users]</pre> |

2. `create_app` 안에 라우트를 추가합니다:

| 작성할 라우트 |
|-------------|
| <pre>@app.route('/users', methods=['GET'])<br>def user_list():<br>    return jsonify(get_all_users())</pre> |

### httpie 테스트

| 순서 | 테스트 | 명령어 | 예상 응답 |
|------|--------|--------|-----------|
| 1 | 전체 유저 조회 | <pre>http GET http://localhost:5000/users</pre> | 유저 목록 (password 미포함 확인) |

예상 응답:

| 응답 |
|------|
| <pre>[<br>    {<br>        "id": 4,<br>        "name": "sangho",<br>        "email": "sangho@test.com",<br>        "profile": "hello"<br>    },<br>    {<br>        "id": 5,<br>        "name": "kim",<br>        "email": "kim@test.com",<br>        "profile": "world"<br>    }<br>]</pre> |

### Git 커밋

| 명령어 |
|--------|
| <pre>git add app.py<br>git commit -m "feat: GET /users 전체 유저 목록 조회 (password 제외)"</pre> |

---

## 과제 3: 트윗 삭제 API

### 요구사항

| 항목 | 내용 |
|------|------|
| 엔드포인트 | `DELETE /tweet/<int:tweet_id>` |
| 기능 | tweet_id로 트윗을 삭제합니다 |
| 응답 | 성공 시 200을 반환합니다 |
| 에러 | 존재하지 않는 트윗이면 404를 반환합니다 |

### 힌트

| 참고할 기존 코드 |
|----------------|
| <pre># insert_unfollow의 DELETE SQL 패턴을 참고합니다<br># begin() 방식으로 작성합니다<br>def insert_unfollow(user_unfollow):<br>    with current_app.database.begin() as conn:<br>        result = conn.execute(text("""<br>            DELETE FROM users_follow_list<br>            WHERE user_id = :id<br>            AND follow_user_id = :unfollow<br>        """), user_unfollow)<br>        return result.rowcount</pre> |

### 작성할 코드

1. `create_app` **위에** DB 헬퍼 함수를 추가합니다:

| 작성할 함수 |
|------------|
| <pre>def delete_tweet(tweet_id):<br>    with current_app.database.begin() as conn:<br>        result = conn.execute(text("""<br>            DELETE FROM tweets<br>            WHERE id = :tweet_id<br>        """), {'tweet_id': tweet_id})<br>        return result.rowcount</pre> |

2. `create_app` 안에 라우트를 추가합니다:

| 작성할 라우트 |
|-------------|
| <pre>@app.route('/tweet/&lt;int:tweet_id&gt;', methods=['DELETE'])<br>def delete_tweet_endpoint(tweet_id):<br>    rows = delete_tweet(tweet_id)<br>    if rows == 0:<br>        return '트윗이 존재하지 않습니다.', 404<br>    return '', 200</pre> |

### httpie 테스트

먼저 현재 트윗을 확인합니다:

| 명령어 |
|--------|
| <pre>sudo mariadb -u root -p -e "SELECT * FROM miniter.tweets;"</pre> |

| 순서 | 테스트 | 명령어 | 예상 응답 |
|------|--------|--------|-----------|
| 1 | 트윗 작성 | <pre>http POST http://localhost:5000/tweet id:=4 tweet="delete me"</pre> | `200 OK` |
| 2 | 트윗 확인 | <pre>sudo mariadb -u root -p -e "SELECT * FROM miniter.tweets;"</pre> | 트윗 id 확인 |
| 3 | 트윗 삭제 (id에 맞게 수정) | <pre>http DELETE http://localhost:5000/tweet/1</pre> | `200 OK` |
| 4 | 삭제 확인 | <pre>sudo mariadb -u root -p -e "SELECT * FROM miniter.tweets;"</pre> | 해당 트윗이 사라집니다 |
| 5 | 없는 트윗 삭제 | <pre>http DELETE http://localhost:5000/tweet/999</pre> | `404` 트윗이 존재하지 않습니다. |

### Git 커밋

| 명령어 |
|--------|
| <pre>git add app.py<br>git commit -m "feat: DELETE /tweet 트윗 삭제 기능 추가"</pre> |

---

## 과제 4: 프로필 수정 API (도전)

### 요구사항

| 항목 | 내용 |
|------|------|
| 엔드포인트 | `PUT /user/<int:user_id>` |
| 기능 | 유저의 name, profile을 수정합니다 |
| 요청 본문 | `{"name": "새이름", "profile": "새프로필"}` |
| 응답 | 수정된 유저 정보를 JSON으로 반환합니다 |
| 에러 | 존재하지 않는 유저면 404를 반환합니다 |

### 힌트

| 참고 |
|------|
| <pre># INSERT 대신 UPDATE SQL을 사용합니다<br># UPDATE users SET name = :name, profile = :profile WHERE id = :user_id<br># begin() 방식으로 커밋합니다<br># 수정 후 get_user()로 변경된 정보를 조회하여 반환합니다</pre> |

### 직접 작성해보기

이 과제는 힌트만 제공합니다. 아래를 직접 작성해보세요:

| 작성할 항목 | 설명 |
|------------|------|
| `update_user(user_id, data)` | UPDATE SQL을 실행하는 DB 헬퍼 함수 |
| `PUT /user/<user_id>` 라우트 | 수정 요청을 처리하는 라우트 |

### httpie 테스트

| 순서 | 테스트 | 명령어 | 예상 응답 |
|------|--------|--------|-----------|
| 1 | 수정 전 확인 | <pre>http GET http://localhost:5000/user/4</pre> | 현재 정보 |
| 2 | 프로필 수정 | <pre>http PUT http://localhost:5000/user/4 name="sangho_updated" profile="updated profile"</pre> | 수정된 정보 |
| 3 | 수정 후 확인 | <pre>http GET http://localhost:5000/user/4</pre> | name, profile이 변경되어 있습니다 |
| 4 | 없는 유저 수정 | <pre>http PUT http://localhost:5000/user/999 name="test" profile="test"</pre> | `404` |

### Git 커밋

| 명령어 |
|--------|
| <pre>git add app.py<br>git commit -m "feat: PUT /user/&lt;user_id&gt; 프로필 수정 API 추가"</pre> |

---

## 진행 상황 확인

모든 과제를 완료했다면 커밋 이력을 확인합니다:

| 명령어 |
|--------|
| <pre>git log --oneline</pre> |

예상 결과:

| 커밋 이력 |
|----------|
| <pre>a1b2c3d feat: PUT /user/&lt;user_id&gt; 프로필 수정 API 추가<br>d4e5f6g feat: DELETE /tweet 트윗 삭제 기능 추가<br>h7i8j9k feat: GET /users 전체 유저 목록 조회 (password 제외)<br>l0m1n2o feat: GET /user/&lt;user_id&gt; 유저 정보 조회 API 추가<br>p3q4r5s feat: 기본 미니터 API (ping, sign-up, tweet, follow, unfollow, timeline)</pre> |

## 과제 요약

| 과제 | 엔드포인트 | 메서드 | 난이도 | 핵심 학습 |
|------|-----------|--------|--------|----------|
| 1 | `/user/<user_id>` | GET | 쉬움 | 기존 함수 재활용 |
| 2 | `/users` | GET | 쉬움 | fetchall + 리스트 컴프리헨션 |
| 3 | `/tweet/<tweet_id>` | DELETE | 보통 | DELETE SQL, rowcount 활용 |
| 4 | `/user/<user_id>` | PUT | 도전 | UPDATE SQL, 직접 설계 |
