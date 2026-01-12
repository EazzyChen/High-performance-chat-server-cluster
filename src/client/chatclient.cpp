#include "chatclient.hpp"
#include "public.hpp"

// 构造函数实现
ChatClient::ChatClient(EventLoop *loop, const InetAddress &serverAddr)
    : _client(loop, serverAddr, "ChatClient"),
      _loop(loop)
{
    myid = -1;
    // 设置连接回调
    _client.setConnectionCallback(
        std::bind(&ChatClient::onConnection, this, std::placeholders::_1));

    // 设置消息接收回调
    _client.setMessageCallback(
        std::bind(&ChatClient::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    // 启用自动重连
    _client.enableRetry();
}

// 连接服务器实现
void ChatClient::connect()
{
    _client.connect();
}

// 断开连接实现
void ChatClient::disconnect()
{
    _client.disconnect();
}

// 登录接口实现
void ChatClient::login(int id, const std::string &password)
{
    json js;
    js["msgid"] = static_cast<int>(EnMsgType::LOGIN_MSG);
    js["id"] = id;
    js["password"] = password;
    setMyid(id);
    sendMessage(js.dump());
}

// 注册接口实现
void ChatClient::registerUser(const std::string &name, const std::string &password)
{
    json js;
    js["msgid"] = static_cast<int>(EnMsgType::REGIS_MSG);
    js["name"] = name;
    js["password"] = password;
    sendMessage(js.dump());
}

// 单聊接口实现
void ChatClient::oneChat(int fromid, int toid, const std::string &msg)
{
    json js;
    js["msgid"] = static_cast<int>(EnMsgType::ONE_CHAT_MSG);
    js["fromid"] = fromid;
    js["toid"] = toid;
    js["name"] = getMyname();
    js["msg"] = msg;
    sendMessage(js.dump());
}

// 添加好友接口实现
void ChatClient::addFriend(int userid, int friendid)
{
    json js;
    js["msgid"] = static_cast<int>(EnMsgType::ADD_FRIEND_MSG);
    js["userid"] = userid;
    js["friendid"] = friendid;
    sendMessage(js.dump());
}

// 创建群组接口实现
void ChatClient::createGroup(int userid, const std::string &groupname, const std::string &groupdesc)
{
    json js;
    js["msgid"] = static_cast<int>(EnMsgType::CREATE_GROUP_MSG);
    js["userid"] = userid;
    js["groupname"] = groupname;
    js["groupdesc"] = groupdesc;
    sendMessage(js.dump());
}

// 加入群组接口实现
void ChatClient::joinGroup(int userid, int groupid)
{
    json js;
    js["msgid"] = static_cast<int>(EnMsgType::JOIN_GROUP_MSG);
    js["userid"] = userid;
    js["groupid"] = groupid;
    sendMessage(js.dump());
}

// 获取群组列表接口实现
void ChatClient::getGroupList(int userid)
{
    json js;
    js["msgid"] = static_cast<int>(EnMsgType::GET_GROUPLIST);
    js["userid"] = userid;
    sendMessage(js.dump());
}

// 获取群成员信息接口实现
void ChatClient::getGroupUserInfo(int groupid)
{
    json js;
    js["msgid"] = static_cast<int>(EnMsgType::GET_GROUP_USERINFO);
    js["groupid"] = groupid;
    sendMessage(js.dump());
}

// 群聊接口实现
void ChatClient::groupChat(int fromid, int groupid, const std::string &msg)
{
    auto it = getGroupMap().find(groupid);
    if (it == getGroupMap().end())
    {
        std::cout << "您不是群" << groupid << "成员" << std::endl;
        return;
    }
    else
    {
        json js;
        js["msgid"] = static_cast<int>(EnMsgType::GROUP_CHAT_MSG);
        js["fromid"] = fromid;
        js["fromname"] = getMyname();
        js["groupid"] = groupid;
        js["groupname"] = getGroupMap().at(groupid);
        js["msg"] = msg;
        sendMessage(js.dump());
    }
}

void ChatClient::getfriends(int id)
{
    json js;
    js["msgid"] = static_cast<int>(EnMsgType::GET_FRIENDS);
    js["userid"] = id;
    sendMessage(js.dump());
}

int ChatClient::getMyid()
{
    return this->myid;
}

void ChatClient::setMyid(int id)
{
    this->myid = id;
}

std::string ChatClient::getMyname()
{
    return this->myname;
}

void ChatClient::setMyname(std::string name)
{
    this->myname = name;
}

std::unordered_map<int, std::string> ChatClient::getGroupMap()
{
    return this->groupnamemap;
}

void ChatClient::setGroupMap(std::unordered_map<int, std::string> groupnamemap)
{
    this->groupnamemap = groupnamemap;
}

// 连接状态回调实现
void ChatClient::onConnection(const TcpConnectionPtr &conn)
{
    if (conn->connected())
    {
        std::cout << "服务器连接成功！" << std::endl;
        _conn = conn;
    }
    else
    {
        std::cout << "与服务器断开连接！" << std::endl;
        _conn.reset();
        _loop->quit();
    }
}

// 消息接收回调实现
void ChatClient::onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)
{
    std::string msg = buf->retrieveAllAsString();
    json js = json::parse(msg);

    int msgid = js["msgid"];
    switch (static_cast<EnMsgType>(msgid))
    {
    case EnMsgType::LOGIN_MSG_ACK:
        handleLoginResponse(js);
        break;
    case EnMsgType::REG_MSG_ACK:
        handleRegisterResponse(js);
        break;
    case EnMsgType::ONE_CHAT_MSG:
        handleOneChatMessage(js);
        break;
    case EnMsgType::GROUP_CHAT_MSG:
        handleGroupChatMessage(js);
        break;
    case EnMsgType::ACK:
        handleCommonAck(js);
        break;
    default:
        std::cerr << "收到未知类型的消息！msgid=" << msgid << std::endl;
        break;
    }
}

// 处理登录响应实现
void ChatClient::handleLoginResponse(const json &js)
{
    int errno1 = js["errno"];

    if (errno1 != 0)
    {
        std::cerr << "登录失败: " << js["message"] << std::endl;
        return;
    }

    std::cout << "===== 登录成功 =====" << std::endl;
    // 打印离线消息
    if (js.contains("name"))
        setMyname(js["name"]);

    if (js.contains("offlinemsg"))
    {
        std::vector<json> offlinemsgVec = js["offlinemsg"];
        std::cout << "[离线消息] :" << std::endl;
        int cnt = 1;
        for (json offmsgjs : offlinemsgVec)
        {
            std::cout << "(离线消息" << cnt << ")" << std::endl;
            std::cout << "name: " << offmsgjs["name"] << "(" << offmsgjs["fromid"].get<int>() << ")" << std::endl;
            std::cout << "message: " << offmsgjs["msg"] << std::endl;
            cnt++;
        }
    }
    // 打印好友列表
    if (js.contains("friends"))
    {
        std::vector<json> friends = js["friends"];
        std::cout << "【你的好友列表】" << std::endl;
        for (const auto &fjs : friends)
        {
            std::cout << "ID: " << fjs["friendid"] << " | 昵称: " << fjs["name"] << " | 状态: " << fjs["state"] << std::endl;
        }
    }

    // 维护群号-群名表
    if (js.contains("groupnamemap"))
    {
        std::unordered_map<int, std::string> groupnameMap;
        setGroupMap(groupnameMap);
    }
    std::cout << "====================" << std::endl;
}

// 处理注册响应实现
void ChatClient::handleRegisterResponse(const json &js)
{
    int errno1 = js["errno"];
    if (errno1 != 0)
    {
        std::cerr << "注册失败！" << std::endl;
        return;
    }
    std::cout << "注册成功！你的用户ID为: " << js["id"] << std::endl;
}

// 处理单聊消息实现
void ChatClient::handleOneChatMessage(const json &js)
{
    int fromid = js["fromid"];
    std::string msg = js["msg"];
    std::string name = js["name"];
    std::cout << "\n【私聊】用户[" << name << "_" << fromid << "]：" << msg << std::endl;
}

// 处理群聊消息实现
void ChatClient::handleGroupChatMessage(const json &js)
{
    int fromid = js["fromid"];
    int groupid = js["groupid"];
    std::string msg = js["msg"];
    std::string groupname = js["groupname"];
    std::string fromname = js["fromname"];
    std::cout << "\n【群聊】群[" << groupname << "_" << groupid << "]-用户[" << fromname << "_" << fromid << "]：" << msg << std::endl;
}

// 处理通用响应实现
void ChatClient::handleCommonAck(const json &js)
{
    int errno1 = js["errno"];
    if (errno1 != 0)
    {
        std::cerr << "操作失败！错误码: " << errno1;
        if (js.contains("errmsg"))
        {
            std::cerr << " | 错误信息: " << js["errmsg"];
        }
        std::cerr << std::endl;
        return;
    }

    // 处理获取好友列表
    if (js.contains("friends"))
    {
        std::vector<json> friends = js["friends"];
        std::cout << "\n 你的好友列表】" << std::endl;
        for (const auto &fjs : friends)
        {
            std::cout << "好友ID：" << fjs["friendid"].get<int>() << " | 名称：" << fjs["name"] << " | 状态：" << fjs["state"] << std::endl;
        }
    }

    // 处理获取群组列表响应
    if (js.contains("grouplist"))
    {
        std::vector<json> grouplist = js["grouplist"];
        std::cout << "\n【你的群组列表】" << std::endl;
        for (const auto &gjs : grouplist)
        {
            std::cout << "群ID: " << gjs["groupid"].get<int>() << " | 群名: " << gjs["groupname"] << " | 群介绍: " << gjs["groupdesc"] << std::endl;
        }
    }

    // 处理获取群成员信息响应
    if (js.contains("groupusers"))
    {
        json groupusers = js["groupusers"];
        std::cout << "\n【群成员信息】" << std::endl;
        for (const auto &u : groupusers)
        {
            std::cout << "用户ID: " << u["id"].get<int>() << " | 昵称: " << u["name"] << " | 状态: " << u["state"] << " | 身份: " << u["role"] << std::endl;
        }
    }

    std::cout << "操作执行成功！" << std::endl;
}

// 发送消息实现
void ChatClient::sendMessage(const std::string &msg)
{
    if (_conn)
    {
        _conn->send(msg);
    }
    else
    {
        std::cerr << "未连接服务器，发送消息失败！" << std::endl;
    }
}
