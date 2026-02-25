# 2단계: 회원가입 엔드포인트 DB 연동

## 목표

sign-up 라우트를 메모리 기반에서 MariaDB 연동으로 변경한다.

## 변경 전후 비교

| 변경 전 (메모리) | 변경 후 (DB) |
|------------------|-------------|
| <pre>@app.route("/sign-up", methods=['POST'])<br>def sign_up():<br>    new_user = request.json<br>    new_user["id"] = app.id_count<br>    app.users[app.id_count] = new_user<br>    app.id_count += 1<br>    return jsonify(new_user)</pre> | <pre>@app.route("/sign-up", methods=['POST'])<br>def sign_up():<br>    new_user = request.json<br>    new_user_id = insert_user(new_user)<br>    new_user = get_user(new_user_id)<br>    return jsonify(new_user)</pre> |

## 0. MariaDB root 비밀번호 설정

Flask에서 DB에 접속하려면 root 계정의 비밀번호 인증을 허용해야 한다:

| 명령어 |
|--------|
| <pre>sudo mariadb -u root</pre> |

| SQL |
|-----|
| <pre>ALTER USER 'root'@'localhost' IDENTIFIED BY 'test1234';<br>FLUSH PRIVILEGES;<br>EXIT;</pre> |

설정 후 비밀번호로 접속 확인:

| 명령어 |
|--------|
| <pre>mariadb -u root -p</pre> |

`test1234` 입력 후 접속되면 성공.

## 1. DB 헬퍼 함수 추가

`create_app` 함수 **위에** 다음 두 함수를 추가한다:

| 추가할 함수 |
|------------|
| <pre>from flask import Flask, request, jsonify, current_app  # current_app 추가<br><br>def insert_user(user):<br>    with current_app.database.connect() as conn:<br>        result = conn.execute(text("""<br>            INSERT INTO users (<br>                name,<br>                email,<br>                profile,<br>                hashed_password<br>            ) VALUES (<br>                :name,<br>                :email,<br>                :profile,<br>                :password<br>            )<br>        """), user)<br>        conn.commit()<br>        return result.lastrowid<br><br>def get_user(user_id):<br>    with current_app.database.connect() as conn:<br>        user = conn.execute(text("""<br>            SELECT<br>                id,<br>                name,<br>                email,<br>                profile<br>            FROM users<br>            WHERE id = :user_id<br>        """), {'user_id': user_id}).fetchone()<br><br>    return {<br>        'id'      : user[0],<br>        'name'    : user[1],<br>        'email'   : user[2],<br>        'profile' : user[3]<br>    } if user else None</pre> |

## 2. sign-up 라우트 수정

`create_app` 안의 sign-up 라우트를 수정한다:

| 수정 전 | 수정 후 |
|---------|---------|
| <pre>@app.route("/sign-up", methods=['POST'])<br>def sign_up():<br>    new_user = request.json<br>    new_user["id"] = app.id_count<br>    app.users[app.id_count] = new_user<br>    app.id_count += 1<br>    return jsonify(new_user)</pre> | <pre>@app.route("/sign-up", methods=['POST'])<br>def sign_up():<br>    new_user = request.json<br>    new_user_id = insert_user(new_user)<br>    new_user = get_user(new_user_id)<br>    return jsonify(new_user)</pre> |

## 3. 메모리 변수 정리

`create_app` 안의 메모리 변수 중 users 관련 항목을 삭제한다:

| 삭제할 코드 |
|------------|
| <pre>app.users = {}    # 삭제<br>app.id_count = 1  # 삭제</pre> |

`app.tweets = []`는 아직 남겨둔다 (3단계에서 처리).

## 4. 수정 후 전체 app.py

| app.py |
|--------|
| <pre>from flask import Flask, request, jsonify, current_app<br>from flask.json.provider import DefaultJSONProvider<br>from sqlalchemy import create_engine, text<br><br>class CustomJSONProvider(DefaultJSONProvider):<br>    def default(self, obj):<br>        if isinstance(obj, set):<br>            return list(obj)<br>        return super().default(obj)<br><br>def insert_user(user):<br>    with current_app.database.connect() as conn:<br>        result = conn.execute(text("""<br>            INSERT INTO users (<br>                name,<br>                email,<br>                profile,<br>                hashed_password<br>            ) VALUES (<br>                :name,<br>                :email,<br>                :profile,<br>                :password<br>            )<br>        """), user)<br>        conn.commit()<br>        return result.lastrowid<br><br>def get_user(user_id):<br>    with current_app.database.connect() as conn:<br>        user = conn.execute(text("""<br>            SELECT<br>                id,<br>                name,<br>                email,<br>                profile<br>            FROM users<br>            WHERE id = :user_id<br>        """), {'user_id': user_id}).fetchone()<br><br>    return {<br>        'id'      : user[0],<br>        'name'    : user[1],<br>        'email'   : user[2],<br>        'profile' : user[3]<br>    } if user else None<br><br>def create_app(test_config=None):<br>    app = Flask(\_\_name\_\_)<br>    app.json_provider_class = CustomJSONProvider<br>    app.json = CustomJSONProvider(app)<br><br>    if test_config is None:<br>        app.config.from_pyfile("config.py")<br>    else:<br>        app.config.update(test_config)<br><br>    database = create_engine(app.config['DB_URL'], max_overflow=0)<br>    app.database = database<br><br>    # 메모리 기반 데이터 (3단계에서 DB로 교체 예정)<br>    app.tweets = []<br><br>    @app.route("/ping", methods=['GET'])<br>    def ping():<br>        return "pong"<br><br>    @app.route("/sign-up", methods=['POST'])<br>    def sign_up():<br>        new_user = request.json<br>        new_user_id = insert_user(new_user)<br>        new_user = get_user(new_user_id)<br>        return jsonify(new_user)<br><br>    @app.route('/tweet', methods=['POST'])<br>    def tweet():<br>        payload = request.json<br>        user_id = int(payload['id'])<br>        tweet = payload['tweet']<br><br>        if user_id not in app.users:<br>            return '사용자가 존재하지 않습니다.', 400<br>        if len(tweet) > 300:<br>            return '300자를 초과했습니다.', 400<br><br>        app.tweets.append({<br>            'user_id': user_id,<br>            'tweet': tweet<br>        })<br>        return '', 200<br><br>    @app.route('/follow', methods=['POST'])<br>    def follow():<br>        payload = request.json<br>        user_id = int(payload['id'])<br>        user_id_to_follow = int(payload['follow'])<br><br>        if user_id not in app.users or user_id_to_follow not in app.users:<br>            return '사용자가 존재하지 않습니다.', 400<br><br>        user = app.users[user_id]<br>        user.setdefault('follow', set()).add(user_id_to_follow)<br>        return jsonify(user)<br><br>    @app.route('/unfollow', methods=['POST'])<br>    def unfollow():<br>        payload = request.json<br>        user_id = int(payload['id'])<br>        user_id_to_follow = int(payload['unfollow'])<br><br>        if user_id not in app.users or user_id_to_follow not in app.users:<br>            return '사용자가 존재하지 않습니다.', 400<br><br>        user = app.users[user_id]<br>        user.setdefault('follow', set()).discard(user_id_to_follow)<br>        return jsonify(user)<br><br>    @app.route('/timeline/&lt;int:user_id&gt;', methods=['GET'])<br>    def timeline(user_id):<br>        if user_id not in app.users:<br>            return '사용자가 존재하지 않습니다.', 400<br><br>        follow_list = app.users[user_id].get('follow', set())<br>        follow_list.add(user_id)<br>        timeline = [t for t in app.tweets if t['user_id'] in follow_list]<br><br>        return jsonify({<br>            'user_id': user_id,<br>            'timeline': timeline<br>        })<br><br>    return app</pre> |

## 5. 실행

| 명령어 |
|--------|
| <pre>FLASK_APP=app:create_app FLASK_DEBUG=1 flask run --host=0.0.0.0</pre> |

## 6. httpie 테스트

| 순서 | 테스트 | 명령어 | 예상 응답 |
|------|--------|--------|-----------|
| 1 | ping 확인 | <pre>http GET http://localhost:5000/ping</pre> | `pong` |
| 2 | 회원가입 | <pre>http POST http://localhost:5000/sign-up name=sangho email=sangho@test.com password=test1234 profile="hello"</pre> | `{"id":1,"name":"sangho","email":"sangho@test.com","profile":"hello"}` |
| 3 | 중복 이메일 테스트 | <pre>http POST http://localhost:5000/sign-up name=sangho2 email=sangho@test.com password=test1234 profile="hi"</pre> | `500 에러` (UNIQUE KEY 제약조건) |
| 4 | 두번째 회원가입 | <pre>http POST http://localhost:5000/sign-up name=kim email=kim@test.com password=test1234 profile="world"</pre> | `{"id":2,"name":"kim","email":"kim@test.com","profile":"world"}` |

## 7. DB에서 직접 확인

| 명령어 |
|--------|
| <pre>sudo mariadb -u root -p -e "SELECT * FROM miniter.users;"</pre> |

비밀번호 `test1234`를 입력한다.

예상 결과:

| id | name | email | profile | hashed_password | created_at |
|----|------|-------|---------|-----------------|------------|
| 1 | sangho | sangho@test.com | hello | test1234 | 2026-02-19 ... |
| 2 | kim | kim@test.com | world | test1234 | 2026-02-19 ... |

## current_app 동작 원리

| 위치 | DB 접근 방법 | 설명 |
|------|-------------|------|
| `create_app` 안 | `app.database` | 직접 접근 가능 (지역 변수) |
| `create_app` 밖 (`insert_user`, `get_user`) | `current_app.database` | Flask가 제공하는 프록시로 접근 |
