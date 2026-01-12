#include "chatserver.hpp"
#include <functional>
#include <string>
#include "json.hpp"
#include "chatservice.hpp"
#include <exception>

using json = nlohmann::json;
ChatServer::ChatServer(EventLoop *loop,
                       const InetAddress &listenAddr,
                       const string &nameArg)
    : _server(loop, listenAddr, nameArg), _loop(loop)
{
    // register connection callback
    _server.setConnectionCallback(bind(&ChatServer::onConnection, this,
                                       std::placeholders::_1));
    // register message callback
    _server.setMessageCallback(bind(&ChatServer::onMessage, this,
                                    std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    // set thread number
    _server.setThreadNum(4);
}

ChatServer::~ChatServer()
{
    ChatService::instance()->closeAllUser();
    LOG_INFO << "the server is dead, all users offline";
}

void ChatServer::start()
{
    _server.start();
}

void ChatServer::onConnection(const TcpConnectionPtr &conn)
{
    // when a new client connect
    if (!conn->connected())
    { // check state
        // release resources when client offline
        ChatService::instance()->clientCloseException(conn);
        conn->shutdown();
    }
}

void ChatServer::onMessage(const TcpConnectionPtr &conn, Buffer *buffer, Timestamp time)
{
    // when a client have meassage event
    string buf = buffer->retrieveAllAsString();
    json js;
    try
    {
        js = json::parse(buf); // deserialization
        // forward content to business logic for decoupling between net and business
        int msgId = js.at("msgid").get<int>();                                                // js["msgid"] -> int
        auto msgHandler = ChatService::instance()->getHandler(static_cast<EnMsgType>(msgId)); // int -> EnMsgType
        msgHandler(conn, js, time);                                                           // callback
    }
    catch (const json::exception &err)
    {
        LOG_ERROR << "json error:" << err.what();
    }
    catch (const std::exception &err)
    {
        LOG_ERROR << "error:" << err.what();
    }
}