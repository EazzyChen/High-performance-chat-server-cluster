#include "redis.hpp"
#include <iostream>
#include <string.h>
#include <thread>
using namespace std;

Redis::Redis()
    : _publishContext(nullptr), _subscribeContext(nullptr)
{
}

Redis::~Redis()
{
    // release context resource
    if (_publishContext != nullptr)
    {
        redisFree(_publishContext);
    }
    if (_subscribeContext != nullptr)
    {
        redisFree(_subscribeContext);
    }
}

bool Redis::connect()
{
    // initial publish connect
    _publishContext = redisConnect("127.0.0.1", 6379);
    if (_publishContext == nullptr || _publishContext->err)
    {
        cerr << "connect redis failed! " << (_publishContext ? _publishContext->errstr : "create context error") << endl;
        return false;
    }

    // initial subscribe connect
    _subscribeContext = redisConnect("127.0.0.1", 6379);
    if (_subscribeContext == nullptr || _subscribeContext->err)
    {
        cerr << "connect redis failed! " << (_subscribeContext ? _subscribeContext->errstr : "create context error") << endl;
        return false;
    }

    // use thread to observe channel, because it will be blocked
    thread t(&Redis::observerChannelMessage, this);
    t.detach(); // let therad run in background

    cout << "connect redis-server success!" << endl;
    return true;
}

bool Redis::publish(int channel, string message)
{
    // write redisCommand to publish
    // format: PUBLISH channel message
    redisReply *reply = (redisReply *)redisCommand(_publishContext, "PUBLISH %d %s", channel, message.c_str());
    if (reply == nullptr)
    {
        cerr << "publish command execute failed!" << endl;
        return false;
    }
    // release resource
    freeReplyObject(reply);
    return true;
}

bool Redis::subscribe(int channel)
{
    /*
        do not use redisCommand, it would be blocked so that block your whole thread
        redisAppendCommand：write command to buffer，not block
        redisBufferWrite：write command from buffer to redis server，not block
    */
    if (redisAppendCommand(_subscribeContext, "SUBSCRIBE %d", channel) == REDIS_ERR)
    {
        cerr << "subscribe command append failed! channel = " << channel << endl;
        return false;
    }
    int done = 0;
    while (!done)
    {
        if (redisBufferWrite(_subscribeContext, &done) == REDIS_ERR)
        {
            cerr << "subscribe command write failed! channel = " << channel << endl;
            return false;
        }
    }
    return true;
}

bool Redis::unsubscribe(int channel)
{
    // use redisAppendCommand + redisBufferWrite to avoid blocking
    if (redisAppendCommand(_subscribeContext, "UNSUBSCRIBE %d", channel) == REDIS_ERR)
    {
        cerr << "unsubscribe command append failed! channel = " << channel << endl;
        return false;
    }
    int done = 0;
    while (!done)
    {
        if (redisBufferWrite(_subscribeContext, &done) == REDIS_ERR)
        {
            cerr << "unsubscribe command write failed! channel = " << channel << endl;
            return false;
        }
    }
    return true;
}

void Redis::observerChannelMessage()
{
    redisReply *reply = nullptr;
    while (redisGetReply(_subscribeContext, (void **)&reply))
    {
        // 解析订阅的消息格式：SUBSCRIBE的回复是 三元素数组
        // 数组内容：[0] = "message", [1] = 通道号(string), [2] = 消息内容(string)
        if (reply != nullptr && reply->element[2] != nullptr && reply->element[2]->str != nullptr)
        {
            // 取出通道号和消息内容
            string channel = reply->element[1]->str;
            string message = reply->element[2]->str;
            // 调用上层业务的回调函数，上报消息
            _notifyMessageHandler(atoi(channel.c_str()), message);
        }

        // 释放reply内存，防止内存泄漏【必须】
        freeReplyObject(reply);
    }
}

// 给上层业务设置回调函数，收到消息后通过该函数上报
void Redis::initNotifyHandler(function<void(int, string)> fn)
{
    _notifyMessageHandler = fn;
}
