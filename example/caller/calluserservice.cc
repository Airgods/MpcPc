#include <iostream>
#include "mprpcapplication.h"
#include "user.pb.h"
#include "mprpcchannel.h"
/*
这里可以改变的在于你要封装的方法,取决于你要调用的方式
*/
int main(int argc, char** argv)
{
    // 整个程序启动以后,想使用mprpc框架来享受rpc服务调用,一定需要先调用框架的初始化函数,且只初始化1次
    MprpcApplication::Init(argc, argv);
    
    // 演示调用远程发布的rpc方法Login 即 可以使用protobuf上生成的代码调用远程服务器上的RPC方法
    fixbug::UserServiceRpc_Stub stub(new MprpcChannel());
    
    // rpc方法的请求参数(因为是我调用,所以参数是我负责给予,构造LoginRequest请求)
    // 设置用户名和密码
    fixbug::LoginRequest request;
    request.set_name("zhang san");
    request.set_pwd("123456");

    // 创建LoginResponse响应对象
    fixbug::LoginResponse response; // 响应是另一方给予
    
    // 发起rpc方法的调用 同步的rpc调用过程 底层是Mprpcchannel::callmethod的方法的调用
    // 这里调用的方式是同步阻塞 | 发起同步的RPC调用,等待返回结果
    stub.Login(nullptr, &request, &response, nullptr); // RpcChannel-   >RpcChannel::callMethod 集中来做所有rpc方法调用的参数序列化和网络发送
    
    // 解析RPC调用结果并输出
    if(response.result().errcode() == 0)
    {
        std::cout<< "rpc login response:" << response.success() << std::endl;
    }else
    {
        std::cout<< "rpc login response error:" << response.result().errmsg() <<std::endl;
    }

    // 构造RegisterRequest请求,设置注册信息
    fixbug::RegisterRequest req;
    req.set_id(2000);
    req.set_name("mprpc");
    req.set_pwd("666666");

    // 创建RegisterResponse响应对象
    fixbug::RegisterResponse rsp;

    // 以同步的方式发起rpc调用请求,等待返回结果
    stub.Register(nullptr, &req, &rsp, nullptr);

    // 解析RPC调用结果,输出成功与否的结果
    if(rsp.result().errcode() == 0)
    {   
        std::cout<< "rpc register response success: "<< response.success() << std::endl;
    }
    else
    {
        std::cout<< "rpc register response error: "<< response.result().errmsg() << std::endl;
    }

    return 0;
}
