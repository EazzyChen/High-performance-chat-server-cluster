#pragma once
#include "user.hpp"
#include <string>
using std::string;
class GroupUser: public User 
{
public:
    string getRole() const {return this->role;}
    void setRole(string role){this->role = role;}
private:
    string role;
};