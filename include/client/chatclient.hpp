#pragma once
#include <sstream>
#include <muduo/net/TcpClient.h>
#include <muduo/net/EventLoop.h>
#include "json.hpp"
#include <string>
#include <vector>
#include <mutex>
#include <thread>
#include <iostream>
#include <map>
using namespace muduo;
using namespace muduo::net;
using json = nlohmann::json;

class ChatClient
{
public:
    // initial client
    ChatClient(EventLoop *loop, const InetAddress &serverAddr);

    // connect server
    void connect();

    void disconnect();

    void login(int id, const std::string &password);

    void registerUser(const std::string &name, const std::string &password);

    void oneChat(int fromid, int toid, const std::string &msg);

    void addFriend(int userid, int friendid);

    void createGroup(int userid, const std::string &groupname, const std::string &groupdesc);

    void joinGroup(int userid, int groupid);

    void getGroupList(int userid);

    void getGroupUserInfo(int groupid);

    void groupChat(int fromid, int groupid, const std::string &msg);

    void getfriends(int id);

    // get set id
    int getMyid();
    void setMyid(int id);

    // get set name
    std::string getMyname();
    void setMyname(std::string name);

    std::unordered_map<int, std::string> getGroupMap();
    void setGroupMap(std::unordered_map<int, std::string> groupmap);

private:
    // when conncect
    void onConnection(const TcpConnectionPtr &conn);

    // when receive message from server
    void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time);

    // response handler
    void handleLoginResponse(const json &js);

    void handleRegisterResponse(const json &js);

    void handleOneChatMessage(const json &js);

    void handleGroupChatMessage(const json &js);

    void handleCommonAck(const json &js);

    // send message to server
    void sendMessage(const std::string &msg);

private:
    int myid;
    std::string myname;
    // 群号-群名表
    std::unordered_map<int, std::string> groupnamemap;

    TcpClient _client;
    EventLoop *_loop;
    TcpConnectionPtr _conn; // 保存当前连接
};

