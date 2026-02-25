# 4단계: timeline 엔드포인트 DB 연동

## 목표

timeline 라우트를 DB 연동으로 완성한다. 이 단계를 마치면 모든 엔드포인트가 DB 기반으로 동작한다.

## 변경 전후 비교

| 변경 전 (3단계 상태) | 변경 후 (DB) |
|---------------------|-------------|
| <pre>@app.route('/timeline/&lt;int:user_id&gt;', methods=['GET'])<br>def timeline(user_id):<br>    return jsonify({<br>        'user_id'  : user_id,<br>        'timeline' : get_timeline(user_id)<br>    })</pre> | 라우트는 동일, `get_timeline` 함수를 추가한다 |

## 1. DB 헬퍼 함수 추가

`create_app` 함수 **위에** (기존 `insert_unfollow` 함수 아래에) 다음 함수를 추가한다:

| 추가할 함수 |
|------------|
| <pre>def get_timeline(user_id):<br>    with current_app.database.connect() as conn:<br>        timeline = conn.execute(text("""<br>            SELECT<br>                t.user_id,<br>                t.tweet<br>            FROM tweets t<br>            LEFT JOIN users_follow_list ufl ON ufl.user_id = :user_id<br>            WHERE t.user_id = :user_id<br>            OR t.user_id = ufl.follow_user_id<br>        """), {'user_id': user_id}).fetchall()<br><br>    return [{<br>        'user_id' : tweet[0],<br>        'tweet'   : tweet[1]<br>    } for tweet in timeline]</pre> |

### SQL 설명

| 구문 | 설명 |
|------|------|
| `FROM tweets t` | tweets 테이블에서 트윗을 가져온다 |
| `LEFT JOIN users_follow_list ufl` | 팔로우 목록을 조인한다 |
| `ON ufl.user_id = :user_id` | 해당 유저가 팔로우하는 사람들을 찾는다 |
| `WHERE t.user_id = :user_id` | 본인의 트윗 포함 |
| `OR t.user_id = ufl.follow_user_id` | 팔로우한 사람의 트윗 포함 |

## 2. 라우트 확인

timeline 라우트는 3단계에서 이미 수정되어 있으므로 변경 불필요:

| timeline 라우트 (변경 없음) |
|---------------------------|
| <pre>@app.route('/timeline/&lt;int:user_id&gt;', methods=['GET'])<br>def timeline(user_id):<br>    return jsonify({<br>        'user_id'  : user_id,<br>        'timeline' : get_timeline(user_id)<br>    })</pre> |

## 3. 수정 후 전체 app.py (최종 완성본)

| app.py |
|--------|
| <pre>from flask import Flask, request, jsonify, current_app<br>from flask.json.provider import DefaultJSONProvider<br>from sqlalchemy import create_engine, text<br><br>class CustomJSONProvider(DefaultJSONProvider):<br>    def default(self, obj):<br>        if isinstance(obj, set):<br>            return list(obj)<br>        return super().default(obj)<br><br>def insert_user(user):<br>    with current_app.database.connect() as conn:<br>        result = conn.execute(text("""<br>            INSERT INTO users (<br>                name,<br>                email,<br>                profile,<br>                hashed_password<br>            ) VALUES (<br>                :name,<br>                :email,<br>                :profile,<br>                :password<br>            )<br>        """), user)<br>        conn.commit()<br>        return result.lastrowid<br><br>def get_user(user_id):<br>    with current_app.database.connect() as conn:<br>        user = conn.execute(text("""<br>            SELECT<br>                id,<br>                name,<br>                email,<br>                profile<br>            FROM users<br>            WHERE id = :user_id<br>        """), {'user_id': user_id}).fetchone()<br><br>    return {<br>        'id'      : user[0],<br>        'name'    : user[1],<br>        'email'   : user[2],<br>        'profile' : user[3]<br>    } if user else None<br><br>def insert_tweet(user_tweet):<br>    with current_app.database.connect() as conn:<br>        result = conn.execute(text("""<br>            INSERT INTO tweets (<br>                user_id,<br>                tweet<br>            ) VALUES (<br>                :id,<br>                :tweet<br>            )<br>        """), user_tweet)<br>        conn.commit()<br>        return result.rowcount<br><br>def insert_follow(user_follow):<br>    with current_app.database.connect() as conn:<br>        result = conn.execute(text("""<br>            INSERT INTO users_follow_list (<br>                user_id,<br>                follow_user_id<br>            ) VALUES (<br>                :id,<br>                :follow<br>            )<br>        """), user_follow)<br>        conn.commit()<br>        return result.rowcount<br><br>def insert_unfollow(user_unfollow):<br>    with current_app.database.connect() as conn:<br>        result = conn.execute(text("""<br>            DELETE FROM users_follow_list<br>            WHERE user_id = :id<br>            AND follow_user_id = :unfollow<br>        """), user_unfollow)<br>        conn.commit()<br>        return result.rowcount<br><br>def get_timeline(user_id):<br>    with current_app.database.connect() as conn:<br>        timeline = conn.execute(text("""<br>            SELECT<br>                t.user_id,<br>                t.tweet<br>            FROM tweets t<br>            LEFT JOIN users_follow_list ufl ON ufl.user_id = :user_id<br>            WHERE t.user_id = :user_id<br>            OR t.user_id = ufl.follow_user_id<br>        """), {'user_id': user_id}).fetchall()<br><br>    return [{<br>        'user_id' : tweet[0],<br>        'tweet'   : tweet[1]<br>    } for tweet in timeline]<br><br>def create_app(test_config=None):<br>    app = Flask(\_\_name\_\_)<br>    app.json_provider_class = CustomJSONProvider<br>    app.json = CustomJSONProvider(app)<br><br>    if test_config is None:<br>        app.config.from_pyfile("config.py")<br>    else:<br>        app.config.update(test_config)<br><br>    database = create_engine(app.config['DB_URL'], max_overflow=0)<br>    app.database = database<br><br>    @app.route("/ping", methods=['GET'])<br>    def ping():<br>        return "pong"<br><br>    @app.route("/sign-up", methods=['POST'])<br>    def sign_up():<br>        new_user = request.json<br>        new_user_id = insert_user(new_user)<br>        new_user = get_user(new_user_id)<br>        return jsonify(new_user)<br><br>    @app.route('/tweet', methods=['POST'])<br>    def tweet():<br>        user_tweet = request.json<br>        tweet = user_tweet['tweet']<br><br>        if len(tweet) > 300:<br>            return '300자를 초과했습니다.', 400<br><br>        insert_tweet(user_tweet)<br><br>        return '', 200<br><br>    @app.route('/follow', methods=['POST'])<br>    def follow():<br>        payload = request.json<br>        insert_follow(payload)<br><br>        return '', 200<br><br>    @app.route('/unfollow', methods=['POST'])<br>    def unfollow():<br>        payload = request.json<br>        insert_unfollow(payload)<br><br>        return '', 200<br><br>    @app.route('/timeline/&lt;int:user_id&gt;', methods=['GET'])<br>    def timeline(user_id):<br>        return jsonify({<br>            'user_id'  : user_id,<br>            'timeline' : get_timeline(user_id)<br>        })<br><br>    return app</pre> |

## 4. 실행

| 명령어 |
|--------|
| <pre>FLASK_APP=app:create_app FLASK_DEBUG=1 flask run --host=0.0.0.0</pre> |

## 5. httpie 테스트 (전체 시나리오)

DB를 초기화하고 처음부터 테스트한다:

| 명령어 |
|--------|
| <pre>sudo mariadb -u root -p -e "DELETE FROM miniter.tweets; DELETE FROM miniter.users_follow_list; DELETE FROM miniter.users;"</pre> |

### 회원가입 (2단계 복습)

| 순서 | 명령어 | 예상 응답 |
|------|--------|-----------|
| 1 | <pre>http POST http://localhost:5000/sign-up name=sangho email=sangho@test.com password=test1234 profile="hello"</pre> | `{"id":1,"name":"sangho","email":"sangho@test.com","profile":"hello"}` |
| 2 | <pre>http POST http://localhost:5000/sign-up name=kim email=kim@test.com password=test1234 profile="world"</pre> | `{"id":2,"name":"kim","email":"kim@test.com","profile":"world"}` |

### 트윗 작성 (3단계 복습)

| 순서 | 명령어 | 예상 응답 |
|------|--------|-----------|
| 3 | <pre>http POST http://localhost:5000/tweet id:=1 tweet="sangho's tweet"</pre> | `200 OK` |
| 4 | <pre>http POST http://localhost:5000/tweet id:=2 tweet="kim's tweet"</pre> | `200 OK` |

### 팔로우 (3단계 복습)

| 순서 | 명령어 | 예상 응답 |
|------|--------|-----------|
| 5 | <pre>http POST http://localhost:5000/follow id:=1 follow:=2</pre> | `200 OK` |

### 타임라인 테스트 (4단계 핵심)

| 순서 | 테스트 | 명령어 | 예상 응답 |
|------|--------|--------|-----------|
| 6 | sangho의 타임라인 (본인 + kim 팔로우) | <pre>http GET http://localhost:5000/timeline/1</pre> | sangho와 kim의 트윗 모두 표시 |
| 7 | kim의 타임라인 (본인만) | <pre>http GET http://localhost:5000/timeline/2</pre> | kim의 트윗만 표시 |

순서 6 예상 응답:

| 응답 |
|------|
| <pre>{<br>    "timeline": [<br>        {<br>            "tweet": "sangho's tweet",<br>            "user_id": 1<br>        },<br>        {<br>            "tweet": "kim's tweet",<br>            "user_id": 2<br>        }<br>    ],<br>    "user_id": 1<br>}</pre> |

순서 7 예상 응답:

| 응답 |
|------|
| <pre>{<br>    "timeline": [<br>        {<br>            "tweet": "kim's tweet",<br>            "user_id": 2<br>        }<br>    ],<br>    "user_id": 2<br>}</pre> |

### 언팔로우 후 타임라인 변화 확인

| 순서 | 테스트 | 명령어 | 예상 응답 |
|------|--------|--------|-----------|
| 8 | sangho가 kim 언팔로우 | <pre>http POST http://localhost:5000/unfollow id:=1 unfollow:=2</pre> | `200 OK` |
| 9 | sangho 타임라인 재확인 | <pre>http GET http://localhost:5000/timeline/1</pre> | sangho의 트윗만 표시 (kim 트윗 사라짐) |

## 6. 완성 확인 체크리스트

| 엔드포인트 | 메서드 | DB 연동 | 단계 |
|-----------|--------|---------|------|
| `/ping` | GET | 없음 | 1단계 |
| `/sign-up` | POST | `insert_user`, `get_user` | 2단계 |
| `/tweet` | POST | `insert_tweet` | 3단계 |
| `/follow` | POST | `insert_follow` | 3단계 |
| `/unfollow` | POST | `insert_unfollow` | 3단계 |
| `/timeline/<user_id>` | GET | `get_timeline` | 4단계 |

모든 엔드포인트가 DB 기반으로 동작하면 완성!
