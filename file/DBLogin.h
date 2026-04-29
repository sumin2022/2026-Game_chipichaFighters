#pragma once
#include <string>
#include <mysql.h>

class DBLogin {
public:
	DBLogin();
	~DBLogin();
	bool ConnectDB();
	bool RegisterUser(std::string id, std::string pw);
	bool LoginUser(std::string id, std::string pw);
private:
	MYSQL* conn;
};