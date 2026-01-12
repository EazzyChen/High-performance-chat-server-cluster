#include "usermodel.hpp"
#include "db.hpp"
#include <iostream>
using std::sprintf;

bool UserModel::insert(User &user)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into User(name, password, state) values('%s', '%s', '%s')",
            user.getName().c_str(), user.getPwd().c_str(), user.getState().c_str());
    Db database;
    if (database.connect())
    {
        int insert_id = -1;
        if (database.update(sql, insert_id))
        {
            user.setId(insert_id);
            return true;
        }
    }

    return false;
}

bool UserModel::query(const int &id, User& user)
{
    char sql[1024] = {0};
    sprintf(sql, "select * from User where id = %d", id);
    Db database;
    if (!database.connect())
    {
        return false;
    }
    else
    {
        MYSQL_RES *res = database.query(sql);
        if (res == nullptr)
        {
            mysql_free_result(res);
            return false;
        }
        else
        {
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row == nullptr)
            {
                return false;
            }
            else
            {
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPwd(row[2]);
                user.setState(row[3]);
                // must release res
                mysql_free_result(res);
                return true;
            }
        }
    }
    return true;
}

bool UserModel::update(User &user)
{
    char sql[1024];
    sprintf(sql, "update User set name = '%s', password = '%s', state = '%s' where id = %d", user.getName().c_str(), user.getPwd().c_str(), user.getState().c_str(), user.getId());
    Db database;
    if (database.connect())
    {
        int insert_id = -1;
        if (database.update(sql, insert_id))
        {
            return true;
        }
    }
    return false;
}

bool UserModel::closeAll()
{
    char sql[1024];
    sprintf(sql, "update User set state = 'offline'");
    Db database;
    if (database.connect())
    {
        int insert_id = -1;
        if (database.update(sql, insert_id))
        {
            return true;
        }
    }
    return false;
}
