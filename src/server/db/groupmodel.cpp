#include "groupmodel.hpp"
#include "db.hpp"
bool GroupModel::insert(Group &group)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into AllGroup (groupname, groupdesc) values('%s', '%s')",
            group.getGroupName().c_str(), group.getGroupDesc().c_str());
    Db database;
    if (database.connect())
    {
        int insert_id = -1;
        if (database.update(sql, insert_id))
        {
            group.setGroupId(insert_id);
            return true;
        }
    }

    LOG_INFO << "err: insert group";
    return false;
}

bool GroupModel::joinGroup(const int &userid, const int &groupid, const string &role)
{
    char sql[1024];
    sprintf(sql, "insert into GroupUser (groupid, userid, grouprole) values(%d, %d, '%s')",
            groupid, userid, role.c_str());
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

bool GroupModel::queryGroups(const int &userid, vector<Group> &groupVec)
{
    groupVec.clear();
    char sql[1024] = {0};
    sprintf(sql, "select a.id,a.groupname,a.groupdesc from AllGroup a inner join GroupUser b on a.id = b.groupid where b.userid = %d", userid);
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
                Group group;
                group.setGroupId(atoi(row[0]));
                group.setGroupName(row[1]);
                group.setGroupDesc(row[2]);
                groupVec.push_back(group);
            }
            // must release res
            mysql_free_result(res);
            return true;
        }
    }
    return false;
}

bool GroupModel::queryGroupUsersInfo(const int &groupid, vector<GroupUser> &groupUserVec)
{
    char sql[1024] = {0};
    sprintf(sql, "select a.id, a.name, a.state, b.grouprole from User a inner join GroupUser b on a.id=b.userid where b.groupid = %d", groupid);
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
                GroupUser gu;
                gu.setId(atoi(row[0]));
                gu.setName(row[1]);
                gu.setState(row[2]);
                gu.setRole(row[3]);
                groupUserVec.push_back(gu);
            }
            // must release res
            mysql_free_result(res);
            return true;
        }
    }
    return true;
}

bool GroupModel::broadcastUsersId(const int &userid, const int &groupid, vector<int> &idVec)
{
    char sql[1024] = {0};
    sprintf(sql, "select userid from GroupUser where groupid = %d and userid  != %d", groupid, userid);
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
                idVec.push_back(atoi(row[0]));
            }
            // must release res
            mysql_free_result(res);
            return true;
        }
    }
    return true;
}
