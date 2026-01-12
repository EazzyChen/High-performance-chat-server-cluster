#pragma once
#include <string>

using std::string;

class User
{
public:
    User(int id = -1, string name = "", string pwd = "", string state = "offline")
    : id(id), name(name), password(pwd), state(state)
    {
        
    }
    void setId(const int &id)
    {
        this->id = id;
    }
    void setName(const string &name)
    {
        this->name = name;
    }
    void setPwd(const string &pwd)
    {
        this->password = pwd;
    }
    void setState(const string &state)
    {
        this->state = state;
    }

    int getId() const { return this->id; }
    string getName() const { return this->name; }
    string getPwd() const { return this->password; }
    string getState() const { return this->state; }

private:
    int id;
    string name;
    string password;
    string state;
};