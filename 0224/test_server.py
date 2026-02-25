from flask import Flask

app = Flask(__name__)

# ESP32가 접속 테스트할 엔드포인트
@app.route('/ping')
def ping():
    print("ESP32에서 접속!")
    return "pong"

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
