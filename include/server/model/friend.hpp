#pragma once
class Friend
{
public:
    int getUserid() const {return this->userid;}
    int getFriendid() const {return this->friendid;}
    void setUserid(const int& id){this->userid = id;}
    void setFriendid(const int& id){this->friendid = id;}
private:
    int userid;
    int friendid;
};