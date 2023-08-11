#include <iostream>
#include <string>
#include "user.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"

/*
利用mprpc框架将一个本地服务(UserService)发布成远程的RPC服务
UserService原本是一个本地服务,提供了两个进程内的本地方法,Login和GetFriendLists
这里属于对框架的应用
*/
// UserService继承自fixbug::UserServiceRpc,也就是RPC服务的接口定义
class UserService:public fixbug::UserServiceRpc // 使用在rpc服务发布端(rpc服务的提供者)
{
// 重写了两个虚函数Login,Register,实现自定义的本地服务逻辑
public:
    bool Login(std::string name, std::string pwd)
    {   
        std::cout<<"doing local service:Login"<<std::endl;
        std::cout<<"name:"<<name<<"pwd"<<pwd<<std::endl;
        return true;
    }

    bool Register(uint32_t id, std::string name, std::string pwd)
    {
        std::cout << "doing local service:Register " << std::endl;
        std::cout <<"id:"<< id << "name:" << "pwd:"<< std::endl;  
        return true;
    }

    /*
    重写基类UserServiceRpc的虚函数 下面这些方法都是框架直接调用的
    目的:将远程的RPC调用映射到了本地的业务逻辑方法中,并将结果进行封装以响应给调用方
    框架收到的的应该是远端的描述,包括方法、参数
    1.caller ===>Login(LoginRequest) => muduo=>callee  这部分都是框架直接在做的事
    2.callee ===>Login(LoginRequest) => 交到下面重写的Login方法上了   
    */
    void Login(::google::protobuf::RpcController* controller,
                       const ::fixbug::LoginRequest* request,
                       ::fixbug::LoginResponse* response,
                       ::google::protobuf::Closure* done)
    {
        // 框架给业务上报了请求参数LoginRequest,应用程序获取相应数据做业务
        std::string name = request->name();
        std::string pwd = request->pwd();

        // 做本地业务
        bool login_result = Login(name, pwd);

        // 把响应写入 包括错误码、错误消息、返回值
        fixbug::ResultCode* code = response->mutable_result();
        code->set_errcode(0);   // 未触发异常即没有错误
        code->set_errmsg("");   
        response->set_success(login_result);

        // 执行回调操作 执行响应消息的序列化和网络发送(都是由框架来完成的)
        done->Run();
    }
    // 继承这个类需要重写下Register->将本地注册服务发布成远程rpc服务->四个动作
    void Register(::google::protobuf::RpcController* controller, 
                       const ::fixbug::RegisterRequest* request,
                       ::fixbug::RegisterResponse* response,
                       ::google::protobuf::Closure* done)
    {
        uint32_t id = request->id();
        std::string name = request->name();
        std::string pwd = request->pwd();

        // 做本地业务
        bool ret = Register(id, name, pwd);

        // 为response填写响应值
        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("");
        response->set_success(ret);

        // 执行done的回调 
        done->Run(); 
    }
};

int main(int argc, char **argv)
{
    // 调用框架的初始化操作-读取IP地址和端口号 provider -i config.conf(读取网络服务器以及配置中心的ip地址和端口号)
    MprpcApplication::Init(argc, argv);

    // provider是一个网络服务对象,把UserService对象发布到rpc站点上
    RpcProvider provider;  
    // 在框架上发布服务使用->启动一个provider并且向provider上注册一个方法(userservice),从而将其发布成为远程的RPC服务
    provider.NotifyService(new UserService()); // 这里对应的是一个基类指针
    // provider.NotifyService(new ProductService()); 类似也可以写

    // 启动一个rpc服务发布节点, Run以后, 进程进入阻塞状态, 等待远程的rpc调用请求->进入login部分
    provider.Run();

    return 0;
}