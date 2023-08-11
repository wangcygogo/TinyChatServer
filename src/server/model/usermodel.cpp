#include "usermodel.hpp"
#include "db.h"
#include <iostream>
using namespace std;
// User表的增加方法
bool UserModel::insert(User &user)
{
    char sql[1024] = {0};
    // 1. 组装sql语句
    sprintf(sql, "insert into User(name,password,state) values('%s','%s','%s')",
            user.getName().c_str(), user.getPwd().c_str(), user.getState().c_str());
    MySQL mysql;
    // 2.连接数据库进行操作
    if (mysql.connect())
    {
        // 调用封装的update语句进行插入操作
        if (mysql.update(sql))
        {
            // 获取插入成功用户数据生成的主键id
            user.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}
// 根据用户id查询用户信息
User UserModel::query(int id)
{
    char sql[1024] = {0};
    // 1. 组装sql语句
    sprintf(sql, "select * from User where id = %d", id);
    MySQL mysql;
    // 2.连接数据库进行操作
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            // 从结果中提取出一行
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row != nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPwd(row[2]);
                user.setState(row[3]);
                mysql_free_result(res);
                return user;
            }
        }
    }
    return User();
}
// 更新用户状态信息
bool UserModel::updateState(User user)
{
    char sql[1024] = {0};
    // 1. 组装sql语句
    sprintf(sql, "update User set state='%s' where id='%d'", user.getState().c_str(), user.getId());
    MySQL mysql;
    // 2.连接数据库进行操作
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            return true;
        }
    }
    return false;
}
void UserModel::resetState()
{
    char sql[1024] = "update User set state='offline' where state='online'";
    MySQL mysql;
    // 2.连接数据库进行操作
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}