#pragma once
#include "google/protobuf/service.h"
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h> 
#include <muduo/net/TcpConnection.h>
#include <string>
#include <functional>
#include <google/protobuf/descriptor.h>
#include <unordered_map>

// 框架提供的专门发布rpc服务的网络对象类
class RpcProvider
{
public:
    // 这是框架提供给外部使用的,可以发布rpc方法的函数接口
    // 注册一个需要发布的RPC服务,通过传入"google::protobuf::Service"对象,可以获得
    // 该服务的类型信息和方法描述,并存储在m_serviceMap中,以备后续网络调用使用
    void NotifyService(google::protobuf::Service *service);

    // 启动rpc节点,开始提供rpc远程网络调用服务
    // 函数中启动一个 muduo::net::EventLoop 来监听和处理网络连接和数据的传输
    void Run();

private:
    // 组合EventLoop
    muduo::net::EventLoop m_eventLoop;
 
    // service服务类型信息
    struct ServiceInfo
    {
        google::protobuf::Service* m_service; // 保存服务对象
        std::unordered_map<std::string, const google::protobuf::MethodDescriptor*> m_methodMap; // 保存服务方法
    };
    
    // 存储注册成功的服务对象和其服务方法的所有信息
    std::unordered_map<std::string, ServiceInfo> m_serviceMap; 

    // 新的socket连接回调,建立连接时被调用
    void OnConnection(const muduo::net::TcpConnectionPtr&);
    // 已建立连接用户的读写回调,接收到客户端的数据时被调用
    void OnMessage(const muduo::net::TcpConnectionPtr&, muduo::net::Buffer*, muduo::Timestamp);
    // Closure回调操作,用于序列化rpc的响应和网络发送
    // 将响应消息序列化并通过网络发送给客户端
    void SendRpcResponse(const muduo::net::TcpConnectionPtr&, google::protobuf::Message*);
};