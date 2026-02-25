# 3단계: create_app 구조로 app.py 재작성

## 사전 확인

config.py가 이미 작성되어 있음:

| config.py |
|-----------|
| <pre>db = {<br>    'user'     : 'root',<br>    'password' : 'test1234',<br>    'host'     : 'localhost',<br>    'port'     : 3306,<br>    'database' : 'miniter'<br>}<br><br>DB_URL = f"mysql+mysqlconnector://{db['user']}:{db['password']}@{db['host']}:{db['port']}/{db['database']}?charset=utf8"</pre> |

config.py에서 `mysql+mysqlconnector`를 사용하므로 패키지 설치 필요:

| 명령어 |
|--------|
| <pre>pip install mysql-connector-python</pre> |

## 1. 기존 파일 백업

| 명령어 |
|--------|
| <pre>cp app.py app.py.bk</pre> |

## 2. 새 app.py 작성

`nano app.py` 로 열어서 아래 내용을 전체 복사하여 붙여넣기:

| app.py |
|--------|
| <pre>from flask import Flask, request, jsonify<br>from flask.json.provider import DefaultJSONProvider<br>from sqlalchemy import create_engine, text<br><br>class CustomJSONProvider(DefaultJSONProvider):<br>    def default(self, obj):<br>        if isinstance(obj, set):<br>            return list(obj)<br>        return super().default(obj)<br><br>def create_app(test_config=None):<br>    app = Flask(\_\_name\_\_)<br>    app.json_provider_class = CustomJSONProvider<br>    app.json = CustomJSONProvider(app)<br><br>    if test_config is None:<br>        app.config.from_pyfile("config.py")<br>    else:<br>        app.config.update(test_config)<br><br>    database = create_engine(app.config['DB_URL'], max_overflow=0)<br>    app.database = database<br><br>    # 메모리 기반 데이터 (4단계에서 DB로 교체 예정)<br>    app.users = {}<br>    app.id_count = 1<br>    app.tweets = []<br><br>    @app.route("/ping", methods=['GET'])<br>    def ping():<br>        return "pong"<br><br>    @app.route("/sign-up", methods=['POST'])<br>    def sign_up():<br>        new_user = request.json<br>        new_user["id"] = app.id_count<br>        app.users[app.id_count] = new_user<br>        app.id_count += 1<br>        return jsonify(new_user)<br><br>    @app.route('/tweet', methods=['POST'])<br>    def tweet():<br>        payload = request.json<br>        user_id = int(payload['id'])<br>        tweet = payload['tweet']<br><br>        if user_id not in app.users:<br>            return '사용자가 존재하지 않습니다.', 400<br>        if len(tweet) > 300:<br>            return '300자를 초과했습니다.', 400<br><br>        app.tweets.append({<br>            'user_id': user_id,<br>            'tweet': tweet<br>        })<br>        return '', 200<br><br>    @app.route('/follow', methods=['POST'])<br>    def follow():<br>        payload = request.json<br>        user_id = int(payload['id'])<br>        user_id_to_follow = int(payload['follow'])<br><br>        if user_id not in app.users or user_id_to_follow not in app.users:<br>            return '사용자가 존재하지 않습니다.', 400<br><br>        user = app.users[user_id]<br>        user.setdefault('follow', set()).add(user_id_to_follow)<br>        return jsonify(user)<br><br>    @app.route('/unfollow', methods=['POST'])<br>    def unfollow():<br>        payload = request.json<br>        user_id = int(payload['id'])<br>        user_id_to_follow = int(payload['unfollow'])<br><br>        if user_id not in app.users or user_id_to_follow not in app.users:<br>            return '사용자가 존재하지 않습니다.', 400<br><br>        user = app.users[user_id]<br>        user.setdefault('follow', set()).discard(user_id_to_follow)<br>        return jsonify(user)<br><br>    @app.route('/timeline/&lt;int:user_id&gt;', methods=['GET'])<br>    def timeline(user_id):<br>        if user_id not in app.users:<br>            return '사용자가 존재하지 않습니다.', 400<br><br>        follow_list = app.users[user_id].get('follow', set())<br>        follow_list.add(user_id)<br>        timeline = [t for t in app.tweets if t['user_id'] in follow_list]<br><br>        return jsonify({<br>            'user_id': user_id,<br>            'timeline': timeline<br>        })<br><br>    return app</pre> |

## 3. 실행

| 명령어 |
|--------|
| <pre>FLASK_APP=app:create_app FLASK_DEBUG=1 flask run --host=0.0.0.0</pre> |

## 4. 동작 확인 (다른 터미널에서)

| 테스트 | 명령어 | 예상 응답 |
|--------|--------|-----------|
| ping | <pre>http GET http://localhost:5000/ping</pre> | `pong` |
| 회원가입 | <pre>http POST http://localhost:5000/sign-up name=sangho email=sangho@test.com password=test1234 profile="hello"</pre> | `{"email":"sangho@test.com","id":1,"name":"sangho","password":"test1234","profile":"hello"}` |
| 트윗 | <pre>http POST http://localhost:5000/tweet id:=1 tweet="hello world"</pre> | `200 OK` |
| 타임라인 | <pre>http GET http://localhost:5000/timeline/1</pre> | `{"timeline":[{"tweet":"hello world","user_id":1}],"user_id":1}` |

## 5. 문제 발생 시 복구

| 명령어 |
|--------|
| <pre>cp app.py.bk app.py</pre> |

## 변경 요약

| 변경 전 | 변경 후 | 이유 |
|---------|---------|------|
| 전역 `app = Flask(__name__)` | `create_app` 함수 안에서 생성 | 팩토리 패턴 사용 |
| 라우트가 전역에 정의 | `create_app` 안으로 이동 | 팩토리 패턴에서는 라우트도 함수 안에 있어야 함 |
| `import json` | 삭제 | 사용하지 않음 |
| `python3 app.py`로 실행 | `FLASK_APP=app:create_app flask run`으로 실행 | 팩토리 패턴 실행 방식 |
| config.py 없음 | config.py에서 DB_URL 읽기 | DB 연동 준비 |
