## AI 서버 실행 환경

AI 서버 프로젝트에는 MySQL 연동 코드가 포함되어 있으므로,
데이터베이스 사용 여부와 관계없이 MySQL Connector 설정이 필요합니다.

- MySQL Server 또는 MySQL Connector/C 설치
- Visual Studio 프로젝트 설정
  - 추가 포함 디렉터리: `MySQL 설치경로\include`
  - 추가 라이브러리 디렉터리: `MySQL 설치경로\lib`
  - 추가 종속성: `libmysql.lib`
- 실행 파일과 같은 경로에 `libmysql.dll` 배치

## 데이터베이스 사용 모드

AI 서버 실행 시 `Use Database? (y/n)`에 `y`를 입력하면
MySQL 데이터베이스를 사용합니다.

AI 서버는 게임 서버와 동일한 MySQL 서버를 사용할 수 있습니다.
다만 AI 모델 정보는 `game`이 아닌 `ai_db` 스키마에 저장됩니다.

### AI 데이터베이스 생성

1. MySQL Workbench에서 `ai_db` 스키마를 생성합니다.
2. `Server → Data Import`로 이동합니다.
3. `Import from Self-Contained File`을 선택합니다.
4. 프로젝트의 `ai_db_ai_models.sql`을 선택합니다.
5. `Default Target Schema`로 `ai_db`를 선택합니다.
6. `Start Import`를 눌러 `ai_models` 테이블을 가져옵니다.

### AI 서버 연결 정보

실행 시 다음 정보를 입력합니다.

- DB Server IP: MySQL 서버의 IP 주소
- DB User: MySQL 계정
- DB Password: MySQL 계정 비밀번호
- DB Name: `ai_db`
- MySQL Port: `3306`으로 코드 내부에 고정

AI 서버와 MySQL이 같은 컴퓨터에서 실행된다면
DB Server IP로 `127.0.0.1`을 사용합니다.

서로 다른 컴퓨터에서 실행된다면 MySQL이 실행 중인
컴퓨터의 내부 IP 주소를 입력합니다.

AI 서버를 실행하는 컴퓨터에는 MySQL Server가 반드시 필요하지는 않지만,
빌드 및 실행을 위해 MySQL Connector, `libmysql.lib`,
`libmysql.dll`이 필요합니다.
