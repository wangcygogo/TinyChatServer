#基于linux平台的聊天服务器，由服务端、数据库、客户端、nginx负载均衡、redis消息队列模块组成
文件作用
bin：存放可执行文件
lib：库文件
include：头文件
    server:服务器相关头文件
        db:数据库头文件
            db.hpp 对数据库封装的头文件
        chatserver.hpp 服务器头文件
    chatservice.hpp 业务头文件
        friendmodel.hpp 添加好友，操作Friend表头文件
        offlinemessagemodel.cpp 存储、删除、查询离线消息头文件
        user.hpp 用户信息头文件
        usermodel.hpp 操作user表头文件
    public.hpp
src：源文件
    client
    server:服务器相关
        db:数据库
            db.cpp
        chatserver.cpp 服务器
        chatservice.cpp 业务
        friendmodel.cpp 操作Friend表
        offlinemessagemodel.cpp 存储、删除、查询离线消息
        user.cpp 用户
        usermodel.cpp 操作user表
    public.cpp
build：cmake产生的中间文件
example
thridparty：第三方库
    json.hpp 进行json解析和生成
CMakeLists.txt
autobuild.sh：自动生成脚本
