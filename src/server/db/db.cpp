#include <muduo/base/Logging.h>
#include "db.h"
#include <muduo/base/Logging.h>
static string server = "127.0.0.1";
static string user = "root";
static string password = "123456";
static string dbname = "chat";
// 构造函数，初始化数据库连接
MySQL::MySQL()
{
    _conn = mysql_init(nullptr);
}
// 析构函数，释放数据库连接资源
MySQL::~MySQL()
{
    if (_conn != nullptr)
        mysql_close(_conn);
}
// 连接数据库
bool MySQL::connect()
{
    // 连接数据，通过用户名、密码、服务器、数据库的名字、端口号进行连接
    MYSQL *p = mysql_real_connect(_conn, server.c_str(), user.c_str(), password.c_str(),
                                  dbname.c_str(), 3306, nullptr, 0);
    // mysql默认为ASCII编码，将其转换为gdk能够编码中文
    if (p != nullptr)
    {
        mysql_query(_conn, "set names gdk");
        LOG_INFO << "connect mysql success!";
    }
    else
    {
        LOG_INFO << "connect mysql fail!";
    }
    return p;
}
// 更新操作，包括增删改
bool MySQL::update(string sql)
{
    if (mysql_query(_conn, sql.c_str()))
    {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":" << sql << "更新失败！";
        return false;
    }
    return true;
}
// 查询操作 查
MYSQL_RES *MySQL::query(string sql)
{
    // 查不到 输出异常 查到了 放在_conn对象中
    if (mysql_query(_conn, sql.c_str()))
    {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":" << sql << "查询失败！";
        return nullptr;
    }
    return mysql_use_result(_conn);
}
// 获取连接状态
MYSQL *MySQL::getConnection() { return _conn; }