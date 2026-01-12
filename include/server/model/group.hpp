#pragma once
#include <vector>
#include <string>
#include "groupuser.hpp"
using std::vector;
using std::string;
class Group
{
public:
    int getGroupId() const {return this->groupid;}
    string getGroupName() const {return this->groupname;}
    string getGroupDesc() const {return this->groupdesc;}

    void setGroupId(const int& id){this->groupid = id;}
    void setGroupName(const string& name){this->groupname = name;}
    void setGroupDesc(const string& desc){this->groupdesc = desc;}
   
    
private:
    int groupid;
    string groupname;
    string groupdesc;

};