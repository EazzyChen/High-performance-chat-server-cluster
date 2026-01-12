#pragma once

enum class EnMsgType
{
    ACK = 0,
    LOGIN_MSG,          // 1 for login message
    LOGIN_MSG_ACK,      // 2 for login ack
    REGIS_MSG,          // 3 for register message
    REG_MSG_ACK,        // 4 for register ack
    ONE_CHAT_MSG,       // 5 for chat one to one
    ADD_FRIEND_MSG,     // 6 for add friend
    CREATE_GROUP_MSG,   // 7 for create group
    JOIN_GROUP_MSG,     // 8 for join group
    GET_GROUPLIST,      // 9 for get group list
    GET_GROUP_USERINFO, // 10 for get group user infomation
    GROUP_CHAT_MSG,     // 11 for group chat message
    GET_FRIENDS         // 12 for get friends
};