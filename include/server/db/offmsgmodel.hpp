#pragma once
#include <string>
#include <vector>
#include "offmsg.hpp"
using std::string;
using std::vector;
// offline message model
class OffMsgModel
{
public:
    // store message
    bool insert(const int& userid, const string& message);
    // delete message
    bool remove(const int& userid);
    // inquiry message
    bool query(const int& userid, OffMsg& offMsg);
};

