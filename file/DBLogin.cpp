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

bool DBLogin::ConnectDB()
{
    conn = mysql_init(NULL);

    if (!mysql_real_connect(
        conn,
        "127.0.0.1",   // localhost
        "root",        // user
		"pungbear2018",    // password
        "game",        // database name
        3306,
        NULL,
        0))
    {
        std::cout << "DB connection failed\n";
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