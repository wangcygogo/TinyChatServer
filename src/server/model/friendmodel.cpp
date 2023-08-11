#include "friendmodel.hpp"
#include "db.h"
// 添加好友关系
void FriendModel::insert(int userid, int friendid)
{
    char sql[1024] = {0};
    // 1. 组装sql语句
    sprintf(sql, "insert into Friend values(%d,%d)", userid, friendid);
    MySQL mysql;
    // 2.连接数据库进行操作
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}
// 返回用户好友列表
vector<User> FriendModel::query(int userid)
{
    char sql[1024] = {0};
    // 1. 组装sql语句
    sprintf(sql, "select a.id,a.name,a.state from User a inner join Friend b on b.friendid=a.id where b.userid=%d", userid);
    vector<User> vec;
    MySQL mysql;
    // 2.连接数据库进行操作
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            // 从结果中提取出一行
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                vec.push_back(user);
            }
            mysql_free_result(res);
        }
    }
    return vec;
}