#pragma once

#include <string>
#include "user.hpp"

using std::string;

// operate User table, edit sql sentance
class UserModel
{
public:
    // add
    bool insert(User& user);
    bool query(const int& id, User& user);
    bool update(User& user);
    bool closeAll();
};

