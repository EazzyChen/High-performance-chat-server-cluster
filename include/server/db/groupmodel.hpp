#include "group.hpp"
#include "groupuser.hpp"

class GroupModel
{
public:
    bool insert(Group &group);
    bool joinGroup(const int& userid, const int& groupid, const string& role);
    bool queryGroups(const int& userid, vector<Group>& groupVec);
    bool queryGroupUsersInfo(const int& groupid, vector<GroupUser>& groupUserVec);
    bool broadcastUsersId(const int& userid, const int& groupid, vector<int>& idVec);
};