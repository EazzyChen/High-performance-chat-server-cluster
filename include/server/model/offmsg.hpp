#pragma once
#include <vector>
#include <string>
#include "json.hpp"
using json = nlohmann::json;
class OffMsg
{
public:
    int getUserId() const { return this->userid; }
    json getMessage() const {return this->message;}
    void setUserId(int userid) {this->userid = userid;}
    void setMessage(json message) {this->message = message;}

private:
    int userid;
    json message;
};