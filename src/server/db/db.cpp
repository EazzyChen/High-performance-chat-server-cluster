#include "db.hpp"
#include <muduo/base/Logging.h>
#include <mysql/mysql.h>
#include <string>
// mysql封装，可复用
// 匿名命名空间：配置仅当前文件可见；双层const：指针+内容都只读
namespace {
const char* const DB_IP = "127.0.0.1";
const int DB_PORT = 3306;
const char* const DB_USER = "root";
const char* const DB_PSW = "123456";
const char* const DB_NAME = "chat";
}

Db::Db() : _conn(nullptr)
{
}

Db::~Db()
{
    if (_conn != nullptr)
    {
        mysql_close(_conn);
        _conn = nullptr; // 避免野指针
    }
}

bool Db::connect()
{
    // 初始化MySQL连接对象
    _conn = mysql_init(nullptr);
    if (_conn == nullptr) {
        LOG_ERROR << "mysql_init failed! (初始化连接对象失败)";
        return false;
    }

    // 建立数据库连接
    MYSQL *p = mysql_real_connect(
        _conn, DB_IP, DB_USER, DB_PSW, DB_NAME,
        DB_PORT, nullptr, 0
    );

    if (p != nullptr)
    {
        // 推荐用utf8mb4，兼容所有中文/特殊字符（需数据库表编码匹配）
        if (mysql_query(_conn, "set names utf8mb4")) {
            LOG_WARN << "set charset utf8mb4 failed: " << mysql_error(_conn);
        }
        LOG_INFO << "connect Database(mysql) successfully";
        return true;
    }
    else
    {
        // 错误日志用LOG_ERROR，添加分隔符提升可读性
        LOG_ERROR << "connect Database(mysql) failed: " << mysql_error(_conn);
        mysql_close(_conn);
        _conn = nullptr;
        return false;
    }
}

bool Db::update(string sql, int &insert_id)
{
    // 先检查连接有效性
    if (_conn == nullptr)
    {
        LOG_ERROR << "update failed: mysql connection is null! sql: " << sql;
        return false;
    }

    // 初始化自增ID
    insert_id = -1;

    // 执行SQL语句
    if (mysql_query(_conn, sql.c_str()))
    {
        // 错误日志级别改为ERROR，添加分隔符
        LOG_ERROR << __FILE__ << ":" << __LINE__ << ": " 
                  << sql << " update failed, err: " << mysql_error(_conn);
        return false;
    }

    // 获取自增ID
    insert_id = mysql_insert_id(_conn);
    LOG_INFO << "update success";
    return true;
}

MYSQL_RES *Db::query(string sql)
{   
    // 先检查连接有效性
    if (_conn == nullptr)
    {
        LOG_ERROR << "query failed: mysql connection is null! sql: " << sql;
        return nullptr;
    }

    // 先执行SQL语句，再获取结果集
    if (mysql_query(_conn, sql.c_str())) {
        LOG_ERROR << __FILE__ << ":" << __LINE__ << ": " 
                  << sql << " query failed, err: " << mysql_error(_conn);
        return nullptr;
    }

    // 获取结果集（一次性读取，易管理）
    MYSQL_RES* res = mysql_store_result(_conn);
    if (res == nullptr) {
        LOG_WARN << "query success but no result: " << sql;
    } else {
        LOG_INFO << "query success, rows: " << mysql_num_rows(res);
    }
    return res;
}
