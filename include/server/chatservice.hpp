#ifndef CHATSERVICE_H
#define CHATSERVICE_H
#include "json.hpp"
#include "usermodel.hpp"
#include "offlinemessagemodel.hpp"
#include "friendmodel.hpp"
#include "groupmodel.hpp"
#include "redis.hpp"
#include <muduo/net/TcpConnection.h>
#include <unordered_map>
#include <functional>
#include <mutex>
using namespace std;
using namespace muduo::net;
using namespace muduo;
using json = nlohmann::json;
// 表示处理消息的事件回调方法类型
using MsgHandler = std::function<void(const TcpConnectionPtr &conn, json &js, Timestamp)>;
// 聊天服务器业务类
class ChatService
{
public:
    // 获取单例对象的接口函数
    static ChatService *instance();
    // 处理登录
    void login(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 处理注册
    void reg(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 处理异常退出
    void clientCloseException(const TcpConnectionPtr &conn);
    // 一对一聊天
    void oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time);
    //创建群组
    void createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);
    //加入群组
    void addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);
    //群组聊天
    void groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 添加好友业务
    void addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 处理客户端异常退出
    void loginout(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 异常退出 重置状态信息
    void reset();
    // 获取id对应的处理器方法
    MsgHandler getHandler(int msgid);
    void handleRedisSubscribeMessage(int channel,string msg);

private:
    // 构造函数
    ChatService();
    // 消息处理器的map表 消息id对应的处理操作
    unordered_map<int, MsgHandler> _msgHandlerMap;
    // 存储在线用户的通信连接
    unordered_map<int, TcpConnectionPtr> _userConnMap;
    // 定义互斥锁，保证_userConnMap的线程安全
    mutex _connMutex;
    // 数据操作类对象
    UserModel _userModel;
    // 离线消息操作类对象
    OfflineMsgModel _offlineMsgModel;
    FriendModel _friendModel;
    GroupModel _groupModel;

    Redis _redis;
};
#endif
