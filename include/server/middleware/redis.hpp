#pragma once
#include <hiredis/hiredis.h>
#include <string>
#include <functional>
using std::string;

class Redis
{
public:
    Redis();
    ~Redis();
    // connect redis
    bool connect();
    // publish message to channel
    bool publish(int channel, string message);
    // subscribe channel
    bool subscribe(int channel);
    // cancel subscribe channel
    bool unsubscribe(int channel);
    // receive channel message in thread
    void observerChannelMessage();
    // initial channel message callback function
    void initNotifyHandler(std::function<void(int, string)> fn);
private:
    redisContext *_publishContext;
    redisContext *_subscribeContext;
    std::function<void(int, string)> _notifyMessageHandler;
};