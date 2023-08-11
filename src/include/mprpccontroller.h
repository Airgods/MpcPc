#pragma once
#include <google/protobuf/service.h>
#include <string.h>

// 自定义类,用于管理和控制远程过程调用(RPC)过程中的状态和信息错误
// 用于在Rpc方法执行过程中管理错误状态和信息,以及一些取消操作的处理,支持自定义的RPC控制功能
class MprpcController:public google::protobuf::RpcController
{
public: 
    MprpcController();  // 用于初始化MprpcController对象的成员变量
    void Reset();       // 用于重置控制器的状态，即将状态恢复到初始状态
    bool Failed() const;  // 用于判断 RPC 方法执行过程中是否出现错误。如果错误已经设置，它将返回 true，否则返回 false。
    std::string ErrorText() const; // 用于获取 RPC 方法执行过程中的错误信息
    void SetFailed(const std::string& reason); // 用于设置 RPC 方法执行过程中的错误状态和错误信息。传入的 reason 参数是一个字符串，表示错误的原因

    // 目前未实现具体的功能
    void StartCancel();      // 用于启动取消 RPC 方法的操作
    bool IsCanceled() const; // 用于判断是否已经取消了 RPC 方法的操作
    void NotifyOnCancel(google::protobuf::Closure* calllback); // 用于在取消 RPC 方法时通知相关的回调函数
private:
    bool m_failed;         // Rpc方法执行过程中的状态
    std::string m_errText; // Rpc方法执行过程中的错误信息
};