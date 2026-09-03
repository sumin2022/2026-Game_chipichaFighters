#pragma once
#include <string>
#include <mysql.h>

class DBLogin {
public:
	DBLogin();
	~DBLogin();

	bool ConnectDB(
		const std::string& host,
		const std::string& user,
		const std::string& password,
		const std::string& database
	);

	bool UserExists(const std::string& id);
	bool RegisterUser(std::string id, std::string pw);
	bool LoginUser(std::string id, std::string pw);
private:
	MYSQL* conn;
};
