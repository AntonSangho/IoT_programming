#-*- coding: utf-8 -*-
from flask import Flask, render_template
import http.client
import urllib.request
from urllib.error import HTTPError, URLError

deviceIp = "192.168.0.28"
portnum = "80"

base_url = "http://" + deviceIp + ":" + portnum
events_url = base_url + "/events"

# wlan0 IP를 source로 지정하여 ESP8266과 무선으로 통신
source_ip = "192.168.0.81"

class BoundHTTPConnection(http.client.HTTPConnection):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, source_address=(source_ip, 0), **kwargs)

class BoundHTTPHandler(urllib.request.HTTPHandler):
    def http_open(self, req):
        return self.do_open(BoundHTTPConnection, req)

opener = urllib.request.build_opener(BoundHTTPHandler)

app = Flask(__name__, template_folder=".")

@app.route('/events')
def getevents():
    try:
        u = opener.open(events_url, timeout=5)
        data = u.read()
    except HTTPError as e:
        print("HTTP error: %d" % e.code)
        data = '{"error": "HTTP error"}'
    except URLError as e:
        print("Network error: %s" % e.reason)
        data = '{"error": "ESP8266 연결 실패"}'
    return data

@app.route('/')
def dht22chart():
    return render_template("dhtchart.html")

if __name__ == '__main__':
    app.run(host="0.0.0.0", port=5000)
