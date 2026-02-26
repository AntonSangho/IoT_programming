# 라즈베리파이 유선/무선 동시 사용 시 ESP 통신 문제 해결 가이드

## 문제 상황

라즈베리파이에 유선(eth0)과 무선(wlan0)이 동시에 연결된 상태에서, ESP8266/ESP32와 ping 또는 HTTP 통신이 안 되는 문제.

```
훈련생 데스크탑 ──(유선)──> 라즈베리파이 ──(무선)──> ESP8266/ESP32
                  SSH 접속 (eth0)         HTTP 통신 (wlan0)
```

## 사전 준비

이 가이드의 모든 작업은 **훈련생 데스크탑에서 유선(eth0) SSH로 라즈베리파이에 접속**하여 진행합니다.

```bash
ssh pi@<라즈베리파이 유선 IP>
```

예시:
```bash
ssh pi@192.168.0.189
```

> 라즈베리파이의 유선 IP를 모르는 경우, 라즈베리파이에 모니터와 키보드를 직접 연결하여 `ip addr` 명령으로 eth0의 IP를 확인하세요.

또한 라즈베리파이가 WiFi에 연결되어 있어야 합니다. 아직 연결하지 않았다면:

```bash
sudo nmcli device wifi connect "<WiFi SSID>" password "<비밀번호>"
```

예시:
```bash
sudo nmcli device wifi connect "5층" password "48864886"
```

## 원인

라즈베리파이는 기본적으로 **유선(eth0)을 우선** 사용합니다. ESP는 WiFi 기기이므로 wlan0을 통해서만 통신이 가능한데, 라즈베리파이가 eth0으로 요청을 보내기 때문에 ESP에 도달하지 못합니다.

## 1단계: 현재 상태 확인

### IP 주소 확인

```bash
ip addr
```

- `eth0`의 IP를 메모하세요 (예: `192.168.0.189`)
- `wlan0`의 IP를 메모하세요 (예: `192.168.0.81`)

### WiFi 연결 이름 확인

```bash
nmcli device status
```

출력 예시:
```
DEVICE   TYPE      STATE      CONNECTION
eth0     ethernet  connected  Wired connection 1
wlan0    wifi      connected  5층
```

`wlan0`의 CONNECTION 이름을 메모하세요 (예: `5층`).

### 현재 라우팅 metric 확인

```bash
ip route show
```

출력 예시:
```
default via 192.168.0.1 dev eth0 proto dhcp src 192.168.0.189 metric 100
default via 192.168.0.1 dev wlan0 proto dhcp src 192.168.0.81 metric 600
```

**metric 값이 낮을수록 우선순위가 높습니다.** 위 예시에서는 eth0(100)이 wlan0(600)보다 우선입니다.

> **주의:** metric 값은 환경마다 다를 수 있습니다. 예를 들어 eth0이 `100`이 아니라 `101`, `200` 등으로 나올 수 있고, wlan0이 `600`이 아니라 `20600` 등으로 나올 수도 있습니다. 중요한 것은 **숫자의 크고 작음**입니다.
>
> - eth0의 metric이 wlan0보다 **낮으면** → 유선 우선 (일반적인 상태)
> - eth0의 metric이 wlan0보다 **높으면** → 무선 우선 (SSH가 끊길 수 있음)
>
> 우리는 다음 단계에서 **eth0은 낮은 값(100), wlan0은 높은 값(600)**으로 명시적으로 설정하여 유선 우선을 확실히 합니다. 현재 값이 무엇이든 아래 명령으로 덮어쓰면 됩니다.

## 2단계: metric 설정

유선(eth0)의 우선순위를 확실히 높게 유지합니다.

```bash
sudo nmcli connection modify "Wired connection 1" ipv4.route-metric 100
sudo nmcli connection modify "5층" ipv4.route-metric 600
```

> `"5층"` 부분은 1단계에서 확인한 본인의 WiFi SSID로 변경하세요.

## 3단계: source-based routing 설정

이 설정이 핵심입니다. **유선으로 들어온 요청은 유선으로, 무선으로 들어온 요청은 무선으로 응답**하도록 합니다. 이 설정이 없으면 유선 SSH 접속 시 응답이 무선으로 나가버려 SSH가 끊깁니다.

```bash
sudo nmcli connection modify "Wired connection 1" ipv4.routing-rules "priority 100 from <eth0 IP> table 100"
sudo nmcli connection modify "5층" ipv4.routing-rules "priority 100 from <wlan0 IP> table 200"
```

예시:
```bash
sudo nmcli connection modify "Wired connection 1" ipv4.routing-rules "priority 100 from 192.168.0.189 table 100"
sudo nmcli connection modify "5층" ipv4.routing-rules "priority 100 from 192.168.0.81 table 200"
```

> IP는 반드시 1단계에서 확인한 본인의 IP로 변경하세요.

## 4단계: dispatcher 스크립트 생성

재부팅 후에도 라우팅이 자동 적용되도록 스크립트를 생성합니다.

```bash
sudo nano /etc/NetworkManager/dispatcher.d/10-policy-routing
```

nano 편집기에 아래 내용을 작성하세요:

```bash
#!/bin/bash
if [ "$1" = "eth0" ] && [ "$2" = "up" ]; then
    ip route replace default via 192.168.0.1 dev eth0 table 100
fi
if [ "$1" = "wlan0" ] && [ "$2" = "up" ]; then
    ip route replace default via 192.168.0.1 dev wlan0 table 200
    ip route replace <ESP IP>/32 dev wlan0 src <wlan0 IP>
fi
```

> `<ESP IP>`와 `<wlan0 IP>`를 본인 환경에 맞게 변경하세요.
> 예: `ip route replace 192.168.0.28/32 dev wlan0 src 192.168.0.81`

`Ctrl + O` → `Enter`로 저장, `Ctrl + X`로 나갑니다.

실행 권한을 부여합니다:

```bash
sudo chmod +x /etc/NetworkManager/dispatcher.d/10-policy-routing
```

## 5단계: ESP 장치에 대한 호스트 라우트 추가

ESP 장치가 wlan0을 통해 통신하도록 명시적으로 지정합니다.

```bash
sudo ip route add <ESP IP>/32 dev wlan0 src <wlan0 IP>
```

예시:
```bash
sudo ip route add 192.168.0.28/32 dev wlan0 src 192.168.0.81
```

> 이 명령은 지금 바로 적용됩니다. 재부팅 후에는 4단계의 dispatcher 스크립트가 자동으로 적용해줍니다.

## 6단계: 재부팅 후 확인

```bash
sudo reboot
```

재부팅 후 테스트:

```bash
# 1. SSH 접속 확인 (훈련생 데스크탑에서)
ssh pi@<eth0 IP>

# 2. ESP 통신 확인 (라즈베리파이에서)
ping -c 3 <ESP IP>
```

## 확인 결과

| 접속 방식 | 경로 | 결과 |
|---|---|---|
| 훈련생 데스크탑 → SSH | eth0 → eth0 | 정상 |
| ESP → HTTP 통신 | wlan0 → wlan0 | 정상 |

## 문제가 계속되면

```bash
# 라우팅 테이블 확인
ip route show
ip rule show
ip route show table 100
ip route show table 200

# wlan0으로 직접 ping 테스트
ping -c 3 -I wlan0 <ESP IP>
```

`-I wlan0`(대문자 I)으로는 되고 그냥 ping은 안 되면, 5단계의 호스트 라우트가 적용되지 않은 것입니다. 5단계를 다시 실행하세요.
