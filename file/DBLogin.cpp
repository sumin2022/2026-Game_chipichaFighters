#include "DBLogin.h"
#include <iostream>

DBLogin::DBLogin() {
        conn = nullptr;
}

DBLogin::~DBLogin() {
    if (conn) {
        mysql_close(conn);
    }
}

bool DBLogin::ConnectDB(
	const std::string& host,
	const std::string& user,
	const std::string& password,
	const std::string& database)
{
    conn = mysql_init(NULL);

	if (conn == nullptr)
	{
		std::cout << "MySQL initialization failed\n";
		return false;
	}

    if (!mysql_real_connect(
        conn,
		host.c_str(),   // localhost
		user.c_str(),       // user
		password.c_str(),    // password
		database.c_str(),        // database name
        3306,
        NULL,
        0))
    {
        std::cout << "DB connection failed\n";
		mysql_close(conn);
		conn = nullptr;
		return false;
    }

    std::cout << "DB connected\n";
    return true;
}

bool DBLogin::RegisterUser(std::string id, std::string pw)
{
    std::string query =
        "INSERT INTO users (id, password_hash) VALUES ('"
        + id + "','" + pw + "')";

    if (mysql_query(conn, query.c_str()))
    {
        std::cout << "Register failed: " << mysql_error(conn) << std::endl;
        return false;
    }

    std::cout << "Register success\n";
    return true;
}

bool DBLogin::LoginUser(std::string id, std::string pw)
{
    std::string query =
        "SELECT password_hash FROM users WHERE id='" + id + "'";

    if (mysql_query(conn, query.c_str()))
    {
        std::cout << "Query failed\n";
        return false;
    }

    MYSQL_RES* res = mysql_store_result(conn);
    MYSQL_ROW row = mysql_fetch_row(res);

    if (!row)
    {
        std::cout << "ID not found\n";
        mysql_free_result(res);
        return false;
    }

    std::string db_pw = row[0];

    mysql_free_result(res);

    if (db_pw == pw)
    {
        std::cout << "Login success\n";
        return true;
    }

    std::cout << "Password incorrect\n";
    return false;
}

bool DBLogin::UserExists(const std::string& id)
{
	std::string query =
		"SELECT id FROM users WHERE id='" + id + "'";

	if (mysql_query(conn, query.c_str()))
	{
		std::cout << "UserExists query failed: "
			<< mysql_error(conn) << '\n';
		return false;
	}

	MYSQL_RES* res = mysql_store_result(conn);

	if (res == nullptr) {
		return false;
	}

	MYSQL_ROW row = mysql_fetch_row(res);

	bool exists = (row != nullptr);

	mysql_free_result(res);

	return exists;
}
