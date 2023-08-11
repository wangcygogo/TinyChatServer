#ifndef DB_H
#define DB_H

#include <mysql/mysql.h>
#include <string>
using namespace std;

class MySQL
{
public:
    // 构造函数，初始化数据库连接
    MySQL();
    // 析构函数，释放数据库连接资源
    ~MySQL();
    // 连接数据库
    bool connect();
    // 获取连接状态
    MYSQL* getConnection();
    // 更新操作，包括增删改
    bool update(string sql);
    // 查询操作 查
    MYSQL_RES *query(string sql);

private:
    MYSQL *_conn;
};
#endif