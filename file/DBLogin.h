#pragma once
#include <string>
// #include <mysql.h>

class DBLogin {
 public:
  DBLogin() {}
  ~DBLogin() {}
  bool ConnectDB() { return true; }
  bool RegisterUser(std::string id, std::string pw) { return true; }
  bool LoginUser(std::string id, std::string pw) { return true; }

 private:
  // MYSQL* conn;
};