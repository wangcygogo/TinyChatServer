#include "chatservice.hpp"
#include "public.hpp"
#include <muduo/base/Logging.h>
using namespace std;
using namespace muduo;
// 获取单例对象的接口函数
ChatService *ChatService::instance()
{
    static ChatService service;
    return &service;
}
// 构造函数
ChatService::ChatService()
{
    // 注意需要绑定器
    // 登录信息
    _msgHandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2, _3)});
    // 注册
    _msgHandlerMap.insert({REG_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)});
    // 一对一聊天
    _msgHandlerMap.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, _1, _2, _3)});
    // 添加好友
    _msgHandlerMap.insert({ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, _1, _2, _3)});
     // 处理客户端异常退出
    _msgHandlerMap.insert({LOGINOUT_MSG, std::bind(&ChatService::loginout, this, _1, _2, _3)});
    //创建群组
    _msgHandlerMap.insert({CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, _1, _2, _3)});
    //加入群组
    _msgHandlerMap.insert({ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this, _1, _2, _3)});
    //群组聊天
    _msgHandlerMap.insert({GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this, _1, _2, _3)});
    //连接redis
    if (_redis.connect()){
        //设置上传消息回调
        _redis.init_notify_handler(std::bind(&ChatService::handleRedisSubscribeMessage, this, _1, _2));
    }
}
// 获取id对应的处理器方法
MsgHandler ChatService::getHandler(int msgid)
{
    // 记录错误日志，msgid没有对应的事件回调
    // 使用muduo库自带的日志方法
    auto it = _msgHandlerMap.find(msgid);
    if (it == _msgHandlerMap.end())
    {
        // 返回一个默认的处理器，空操作
        return [=](const TcpConnectionPtr &conn, json &js, Timestamp time)
        {
            LOG_ERROR << "msgid:" << msgid << " can not find handler!";
        };
    }
    else
    {
        return _msgHandlerMap[msgid];
    }
}
// 处理登录
void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int id = js["id"].get<int>();
    string pwd = js["password"];

    User user = _userModel.query(id);
    if (user.getId() == id && user.getPwd() == pwd)
    {
        if (user.getState() == "online")
        {
            // 已经登录 不允许重复登录
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2; // 已经登录标识
            response["errmsg"] = "该账号已经登录，请重新输入账号";
            conn->send(response.dump()); // 通过TCP连接发送 将js导出成字符串发送
        }
        else
        {
            // 登录成功
            {
                // lock_guard是自动释放的，离开作用域析构函数释放锁
                // 加上{}限定作用域，把锁的粒度减小
                lock_guard<mutex> lock(_connMutex);
                _userConnMap.insert({id, conn});
            }
            //id登录成功，向redis订阅channel
            _redis.subscribe(id);
            // 1.更新用户状态信息 offline->online
            user.setState("online");
            _userModel.updateState(user);
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 0; // 登录成功标识
            response["id"] = user.getId();
            response["name"] = user.getName();
            // 查询是否有离线消息
            vector<string> vec = _offlineMsgModel.query(id);
            if (!vec.empty())
            {
                response["offlinemsg"] = vec;
                // 读取用户的离线消息后，将用户离线消息删除
                _offlineMsgModel.remove(id);
            }
            vector<User> usrvec = _friendModel.query(id);
            if (!usrvec.empty())
            {
                json js;
                vector<string> jsvec;
                for (User &user : usrvec)
                {
                    js["id"] = user.getId();
                    js["state"] = user.getState();
                    js["name"] = user.getName();
                    jsvec.push_back(js.dump());
                }
                response["friends"] = jsvec;
            }
            conn->send(response.dump()); // 通过TCP连接发送 将js导出成字符串发送
        }
    }
    else
    {
        // 登录失败
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1;                   // 登录失败标识
        response["errmsg"] = "用户名或密码错误"; // 错误信息
        conn->send(response.dump());
    }
}
// 处理注册
void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    string name = js["name"];
    string pwd = js["password"];
    User user;
    user.setName(name);
    user.setPwd(pwd);
    bool state = _userModel.insert(user);
    if (state)
    {
        // 注册成功
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 0; // 注册成功标识
        response["id"] = user.getId();
        conn->send(response.dump()); // 通过TCP连接发送 将js导出成字符串发送
    }
    else
    {
        // 注册失败
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1; // 注册失败标识
        // 可以在这里设置一个注册失败的返回消息
        conn->send(response.dump());
    }
}
// 处理异常退出
void ChatService::clientCloseException(const TcpConnectionPtr &conn)
{
    User user;
    {
        lock_guard<mutex> lock(_connMutex);
        for (auto iter = _userConnMap.begin(); iter != _userConnMap.end(); iter++)
        {
            if (iter->second == conn)
            {
                // 从map表删除用户连接信息
                user.setId(iter->first);
                _userConnMap.erase(iter);
                break;
            }
        }
    }
    //注销，即下线
    _redis.unsubscribe(user.getId());
    // 有效用户更新id值
    if (user.getId() != -1)
    {
        user.setState("offline");
        _userModel.updateState(user);
    }
}
// 一对一聊天
void ChatService::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int toid = js["toid"].get<int>();
    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(toid);
        if (it != _userConnMap.end())
        {
            // toid在线，转发消息 服务器主动推送消息给toid用户
            it->second->send(js.dump());
            return;
        }
    }
    User user=_userModel.query(toid);
    if (user.getState()=="online"){
        _redis.publish(toid,js.dump());
        return;
    }
    // toid 不在线存储离线消息
    _offlineMsgModel.insert(toid, js.dump());
}
void ChatService::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    vector<int> useridVec = _groupModel.queryGroupUsers(userid, groupid);

    lock_guard<mutex> lock(_connMutex);
    for (int id : useridVec)
    {
        auto it = _userConnMap.find(id);
        if (it != _userConnMap.end())
        {
            // 转发群消息
            it->second->send(js.dump());
        }
        else
        {
            // 查询toid是否在线 
            User user = _userModel.query(id);
            if (user.getState() == "online")
            {
                _redis.publish(id, js.dump());
            }
            else
            {
                // 存储离线群消息
                _offlineMsgModel.insert(id, js.dump());
            }
        }
    }
}
// 异常退出 重置状态信息
void ChatService::reset()
{
    // 把online状态的用户设置为offline
    _userModel.resetState();
}
// 添加好友业务
void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();
    _friendModel.insert(userid, friendid);
}
 // 处理客户端退出
void ChatService::loginout(const TcpConnectionPtr &conn, json &js, Timestamp time){
    int userid = js["id"].get<int>();
    {
        lock_guard<mutex> lock(_connMutex);
        auto it=_userConnMap.find(userid);
        if (it!=_userConnMap.end()){
            _userConnMap.erase(it);
        }
    }
    //取消订阅
    _redis.unsubscribe(userid);
    User user(userid,"","","offline");
    _userModel.updateState(user);
    
 }
void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    string name = js["groupname"];
    string desc = js["groupdesc"];

    // 存储新创建的群组信息
    Group group(-1, name, desc);
    if (_groupModel.createGroup(group))
    {
        // 存储群组创建人信息
        _groupModel.addGroup(userid, group.getId(), "creator");
    }
}
// 加入群组业务
void ChatService::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    _groupModel.addGroup(userid, groupid, "normal");
}
//获取订阅消息
void ChatService::handleRedisSubscribeMessage(int userid,string msg){
    lock_guard<mutex> lock(_connMutex);
    //解决通过redis传输时用户下线问题
    auto it=_userConnMap.find(userid);
    if (it!=_userConnMap.end()){
        it->second->send(msg);
        return ;
    }
    //存储离线消息
    _offlineMsgModel.insert(userid,msg);
}