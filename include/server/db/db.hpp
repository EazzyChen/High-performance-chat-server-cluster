#pragma once
#include <mysql/mysql.h>
#include <string>
#include <muduo/base/Logging.h>
using std::string;


class Db
{
public:
    Db();
    ~Db();
    bool connect();
    bool update(string sql, int& insert_id); //sql:sql luanguage, insert_id: to get auto created id
    MYSQL_RES* query(string sql);
    
private:
    MYSQL* _conn;

};