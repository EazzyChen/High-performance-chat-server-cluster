#include "chatservice.hpp"
#include "muduo/base/Logging.h"
#include <string>
using std::string;
ChatService *ChatService::instance()
{
    /*
    唯一实例：static ChatService service是局部静态变量，只会初始化一次，无论调用多少次instance()，返回的都是同一个对象的地址；
    线程安全：C++11 及以后，局部static的初始化是线程安全的（编译器自动加锁），避免多线程调用instance()时创建多个实例；
    懒加载：只有第一次调用instance()时，才会创建ChatService对象（而非程序启动时就创建），节省内存和启动时间；
    自动析构：程序退出时，局部static对象会自动调用析构函数，避免内存泄漏。
    */
    static ChatService service;
    return &service;
}

void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    // login business eg:{"msgid":1,"id":100000,"password":"111111"}
    LOG_INFO << "login~~";
    int id = -1;
    string pwd;
    try
    {
        id = js.at("id").get<int>();
        pwd = js.at("password");
    }
    catch (...)
    {
        LOG_ERROR << "json error:";
    }

    User user;
    bool ret = _userModel.query(id, user);
    json jsResponse;
    if (ret)
    {
        jsResponse["msgid"] = static_cast<int>(EnMsgType::LOGIN_MSG_ACK);
        if (user.getPwd() == "")
        {
            // business when password is null
            LOG_INFO << "password is null";
            jsResponse["errno"] = 1;
            jsResponse["message"] = "password is null";
        }
        else if (user.getPwd() != pwd)
        {
            // business when password is wrong
            LOG_INFO << "login failed: password error";
            jsResponse["errno"] = 1;
            jsResponse["message"] = "login failed: password error";
        }
        else
        {
            // business when password is right

            // store user connection info
            {
                std::lock_guard<std::mutex> lg(_connmutex); // RAII style lock_guard<>
                _userConnMap.insert({id, conn});
            }
            user.setState("online"); // change suer online state
            _userModel.update(user);
            // subscribe redis
            _redis.subscribe(id);
            LOG_INFO << "login successfully";
            jsResponse["name"] = user.getName();
            jsResponse["errno"] = 0;
            jsResponse["message"] = "login successfully";

            // inquire offline message
            OffMsg offMsg;
            if (_offMsgModel.query(id, offMsg))
            {
                jsResponse["offlinemsg"] = offMsg.getMessage();
                //_offMsgModel.remove(id);
            }

            // inquire friend list
            vector<User> friendVec = {};
            if (_friendModel.query(id, friendVec))
            {
                json friendJsonArr = json::array();
                for (auto us : friendVec)
                {
                    json usjs;
                    usjs["friendid"] = us.getId();
                    usjs["name"] = us.getName();
                    usjs["state"] = us.getState();
                    friendJsonArr.push_back(usjs);
                }
                jsResponse["friends"] = friendJsonArr;
            }
            // inquire groupid-name map
            vector<Group> groupVec = {};
            if (_groupModel.queryGroups(id, groupVec))
            {
                unordered_map<int, string> groupnameMap;
                for (auto gr : groupVec)
                {
                    groupnameMap.insert({gr.getGroupId(), gr.getGroupName()});
                }
                jsResponse["groupnamemap"] = groupnameMap;
            }
        }
    }
    else
    {
        jsResponse["msgid"] = static_cast<int>(EnMsgType::LOGIN_MSG_ACK);
        jsResponse["errno"] = 1;
        jsResponse["message"] = "can not find this user";
    }
    conn->send(jsResponse.dump());
}

void ChatService::regis(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    // register business eg:{"msgid":3, "name":"aa", "password":"111111"}
    LOG_INFO << "register~~";
    string name, pwd;
    try
    {
        name = js["name"];
        pwd = js["password"];
    }
    catch (...)
    {
        LOG_ERROR << "json error:";
    }

    User user;
    user.setName(name);
    user.setPwd(pwd);
    bool ret = _userModel.insert(user);
    json jsResponse;
    if (ret)
    {
        // register successfully
        jsResponse["msgid"] = static_cast<int>(EnMsgType::REG_MSG_ACK);
        jsResponse["errno"] = 0;
        jsResponse["id"] = user.getId();
    }
    else
    {
        // register failed
        jsResponse["msgid"] = static_cast<int>(EnMsgType::REG_MSG_ACK);
        jsResponse["errno"] = 1;
    }
    conn->send(jsResponse.dump()); // send json message
}

void ChatService::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    // eg:{"msgid":5,"fromid":100000, "name": "aa","toid":100001,"msg":"hello bb 2"}
    int toid = -1;
    string name;
    try
    {
        toid = js.at("toid").get<int>();
    }
    catch (...)
    {
        LOG_ERROR << "json error:";
        return;
    }

    {
        std::lock_guard<std::mutex> lg(_connmutex);
        auto it = _userConnMap.find(toid);
        if (it != _userConnMap.end())
        {
            // toid online, send message to toid clinet
            it->second->send(js.dump());
            return;
        }
    }

    User user;
    _userModel.query(toid, user);
    if (user.getState() == "online")
    {
        // this user login other server, publish to redis
        _redis.publish(toid, js.dump());
        LOG_INFO << "publish redis";
        return;
    }
    else
    {
        // toid offline, store message
        _offMsgModel.insert(toid, js.dump());
        return;
    }
}

void ChatService::getFriends(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    // eg: {"msgid":12, "userid": 100001}
    int id = -1;
    try
    {
        id = js.at("userid").get<int>();
    }
    catch (...)
    {
        LOG_ERROR << "json error:";
        return;
    }
    vector<User> friendVec = {};
    json jsResponse;
    jsResponse["msgid"] = static_cast<int>(EnMsgType::ACK);
    if (_friendModel.query(id, friendVec))
    {
        json friendJsonArr = json::array();
        for (auto us : friendVec)
        {
            json usjs;
            usjs["friendid"] = us.getId();
            usjs["name"] = us.getName();
            usjs["state"] = us.getState();
            friendJsonArr.push_back(usjs);
        }
        jsResponse["errno"] = 0;
        jsResponse["friends"] = friendJsonArr;
    }

    else
    {
        jsResponse["errno"] = 1;
    }
    conn->send(jsResponse.dump());
}

void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    // eg: {"msgid":6,"userid":100000,"friendid":100003}
    int userid = -1;
    int friendid = -1;
    try
    {
        userid = js.at("userid").get<int>();
        friendid = js.at("friendid").get<int>();
    }
    catch (...)
    {
        LOG_ERROR << "json error:";
        return;
    }
    json jsResponse;
    if (_friendModel.insert(userid, friendid))
    {
        jsResponse["msgid"] = static_cast<int>(EnMsgType::ACK);
        jsResponse["errno"] = 0;
    }
    else
    {
        jsResponse["msgid"] = static_cast<int>(EnMsgType::ACK);
        jsResponse["errno"] = 1;
    }
    conn->send(jsResponse.dump());
}

void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    // eg: {"msgid":7, "userid": 100000, "groupname": "fuckclub", "groupdesc": "join to know how to fuck happily"}
    int userid = -1;
    string groupname;
    string groupdesc;
    try
    {
        userid = js.at("userid").get<int>();
        groupname = js.at("groupname");
        groupdesc = js.at("groupdesc");
    }
    catch (...)
    {
        LOG_ERROR << "json error:";
        return;
    }
    Group group;
    json jsResponse;
    group.setGroupName(groupname);
    group.setGroupDesc(groupdesc);
    if (
        _groupModel.insert(group) &&
        _groupModel.joinGroup(userid, group.getGroupId(), "creator"))
    {
        jsResponse["msgid"] = static_cast<int>(EnMsgType::ACK);
        jsResponse["errno"] = 0;
    }
    else
    {
        jsResponse["msgid"] = static_cast<int>(EnMsgType::ACK);
        jsResponse["errno"] = 1;
    }
    conn->send(jsResponse.dump());
}

void ChatService::joinGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    // eg: {"msgid":8, "userid": 100001, "groupid:1001"}
    int userid = -1;
    int groupid = -1;
    try
    {
        userid = js.at("userid").get<int>();
        groupid = js.at("groupid").get<int>();
    }
    catch (...)
    {
        LOG_ERROR << "json error:";
        return;
    }
    _groupModel.joinGroup(userid, groupid, "normal");
}

void ChatService::getGroupList(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    // eg: {"msgid":9, "userid": 100001}
    int userid = -1;
    try
    {
        userid = js.at("userid").get<int>();
    }
    catch (...)
    {
        LOG_ERROR << "json error:";
        return;
    }

    vector<Group> groupVec;
    json jsResponse;
    if (_groupModel.queryGroups(userid, groupVec))
    {

        json groupJsonArr = json::array();
        for (auto gr : groupVec)
        {
            json grjs;
            grjs["groupid"] = gr.getGroupId();
            grjs["groupname"] = gr.getGroupName();
            grjs["groupdesc"] = gr.getGroupDesc();
            groupJsonArr.push_back(grjs);
        }
        jsResponse["msgid"] = static_cast<int>(EnMsgType::ACK);
        jsResponse["errno"] = 0;
        jsResponse["grouplist"] = groupJsonArr;
    }
    else
    {
        jsResponse["msgid"] = static_cast<int>(EnMsgType::ACK);
        jsResponse["errno"] = 1;
    }
    conn->send(jsResponse.dump());
}

void ChatService::getGroupUserInfo(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    // eg: {"msgid": 10, "groupid": 1001}
    int groupid = -1;
    try
    {
        groupid = js.at("groupid").get<int>();
    }
    catch (...)
    {
        LOG_ERROR << "json parse error : get groupid invalid";
        return;
    }

    vector<GroupUser> groupUsers;
    json responseJson;
    if (_groupModel.queryGroupUsersInfo(groupid, groupUsers))
    {
        json userArray = json::array();
        for (auto &user : groupUsers)
        {
            json singleUser;
            singleUser["id"] = user.getId();
            singleUser["name"] = user.getName();
            singleUser["state"] = user.getState();
            singleUser["role"] = user.getRole();
            userArray.push_back(singleUser);
        }

        responseJson["msgid"] = static_cast<int>(EnMsgType::ACK);
        responseJson["errno"] = 0;
        responseJson["groupusers"] = userArray;
        LOG_INFO << "query group " << groupid << " user info success, total users : " << groupUsers.size();
    }
    else
    {
        responseJson["msgid"] = static_cast<int>(EnMsgType::ACK);
        responseJson["errno"] = 1;
        responseJson["errmsg"] = "query group user info failed";
        LOG_ERROR << "query group " << groupid << " user info failed";
    }

    conn->send(responseJson.dump());
}

void ChatService::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    // eg: {"msgid": 11, "fromid": 100000, "groupid": 1001, "msg": "fuck group message"}
    int fromid = -1;
    int groupid = -1;
    string fromname;
    string groupname;
    string msg;
    try
    {
        fromid = js.at("fromid").get<int>();
        groupid = js.at("groupid").get<int>();
    }
    catch (...)
    {
        LOG_ERROR << "json parse error : ";
        return;
    }

    vector<int> groupUserIdVec;
    if (!_groupModel.broadcastUsersId(fromid, groupid, groupUserIdVec))
    {
        LOG_ERROR << "group " << groupid << " query member id failed";
        return;
    }

    std::lock_guard<std::mutex> lock(_connmutex);
    for (int userid : groupUserIdVec)
    {
        if (userid == fromid)
        {
            continue;
        }

        auto it = _userConnMap.find(userid);
        if (it != _userConnMap.end())
        {
            it->second->send(js.dump());
        }
        else
        {
            User user;
            _userModel.query(userid, user);
            if (user.getState() == "online")
            {
                // this user login other server, publish to redis
                _redis.publish(userid, js.dump());
                return;
            }
            else
            {
                // toid offline, store message
                _offMsgModel.insert(userid, js.dump());
                return;
            }
        }
    }
}

void ChatService::closeAllUser()
{
    _userModel.closeAll();
}

MsgHandler ChatService::getHandler(EnMsgType msgType)
{
    auto it = _msgHandlerMap.find(msgType);
    int msgId = static_cast<int>(msgType);
    if (it == _msgHandlerMap.end())
    {

        // return non-operation handler and print muduo log
        return MsgHandler([=](const TcpConnectionPtr &conn, json &js, Timestamp time)
                          {
                            // muduo log
                            LOG_ERROR << "msgId: " << msgId << " can not find handler~"; });
    }
    else
        return _msgHandlerMap[msgType];
}

void ChatService::clientCloseException(const TcpConnectionPtr &conn)
{
    User user;
    {
        std::lock_guard<std::mutex> lg(_connmutex);
        for (auto it = _userConnMap.begin(); it != _userConnMap.end(); it++)
        {
            if (it->second == conn)
            {
                // delete conn
                user.setId(it->first);
                _userConnMap.erase(it);

                break;
            }
        }
    }

    // unsubscribe redis
    _redis.unsubscribe(user.getId());
    if (user.getId() != -1) // make sure be finded
    {
        _userModel.query(user.getId(), user);
        user.setState("offline");
        _userModel.update(user);
    }
}

void ChatService::handleRedisSubscribeMessage(int channel, string msg)
{
    std::lock_guard<std::mutex> lg(_connmutex);
    auto it = _userConnMap.find(channel);
    if (it != _userConnMap.end())
    {
        it->second->send(msg);
        return;
    }
    else
    {
        _offMsgModel.insert(channel, msg);
    }
}

ChatService::ChatService()
{
    // bind login function
    _msgHandlerMap[EnMsgType::LOGIN_MSG] = MsgHandler(bind(&ChatService::login, this,
                                                           std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    // bind regis function
    _msgHandlerMap[EnMsgType::REGIS_MSG] = MsgHandler(bind(&ChatService::regis, this,
                                                           std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    _msgHandlerMap[EnMsgType::ONE_CHAT_MSG] = MsgHandler(bind(&ChatService::oneChat, this,
                                                              std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    _msgHandlerMap[EnMsgType::ADD_FRIEND_MSG] = MsgHandler(bind(&ChatService::addFriend, this,
                                                                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    _msgHandlerMap[EnMsgType::CREATE_GROUP_MSG] = MsgHandler(bind(&ChatService::createGroup, this,
                                                                  std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    _msgHandlerMap[EnMsgType::JOIN_GROUP_MSG] = MsgHandler(bind(&ChatService::joinGroup, this,
                                                                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    _msgHandlerMap[EnMsgType::GET_GROUPLIST] = MsgHandler(bind(&ChatService::getGroupList, this,
                                                               std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    _msgHandlerMap[EnMsgType::GET_GROUP_USERINFO] = MsgHandler(bind(&ChatService::getGroupUserInfo, this,
                                                                    std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    _msgHandlerMap[EnMsgType::GROUP_CHAT_MSG] = MsgHandler(bind(&ChatService::groupChat, this,
                                                                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    _msgHandlerMap[EnMsgType::GET_FRIENDS] = MsgHandler(bind(&ChatService::getFriends, this,
                                                             std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    if (_redis.connect())
    {
        _redis.initNotifyHandler(std::bind(&ChatService::handleRedisSubscribeMessage, this,
                                           std::placeholders::_1, std::placeholders::_2));
    }
}
