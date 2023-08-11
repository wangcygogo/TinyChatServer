#ifndef FRIENDMODEL_H
#define FRIENDMODEL_H
#include "user.hpp"
#include <vector>
// 维护好友信息的操作接口方法
class FriendModel{
public:
    // 添加好友关系
    void insert(int userid,int friendid);
    // 理论上应该存在本地 可以减少消息量
    //返回用户好友列表
    vector<User> query(int userid);
};
#endif