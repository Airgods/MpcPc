#include <iostream>
#include "mprpcapplication.h"
#include "friend.pb.h"

// 这些服务都可以单独发布在一个rpc进程上,或者部署在一台机器上
int main(int argc, char **argv)
{   
    // 整个程序启动以后,想使用mprpc框架来享受rpc服务调用,一定需要先调用框架的初始化函数,且只初始化1次
    MprpcApplication::Init(argc, argv);

    // 演示调用远程发布的rpc方法Login
    fixbug::FriendServiceRpc_Stub stub(new MprpcChannel()); // 创建代理对象,做数据的序列化和反序列化
    
    // 因为是我发起的调用,所以参数是我负责给予-组装参数
    // rpc方法的请求参数
    fixbug::GetFriendListRequest request;
    request.set_userid(1000);
    // rpc方法的响应
    fixbug::GetFriendListResponse response; // 响应是另一方给予
    // 发起rpc方法的调用 同步的rpc调用过程 底层是Mprpcchannel::callmethod的方法的调用
    // 这里调用的方式是同步阻塞
    MprpcController controller; 
    //stub.GetFriendList(nullptr, &request, &response, nullptr); // RpcChannel-   >RpcChannel::callMethod 集中来做所有rpc方法调用的参数序列化和网络发送
    stub.GetFriendList(&controller, &request, &response, nullptr);

    //调用完成后直接调用response是一种比较理想的情况,这里如果调用response出错那么就要依靠controller解决
    // 一次rpc调用完成,读调用的结果
    if(controller.Failed())
    {
        std::cout << controller.ErrorText() << std::endl;
    }
    else
    {
        if(response.result().errcode() == 0)
        {
            std::cout<< "rpc GetFriendList response!" << std::endl;
            int size = response.friends_size();
            for(int i = 0;i<size;++i)
            {
                std::cout<<"index:"<< (i+1) << "name:"<<response.friends(i)<<std::endl;
            }    
        }
        else
        {
            std::cout<< "rpc GetFriendList response error:" << response.result().errmsg() <<std::endl;
        }
    }


    return 0;
}