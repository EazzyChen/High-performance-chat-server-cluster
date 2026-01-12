#include "offmsgmodel.hpp"
#include "db.hpp"

// insert one message
bool OffMsgModel::insert(const int &userid, const string &message)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into OfflineMessage (userid, message) values(%d, '%s')",
            userid, message.c_str());
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

bool OffMsgModel::remove(const int &userid)
{
    char sql[1024] = {0};
    sprintf(sql, "delete from OfflineMessage where userid=%d", userid);
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

bool OffMsgModel::query(const int &userid, OffMsg &offMsg)
{
    char sql[1024] = {0};
    sprintf(sql, "select message from OfflineMessage where userid = %d", userid);
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
            return false;
        }
        else
        {
            MYSQL_ROW row = nullptr;
            json offMsgJsonArr;
            int rowcnt=0;
            while ((row = mysql_fetch_row(res)) != nullptr && row[0]!=nullptr) // read multi-rows
            {
                json offmsgjs;
                try {
                    LOG_INFO << "debug parse";
                    offmsgjs = json::parse(row[0]);
                    offMsgJsonArr.push_back(offmsgjs);
                    rowcnt++;
                } catch (...) {
                    LOG_ERROR << "json parse error:" << __FILE__ << __LINE__;
                }
            }
            // must release res
            mysql_free_result(res);
            if(rowcnt == 0)
            {
                return false;
            }
            offMsg.setUserId(userid);
            LOG_INFO << "debug setmessage 1";
            offMsg.setMessage(offMsgJsonArr);
            LOG_INFO << "debug setmessage 2";
            
        }
    }
    return true;
}
