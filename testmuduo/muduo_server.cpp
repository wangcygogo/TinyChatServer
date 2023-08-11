#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <functional>
#include <iostream>
using namespace std;
using namespace muduo;
using namespace muduo::net;
using namespace placeholders;
class ChatServer
{
public:
    ChatServer(EventLoop *loop,               // 事件循环
               const InetAddress &listenAddr, // IP+端口
               const string &nameArg)         // 服务器的名字
        : _server(loop, listenAddr, nameArg), _loop(loop)
    {
        // 给服务器注册用户连接的创建和断开回调
        // setConnectionCallback只需要一个参数，但实际onConnectionCallback有一个隐藏的形参this,_1表示占位符，还有一个参数
        _server.setConnectionCallback(bind(&ChatServer::onConnection, this, _1));

        // 给服务器注册用户读写事件回调
        _server.setMessageCallback(bind(&ChatServer::onMessage, this, _1, _2, _3));

        // 设置线程数量
        // 1个IO线程，3个worker线程
        _server.setThreadNum(4);
    }
    // 开启事件循环
    void start()
    {
        _server.start();
    }

private:
    // 专门处理用户的连接创建和断开
    void onConnection(const TcpConnectionPtr &conn)
    {
        if (conn->connected())
        {
            // 输出对端IP和本地端口
            cout << conn->peerAddress().toIpPort() << "->"
                 << conn->localAddress().toIpPort() << endl;
        }
        else
        {
            cout << conn->peerAddress().toIpPort() << "->"
                 << conn->localAddress().toIpPort() << endl;
            conn->shutdown(); // 连接断开，close(fd) socket资源释放
            //_loop->quit();  // 服务器结束
        }
    }
    void onMessage(const TcpConnectionPtr &conn, // 连接
                   Buffer *buffer,               // 缓冲区
                   Timestamp time)               // 接收数据的事件信息
    {
        string buf = buffer->retrieveAllAsString();
        cout << "recv data:" << buf << "time:" << time.toString() << endl;
        conn->send(buf);
    }
    TcpServer _server;
    EventLoop *_loop; // 类似于epoll，事件循环对象的指针
};
int main()
{
    EventLoop loop;
    InetAddress addr("127.0.0.1", 6000);
    ChatServer server(&loop, addr, "ChatServer");
    server.start(); // listenfd 通过epoll_ctl添加到epoll
    loop.loop();    // epoll_wait以阻塞方式等待新用户连接，已连接用户的读写、断开
    return 0;
}