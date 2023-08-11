#pragma once
#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>

// 实现用于处理远程过程调用通信的类MprpcChannel,是使用ProtoBuf进行远程过程调用的一部分
// 会将请求消息序列化成字节流,会将请求消息序列化成字节流,然后等待服务器的响应，
// 将响应字节流反序列化成消息对象，最终调用回调函数 done 来表示调用完成。
class MprpcChannel:public google::protobuf::RpcChannel
{
public:
    // 所有通过stub代理对象调用的rpc方法,都走到这里了,统一做rpc方法调用的数据数据序列化和网络发送
    void CallMethod(const google::protobuf::MethodDescriptor* method,  // 表示要调用的Rpc方法的描述符,包含方法的名称、参数等信息
                    google::protobuf::RpcController* controller,   // 用于控制RPC调用的对象,用于中止调用,设置错误信息等
                    const google::protobuf::Message* request,  // 表示调用方法时发送的请求消息，这是一个 ProtoBuf消息对象
                    google::protobuf::Message* response,  // 表示接收到的响应消息，是一个 ProtoBuf消息对象
                    google::protobuf::Closure* done); // 当 RPC 调用完成时会调用的回调函数对象
};