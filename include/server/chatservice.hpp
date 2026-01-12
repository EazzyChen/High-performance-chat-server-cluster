#pragma once
#include <unordered_map>
#include <functional>
#include <muduo/net/TcpClient.h>
#include "json.hpp"
#include "public.hpp"
#include "usermodel.hpp"
#include "offmsgmodel.hpp"
#include "friendmodel.hpp"
#include "groupmodel.hpp"
#include "redis.hpp"
#include <mutex>
using namespace muduo;
using namespace muduo::net;
using json = nlohmann::json;

using MsgHandler = std::function<void(const TcpConnectionPtr &conn, json &js, Timestamp time)>; // alias of the function type
using std::unordered_map;
// singleton pattern, business logic class
// ChatService is a shared object between working threads
class ChatService
{
public:
    static ChatService* instance();
    // login logic
    void login(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // register logic
    void regis(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // chat one to one
    void oneChat(const TcpConnectionPtr& conn, json& js, Timestamp time);
    // add friend
    void addFriend(const TcpConnectionPtr& conn, json& js, Timestamp time);
    void getFriends(const TcpConnectionPtr& conn, json& js, Timestamp time);
    // create group
    void createGroup(const TcpConnectionPtr& conn, json& js, Timestamp time);
    // join group
    void joinGroup(const TcpConnectionPtr& conn, json& js, Timestamp time);
    // get group list
    void getGroupList(const TcpConnectionPtr& conn, json& js, Timestamp time);
    // get group user infomation
    void getGroupUserInfo(const TcpConnectionPtr& conn, json& js, Timestamp time);
    // chat by groupjs.dump()
    void groupChat(const TcpConnectionPtr& conn, json& js, Timestamp time);

    void closeAllUser();
    // get message handler
    MsgHandler getHandler(EnMsgType msgType);
    // address abnormal disconnect of client
    void clientCloseException(const TcpConnectionPtr& conn);

    void handleRedisSubscribeMessage(int channel, string msg);
private:
    ChatService();
    // different message correspond to different MsgHandler function
    unordered_map<EnMsgType, MsgHandler> _msgHandlerMap;

    // store user connection, add mutex to modify it
    unordered_map<int, TcpConnectionPtr> _userConnMap; // int is id
    std::mutex _connmutex;

    // data operation object
    UserModel _userModel;
    OffMsgModel _offMsgModel;
    FriendModel _friendModel;
    GroupModel _groupModel;
    Redis _redis;

};