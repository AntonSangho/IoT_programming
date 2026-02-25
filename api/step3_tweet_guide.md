# 3단계: tweet, follow, unfollow 엔드포인트 DB 연동

## 목표

tweet, follow, unfollow 라우트를 메모리 기반에서 MariaDB 연동으로 변경한다.

## 변경 전후 비교

| 엔드포인트 | 변경 전 (메모리) | 변경 후 (DB) |
|-----------|------------------|-------------|
| tweet | <pre>app.tweets.append({...})</pre> | <pre>insert_tweet(user_tweet)</pre> |
| follow | <pre>user.setdefault('follow', set()).add(...)</pre> | <pre>insert_follow(payload)</pre> |
| unfollow | <pre>user.setdefault('follow', set()).discard(...)</pre> | <pre>insert_unfollow(payload)</pre> |

## 1. DB 헬퍼 함수 추가

`create_app` 함수 **위에** (기존 `get_user` 함수 아래에) 다음 세 함수를 추가한다:

| 추가할 함수 |
|------------|
| <pre>def insert_tweet(user_tweet):<br>    with current_app.database.connect() as conn:<br>        result = conn.execute(text("""<br>            INSERT INTO tweets (<br>                user_id,<br>                tweet<br>            ) VALUES (<br>                :id,<br>                :tweet<br>            )<br>        """), user_tweet)<br>        conn.commit()<br>        return result.rowcount<br><br>def insert_follow(user_follow):<br>    with current_app.database.connect() as conn:<br>        result = conn.execute(text("""<br>            INSERT INTO users_follow_list (<br>                user_id,<br>                follow_user_id<br>            ) VALUES (<br>                :id,<br>                :follow<br>            )<br>        """), user_follow)<br>        conn.commit()<br>        return result.rowcount<br><br>def insert_unfollow(user_unfollow):<br>    with current_app.database.connect() as conn:<br>        result = conn.execute(text("""<br>            DELETE FROM users_follow_list<br>            WHERE user_id = :id<br>            AND follow_user_id = :unfollow<br>        """), user_unfollow)<br>        conn.commit()<br>        return result.rowcount</pre> |

## 2. 라우트 수정

`create_app` 안의 tweet, follow, unfollow 라우트를 수정한다:

### tweet

| 수정 전 | 수정 후 |
|---------|---------|
| <pre>@app.route('/tweet', methods=['POST'])<br>def tweet():<br>    payload = request.json<br>    user_id = int(payload['id'])<br>    tweet = payload['tweet']<br><br>    if user_id not in app.users:<br>        return '사용자가 존재하지 않습니다.', 400<br>    if len(tweet) > 300:<br>        return '300자를 초과했습니다.', 400<br><br>    app.tweets.append({<br>        'user_id': user_id,<br>        'tweet': tweet<br>    })<br>    return '', 200</pre> | <pre>@app.route('/tweet', methods=['POST'])<br>def tweet():<br>    user_tweet = request.json<br>    tweet = user_tweet['tweet']<br><br>    if len(tweet) > 300:<br>        return '300자를 초과했습니다.', 400<br><br>    insert_tweet(user_tweet)<br><br>    return '', 200</pre> |

### follow

| 수정 전 | 수정 후 |
|---------|---------|
| <pre>@app.route('/follow', methods=['POST'])<br>def follow():<br>    payload = request.json<br>    user_id = int(payload['id'])<br>    user_id_to_follow = int(payload['follow'])<br><br>    if user_id not in app.users or ...<br>        return '사용자가 존재하지 않습니다.', 400<br><br>    user = app.users[user_id]<br>    user.setdefault('follow', set()).add(...)<br>    return jsonify(user)</pre> | <pre>@app.route('/follow', methods=['POST'])<br>def follow():<br>    payload = request.json<br>    insert_follow(payload)<br><br>    return '', 200</pre> |

### unfollow

| 수정 전 | 수정 후 |
|---------|---------|
| <pre>@app.route('/unfollow', methods=['POST'])<br>def unfollow():<br>    payload = request.json<br>    user_id = int(payload['id'])<br>    user_id_to_follow = int(payload['unfollow'])<br><br>    if user_id not in app.users or ...<br>        return '사용자가 존재하지 않습니다.', 400<br><br>    user = app.users[user_id]<br>    user.setdefault('follow', set()).discard(...)<br>    return jsonify(user)</pre> | <pre>@app.route('/unfollow', methods=['POST'])<br>def unfollow():<br>    payload = request.json<br>    insert_unfollow(payload)<br><br>    return '', 200</pre> |

## 3. 메모리 변수 정리

`create_app` 안의 나머지 메모리 변수를 삭제한다:

| 삭제할 코드 |
|------------|
| <pre>app.tweets = []  # 삭제</pre> |

## 4. 수정 후 전체 app.py

| app.py |
|--------|
| <pre>from flask import Flask, request, jsonify, current_app<br>from flask.json.provider import DefaultJSONProvider<br>from sqlalchemy import create_engine, text<br><br>class CustomJSONProvider(DefaultJSONProvider):<br>    def default(self, obj):<br>        if isinstance(obj, set):<br>            return list(obj)<br>        return super().default(obj)<br><br>def insert_user(user):<br>    with current_app.database.connect() as conn:<br>        result = conn.execute(text("""<br>            INSERT INTO users (<br>                name,<br>                email,<br>                profile,<br>                hashed_password<br>            ) VALUES (<br>                :name,<br>                :email,<br>                :profile,<br>                :password<br>            )<br>        """), user)<br>        conn.commit()<br>        return result.lastrowid<br><br>def get_user(user_id):<br>    with current_app.database.connect() as conn:<br>        user = conn.execute(text("""<br>            SELECT<br>                id,<br>                name,<br>                email,<br>                profile<br>            FROM users<br>            WHERE id = :user_id<br>        """), {'user_id': user_id}).fetchone()<br><br>    return {<br>        'id'      : user[0],<br>        'name'    : user[1],<br>        'email'   : user[2],<br>        'profile' : user[3]<br>    } if user else None<br><br>def insert_tweet(user_tweet):<br>    with current_app.database.connect() as conn:<br>        result = conn.execute(text("""<br>            INSERT INTO tweets (<br>                user_id,<br>                tweet<br>            ) VALUES (<br>                :id,<br>                :tweet<br>            )<br>        """), user_tweet)<br>        conn.commit()<br>        return result.rowcount<br><br>def insert_follow(user_follow):<br>    with current_app.database.connect() as conn:<br>        result = conn.execute(text("""<br>            INSERT INTO users_follow_list (<br>                user_id,<br>                follow_user_id<br>            ) VALUES (<br>                :id,<br>                :follow<br>            )<br>        """), user_follow)<br>        conn.commit()<br>        return result.rowcount<br><br>def insert_unfollow(user_unfollow):<br>    with current_app.database.connect() as conn:<br>        result = conn.execute(text("""<br>            DELETE FROM users_follow_list<br>            WHERE user_id = :id<br>            AND follow_user_id = :unfollow<br>        """), user_unfollow)<br>        conn.commit()<br>        return result.rowcount<br><br>def create_app(test_config=None):<br>    app = Flask(\_\_name\_\_)<br>    app.json_provider_class = CustomJSONProvider<br>    app.json = CustomJSONProvider(app)<br><br>    if test_config is None:<br>        app.config.from_pyfile("config.py")<br>    else:<br>        app.config.update(test_config)<br><br>    database = create_engine(app.config['DB_URL'], max_overflow=0)<br>    app.database = database<br><br>    @app.route("/ping", methods=['GET'])<br>    def ping():<br>        return "pong"<br><br>    @app.route("/sign-up", methods=['POST'])<br>    def sign_up():<br>        new_user = request.json<br>        new_user_id = insert_user(new_user)<br>        new_user = get_user(new_user_id)<br>        return jsonify(new_user)<br><br>    @app.route('/tweet', methods=['POST'])<br>    def tweet():<br>        user_tweet = request.json<br>        tweet = user_tweet['tweet']<br><br>        if len(tweet) > 300:<br>            return '300자를 초과했습니다.', 400<br><br>        insert_tweet(user_tweet)<br><br>        return '', 200<br><br>    @app.route('/follow', methods=['POST'])<br>    def follow():<br>        payload = request.json<br>        insert_follow(payload)<br><br>        return '', 200<br><br>    @app.route('/unfollow', methods=['POST'])<br>    def unfollow():<br>        payload = request.json<br>        insert_unfollow(payload)<br><br>        return '', 200<br><br>    @app.route('/timeline/&lt;int:user_id&gt;', methods=['GET'])<br>    def timeline(user_id):<br>        return jsonify({<br>            'user_id'  : user_id,<br>            'timeline' : get_timeline(user_id)<br>        })<br><br>    return app</pre> |

## 5. 실행

| 명령어 |
|--------|
| <pre>FLASK_APP=app:create_app FLASK_DEBUG=1 flask run --host=0.0.0.0</pre> |

## 6. httpie 테스트

테스트 전에 DB에 유저가 있는지 확인:

| 명령어 |
|--------|
| <pre>sudo mariadb -u root -p -e "SELECT * FROM miniter.users;"</pre> |

유저가 없으면 먼저 회원가입:

| 순서 | 명령어 |
|------|--------|
| 1 | <pre>http POST http://localhost:5000/sign-up name=sangho email=sangho@test.com password=test1234 profile="hello"</pre> |
| 2 | <pre>http POST http://localhost:5000/sign-up name=kim email=kim@test.com password=test1234 profile="world"</pre> |

### tweet 테스트

| 순서 | 테스트 | 명령어 | 예상 응답 |
|------|--------|--------|-----------|
| 1 | 트윗 작성 | <pre>http POST http://localhost:5000/tweet id:=1 tweet="hello world"</pre> | `200 OK` |
| 2 | 300자 초과 테스트 | <pre>http POST http://localhost:5000/tweet id:=1 tweet="$(python3 -c 'print("a"*301)')"</pre> | `400 에러` |

DB 확인:

| 명령어 |
|--------|
| <pre>sudo mariadb -u root -p -e "SELECT * FROM miniter.tweets;"</pre> |

### follow 테스트

| 순서 | 테스트 | 명령어 | 예상 응답 |
|------|--------|--------|-----------|
| 1 | sangho(1)가 kim(2)를 팔로우 | <pre>http POST http://localhost:5000/follow id:=1 follow:=2</pre> | `200 OK` |

DB 확인:

| 명령어 |
|--------|
| <pre>sudo mariadb -u root -p -e "SELECT * FROM miniter.users_follow_list;"</pre> |

예상 결과:

| user_id | follow_user_id |
|---------|----------------|
| 1 | 2 |

### unfollow 테스트

| 순서 | 테스트 | 명령어 | 예상 응답 |
|------|--------|--------|-----------|
| 1 | sangho(1)가 kim(2)를 언팔로우 | <pre>http POST http://localhost:5000/unfollow id:=1 unfollow:=2</pre> | `200 OK` |

DB 확인:

| 명령어 |
|--------|
| <pre>sudo mariadb -u root -p -e "SELECT * FROM miniter.users_follow_list;"</pre> |

예상 결과: 빈 테이블 (삭제됨)

## 참고: timeline 라우트

timeline은 아직 `get_timeline` 함수가 없으므로 4단계에서 구현한다. 현재 상태에서 `/timeline/1`을 호출하면 에러가 발생한다.
