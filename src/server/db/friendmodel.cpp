#include "friendmodel.hpp"
#include "db.hpp"
using std::vector;
bool FriendModel::insert(const int &userid, const int &friendid)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into Friend (userid, friendid) values(%d, %d)",
            userid, friendid);
    Db database;
    if (database.connect())
    {
        int insert_id = -1;
        if (database.update(sql, insert_id))
        {
            return true;
        }
    }

    LOG_INFO << "err: the user is not exist";
    return false;
}

bool FriendModel::query(const int &userid, std::vector<User> &friends)
{
    friends.clear();
    char sql[1024] = {0};
    sprintf(sql, "select a.id,a.name,a.state from User a inner join Friend b on b.friendid = a.id where b.userid = %d", userid);
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
            MYSQL_ROW row = nullptr;
            while ((row = mysql_fetch_row(res)) != nullptr) // read multi-rows
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                friends.push_back(user);
            }
            // must release res
            mysql_free_result(res);
            return true;
        }
    }
    return false;
}