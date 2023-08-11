#include <iostream>
#include <string>
#include "friend.pb.h"  
#include "mprpcapplication.h"
#include "rpcprovider.h"
#include <vector>
#include "logger.h"

class FriendService: public fixbug::FriendServiceRpc
{
public:
    std::vector<std::string> GetFriendsList(uint32_t)
    {
        std::cout<<"do GetFriendList service! userid:" <<std::endl; // 少一个userid
        std::vector<std::string> vec;
        vec.push_back("gao yang");
        vec.push_back("liu hong");
        vec.push_back("wang shuo");
        return vec;
    }
    
    // 此方法由框架负责调用->重写基类方法
    void GetFriendList(::google::protobuf::RpcController* controller,
                    const ::fixbug::GetFriendListRequest* request,
                    ::fixbug::GetFriendListResponse* response,
                    ::google::protobuf::Closure* done)
    {
        uint32_t userid = request->userid();
        // 将本地服务发布成rpc服务
        std::vector<std::string> friendList = GetFriendsList(userid);
        // 把数据写入response中
        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("");

        response->add_friends();
        for(std::string &name:friendList)
        {
            std::string *p = response->add_friends(); // 这里怎么是对protobuf的操作
            *p = name;
        }
        done->Run();
    }
};

int main(int argc, char **argv)
{   
    LOG_INFO("first log message!\n");
    LOG_ERR("%s:%s%d", __FILE__, __FUNCTION__, __LINE__);

    // 调用框架的初始化操作
    MprpcApplication::Init(argc, argv);

    // provider是一个rpc网络服务对象,把UserService对象发布到rpc节点上
    RpcProvider provider;
    provider.NotifyService(new FriendService());

    // 启动一个rpc服务发布节点, Run以后,进程进入阻塞状态,等待远程的rpc调用请求
    provider.Run();

    return 0;
}