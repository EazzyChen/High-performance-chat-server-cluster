#pragma once
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/base/Logging.h>
using namespace muduo;
using namespace muduo::net;

class ChatServer
{
public:
    ChatServer(EventLoop *loop,
               const InetAddress &listenAddr,
               const string &nameArg);
    ~ChatServer();
    void start();
private:
    void onConnection(const TcpConnectionPtr&); //would be call when connect
    void onMessage(const TcpConnectionPtr&, Buffer*, Timestamp); //would be call when receive message
    TcpServer _server;  //muduo server object
    EventLoop *_loop;   //muduo event loop pointer, like epoll
};