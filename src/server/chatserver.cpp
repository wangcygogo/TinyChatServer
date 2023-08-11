#include "chatserver.hpp"
#include "json.hpp"
#include "chatservice.hpp"
#include <functional>
using json=nlohmann::json;
using namespace std;
using namespace placeholders;
// 初始化聊天服务器对象
ChatServer::ChatServer(EventLoop *loop,
                       const InetAddress &listenAddr,
                       const string &nameArg) : _server(loop, listenAddr, nameArg), _loop(loop)
{
    _server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));

    _server.setMessageCallback(std::bind(&ChatServer::onMessage, this, _1, _2, _3));

    _server.setThreadNum(4);
}
// 启动服务
void ChatServer::start()
{
    _server.start();
}
// 上报链接相关信息的回调函数
void ChatServer::onConnection(const TcpConnectionPtr &conn)
{
    // 用户断开连接，释放资源，文件描述符
    if (!conn->connected()){
        ChatService::instance()->clientCloseException(conn);
        conn->shutdown();
    }
}
// 上报读写事件的回调函数
void ChatServer::onMessage(const TcpConnectionPtr &conn,
                           Buffer *buffer,
                           Timestamp time)
{
    string buf=buffer->retrieveAllAsString();
    // 数据的反序列化
    json js=json::parse(buf);

    // 为了解耦服务处理和响应，解耦网络模块的代码和业务模块的代码
    // 通过js["msgid"]获取 业务handeler
    auto msgHandler=ChatService::instance()->getHandler(js["msgid"].get<int>());
    // 这里的get<int> 是实例化一个函数 然后把msgid强转为int

    // 回调消息绑定好的事件处理器，来执行相应的业务
    msgHandler(conn,js,time);

}