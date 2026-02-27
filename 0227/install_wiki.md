# MediaWiki 설치 가이드 (Raspberry Pi + Docker + SQLite)

## 1. Docker 설치

```bash
sudo apt update -y && sudo apt full-upgrade -y
```
```bash
curl -fsSL https://get.docker.com -o get-docker.sh
sudo sh get-docker.sh
```
```bash
sudo docker version
```
```bash
sudo usermod -aG docker pi
sudo reboot
```

## 2. MediaWiki 디렉토리 준비

웹서버가 바라보는 `/var/www/html/` 하위에 설치합니다.

```bash
sudo mkdir -p /var/www/html/mediawiki
sudo chown -R pi:pi /var/www/html/mediawiki
cd /var/www/html/mediawiki
```

SQLite DB 파일이 저장될 data 디렉토리를 생성하고 권한을 설정합니다.

```bash
mkdir -p data
sudo chmod 777 data
```

## 3. docker-compose.yml 작성

SQLite를 사용하므로 MariaDB 컨테이너 없이 구성합니다.

```bash
touch docker-compose.yml
```

```yaml
services:
  mediawiki:
    image: mediawiki
    restart: unless-stopped
    ports:
      - 8080:80
    volumes:
      - ./images:/var/www/html/images
      - ./data:/var/www/data
      # 초기 설치 완료 후 LocalSettings.php 다운로드 후 아래 주석 해제
      # - ./LocalSettings.php:/var/www/html/LocalSettings.php
```

```bash
sudo docker compose up -d
```

## 4. MediaWiki 초기 설정 (웹 브라우저)

브라우저에서 접속합니다.

```
http://<라즈베리파이 IP>:8080
```

설치 마법사에서 아래와 같이 설정합니다.

| 항목 | 값 |
|---|---|
| 데이터베이스 종류 | SQLite |
| DB 저장 경로 | /var/www/data |
| DB 이름 | YH_wiki (원하는 이름) |

설정 완료 후 `LocalSettings.php` 파일을 다운로드합니다.

## 5. LocalSettings.php 적용

다운로드한 `LocalSettings.php`를 `/var/www/html/mediawiki/`에 복사합니다.

PC에서 Pi로 전송하는 경우:
```bash
scp LocalSettings.php pi@<라즈베리파이 IP>:/var/www/html/mediawiki/
```

`docker-compose.yml`에서 주석을 해제합니다.

```yaml
volumes:
  - ./images:/var/www/html/images
  - ./data:/var/www/data
  - ./LocalSettings.php:/var/www/html/LocalSettings.php
```

컨테이너를 재시작합니다.

```bash
sudo docker compose restart mediawiki
```

## 6. 접속 확인

```
http://<라즈베리파이 IP>:8080
```

## 참고

- [Personal MediaWiki with Raspberry PI and Docker](https://peppe8o.com/personal-mediawiki-with-raspberry-pi-and-docker/)
- [Beginner's Guide to Install and Use Docker with Raspberry PI](https://peppe8o.com/docker-raspberry-pi-portainer/)