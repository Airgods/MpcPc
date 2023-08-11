// 框架设置的头文件
#pragma once

#include "mprpcconfig.h"
#include "mprpcchannel.h"
#include "mprpccontroller.h"

// mprpc框架的基础类,负责一些框架的初始化操作
class MprpcApplication
{
public:
    static void Init(int argc, char **argv);
    static MprpcApplication& GetInstance();
    static MprpcConfig& GetConfig();
private:
    static MprpcConfig m_config;  // 如果将普通的成员变量放置在静态的方法中就不行,所以提供成静态    

    MprpcApplication(){}
    MprpcApplication(const MprpcApplication&) = delete; // 禁止通过 拷贝构造函数复制创造MprpcApplication类实例
    MprpcApplication(MprpcApplication&&) = delete;      // 禁止通过 移动构造函数移动创建MprpcApplication类实例
};