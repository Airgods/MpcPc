#include "mprpccontroller.h"

MprpcController::MprpcController()
{
    m_failed = false;  // 错误状态'm_failed'初始化为false
    m_errText = " ";   // 错误信息m_errText初始化为空字符串
}
// 重置控制器
void MprpcController::Reset()
{
    m_failed = false;
    m_errText = "";
}
// 用于返回 RPC 方法执行过程中是否出现了错误
bool MprpcController::Failed() const
{
    return m_failed;
}
// 用于返回 RPC 方法执行过程中的错误信息
std::string MprpcController::ErrorText() const
{
    return m_errText;
}
// 用于设置 RPC 方法执行过程中的错误状态和错误信息
void MprpcController::SetFailed(const std::string& reason)
{
    m_failed  = true;
    m_errText = reason;
}

// 目前未实现具体的功能
void MprpcController::StartCancel(){}
bool MprpcController::IsCanceled() const {return false;}
void MprpcController::NotifyOnCancel(google::protobuf::Closure* callback){}