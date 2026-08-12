#pragma once
#include <string>
// #include <mysql.h>

class DBLogin {
public:
	DBLogin();
	~DBLogin();
	bool ConnectDB();
	bool UserExists(const std::string& id);
	bool RegisterUser(std::string id, std::string pw);
	bool LoginUser(std::string id, std::string pw);
private:
	MYSQL* conn;
};
