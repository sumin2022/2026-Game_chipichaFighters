## 필수 환경

- MySQL Server 또는 MySQL Connector/C 설치
- Visual Studio 프로젝트 설정
  - 추가 포함 디렉터리: `MySQL 설치경로\include`
  - 추가 라이브러리 디렉터리: `MySQL 설치경로\lib`
  - 추가 종속성: `libmysql.lib`
- 실행 파일 옆에 `libmysql.dll` 배치

## 데이터베이스 설정

MySQL Server와 MySQL Workbench가 필요합니다.

### MySQL Workbench로 생성

1. MySQL Workbench를 실행합니다.
2. 새로운 스키마를 생성하고 이름을 `game`으로 지정합니다.
3. `Server → Data Import`로 이동합니다.
4. `Import from Self-Contained File`을 선택합니다.
5. 프로젝트에 포함된 `game_users.sql`을 선택합니다.
6. `Default Target Schema`로 `game`을 선택합니다.
7. `Start Import`를 눌러 테이블을 가져옵니다.

## 데이터베이스 연결 정보

서버 실행 시 자신의 MySQL 연결 정보를 입력합니다.

- DB Server IP: `127.0.0.1`
- DB User: 본인의 MySQL 계정
- DB Password: 본인의 MySQL 비밀번호
- DB Name: `game`
- MySQL 사용 포트: `3306`

게임 서버와 MySQL이 다른 컴퓨터에서 실행되는 경우에는
`127.0.0.1` 대신 MySQL 서버가 실행 중인 컴퓨터의 IP 주소를 입력합니다.
