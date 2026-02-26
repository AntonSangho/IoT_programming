# VS Code Remote SSH로 라즈베리파이 접속 가이드

## 사전 준비

- VS Code가 설치된 훈련생 PC (Windows)
- 라즈베리파이가 유선(eth0)으로 네트워크에 연결된 상태
- 라즈베리파이의 유선 IP 확인 (예: `192.168.0.189`)

## 1단계: VS Code 확장 프로그램 설치

1. VS Code 실행
2. 왼쪽 사이드바에서 확장 프로그램 아이콘 클릭 (또는 `Ctrl+Shift+X`)
3. 검색창에 `Remote - SSH` 입력
4. **Remote - SSH** (Microsoft) 설치

## 2단계: PowerShell에서 SSH 접속 확인

VS Code 설정 전에 먼저 **PowerShell**에서 접속이 되는지 확인합니다.

`Win + X` → **Windows PowerShell** 또는 **터미널** 실행

```powershell
ssh pi@<라즈베리파이 IP>
```

예시:
```powershell
ssh pi@192.168.0.189
```

비밀번호를 입력하여 접속이 되면 `exit`로 나옵니다.

> `ssh`가 인식되지 않으면 Windows 설정 → 앱 → 선택적 기능 → **OpenSSH 클라이언트**를 설치하세요.

## 3단계: SSH 키 생성 및 등록

VS Code Remote SSH는 백그라운드에서 여러 번 자동 재접속을 하기 때문에, 비밀번호 인증만으로는 불안정합니다. **SSH 키 인증을 설정**해야 합니다.

### SSH 키 생성 (PowerShell에서)

이미 SSH 키가 있는지 확인:

```powershell
dir ~\.ssh\id_ed25519.pub
```

파일이 없으면 새로 생성합니다:

```powershell
ssh-keygen -t ed25519
```

> 엔터 3번(기본 경로, 비밀번호 없음)으로 생성하면 됩니다.

### SSH 키를 라즈베리파이에 등록

Windows에는 `ssh-copy-id`가 없으므로 아래 명령으로 등록합니다:

```powershell
type $env:USERPROFILE\.ssh\id_ed25519.pub | ssh pi@192.168.0.189 "mkdir -p ~/.ssh && cat >> ~/.ssh/authorized_keys"
```

라즈베리파이 비밀번호를 입력하면 키가 등록됩니다.

### 비밀번호 없이 접속 확인

```powershell
ssh pi@192.168.0.189
```

비밀번호 입력 없이 바로 접속되면 성공입니다.

## 4단계: SSH config 설정

매번 IP를 입력하지 않도록 SSH config에 라즈베리파이를 등록합니다.

PowerShell에서 메모장으로 config 파일을 엽니다:

```powershell
notepad $env:USERPROFILE\.ssh\config
```

> 파일이 없다는 메시지가 나오면 **새로 만들기**를 선택하세요.

아래 내용을 추가합니다:

```
Host raspi
    HostName <라즈베리파이 IP>
    User pi
    IdentityFile ~/.ssh/id_ed25519
```

예시:
```
Host raspi
    HostName 192.168.0.189
    User pi
    IdentityFile ~/.ssh/id_ed25519
```

저장 후 메모장을 닫습니다.

등록 확인:

```powershell
ssh raspi
```

`raspi`만으로 접속되면 성공입니다.

## 5단계: VS Code에서 접속

1. `Ctrl+Shift+P` → **Remote-SSH: Connect to Host** 선택
2. 목록에서 `raspi` 선택
3. 플랫폼 선택 → **Linux**
4. 새 VS Code 창이 열리며 라즈베리파이에 연결됨

좌측 하단에 `SSH: raspi`로 표시되면 접속 성공입니다.

## 문제가 발생하면

### `Permission denied (publickey,password)` 에러

SSH 키가 제대로 등록되지 않은 경우입니다. 3단계를 다시 진행하세요.

### VS Code가 연결 중 멈춤

PowerShell에서 라즈베리파이의 VS Code 서버 프로세스를 정리합니다:

```powershell
ssh pi@<라즈베리파이 IP> "rm -rf ~/.vscode-server"
```

이후 VS Code에서 다시 접속하세요.
