#include "friend.hpp"
#include <vector>
#include "user.hpp"
class FriendModel
{
public:
    // add friend
    bool insert(const int &userid, const int &friendid);
    bool query(const int &userid, std::vector<User>& friends);
};