#include "mprpcchannel.h"
#include "string.h"
#include "rpcheader.pb.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include "mprpcapplication.h"
#include "mprpccontroller.h"
#include "zookeeperutil.h"

/*
header_size + service_name method_name args_size + args
*/
// 所有通过stub代理对象调用的rpc方法,都走到这里
// 最终所有的调用代码都转到mprpcchannle的callmethod
// 实现一个自定义的Rpc通道,用于将客户端的Rpc请求发送到服务器并处理响应,
// 通道封装了请求和响应的序列化、网络通信、以及响应的解析等功能
// 简单概括是完成了与RPC服务器的连接、请求发送、响应接收以及反序列化响应数据的过程
// 实现了TCP编程实现远程过程调用
void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                    google::protobuf::RpcController* controller, 
                    const google::protobuf::Message* request,
                    google::protobuf::Message* response, 
                    google::protobuf::Closure* done)
{   

    const google::protobuf::ServiceDescriptor* sd = method->service();
    std::string service_name = sd->name();    // service_name
    std::string method_name = method->name(); // method_name

    // 将请求消息request序列化为字符串,并计算其大小
    // 获取参数的序列化字符串长度args_size
    uint32_t args_size = 0;
    std::string args_str;
    if(request->SerializeToString(&args_str))
    {
        // 序列化成功
        args_size = args_str.size();
    }
    else
    {
        // 序列化失败
        std::cout << "Serialize request error! " << std::endl;
        controller->SetFailed("serialize request error!");
        return;
    }

    // 定义rpc的请求header
    // 创建一个mprpc::RpcHeader对象 rpcHeader,并设置其中的服务名,方法名和参数大小
    // 使用SerializeToString函数将rpcheader序列化为字符串,并计算其大小
    mprpc::RpcHeader rpcHeader;
    rpcHeader.set_service_name(service_name);
    rpcHeader.set_method_name(method_name);
    rpcHeader.set_args_size(args_size);
    
    uint32_t header_size = 0;
    std::string rpc_header_str;
    if(rpcHeader.SerializeToString(&rpc_header_str))
    {
        // 序列化成功
        header_size = rpc_header_str.size();
    }
    else
    {   
        // 序列化失败
        std::cout << "serialize rpc header error!" << std::endl;
        controller->SetFailed("serialize rpc header error!");
        return;
    }


    // 组织待发送的rpc请求的字符串（左侧请求参数的序列化）
    std::string send_rpc_str;
    send_rpc_str.insert(0, std::string((char*)&header_size, 4));  // header_size
    send_rpc_str += rpc_header_str;  // rpcheader
    send_rpc_str += args_str;        // 将序列化后的请求参数args_str添加到send_rpc_str中

    // 打印调试信息
    std::cout << "++++++++++++++++++++++++++++++++++++++" << std::endl;
    std::cout << "header_size:" << header_size <<std::endl;
    std::cout << "rpc_header_str:" << rpc_header_str <<std::endl;
    std::cout << "service_name:" << service_name <<std::endl;
    std::cout << "method_name:" << method_name <<std::endl;
    std::cout << "args_str:" << args_str <<std::endl;
    std::cout << "++++++++++++++++++++++++++++++++++++++" <<std::endl;

    // 使用tcp编程,完成rpc方法调用
    int clientfd = socket(AF_INET, SOCK_STREAM, 0); // 创建一个TCP套接字clientfd,用于与RPC服务器建立    连接
    if(clientfd == -1)
    {   
        std::cout << "create socket errno! errno:" << errno << std::endl;
        char errtxt[512] = {0};
        sprintf(errtxt, "create socket errno! errno: %d", errno); // ?
        controller->SetFailed(errtxt);
        return;
    }
    
    // 读取配置文件rpcserver的信息
    // std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    // uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());
    ZkClient zkCli;  // 使用客户端对象zkCli,已被初始化并启动 
    zkCli.Start();    
    // /UserServiceRpc/Login
    std::string method_path = "/"+service_name + "/" + method_name;
    // 127.0.0.1:8000
    std::string host_data = zkCli.GetData(method_path.c_str()); // 查询zookeeper上注册的服务器地址信息,host_data 是以字符串形式表示的服务器地址和端口.
    // 解析服务器地址信息
    if(host_data == "") // 检查 host_data 是否为空，如果为空则设置错误信息并返回
    {
        controller->SetFailed(method_path + "is not exist!");
        return;
    }
    int idx = host_data.find(":"); // 查找 host_data 中的冒号位置 idx，将地址和端口进行分隔
    if(idx == -1)
    {
        controller->SetFailed(method_path + "address is invalid!");
        return ;
    }
    // 查找 host_data 中的冒号位置 idx，将地址和端口进行分隔
    std::string ip = host_data.substr(0,idx);
    uint16_t port = atoi(host_data.substr(idx+1, host_data.size()-idx).c_str());

    // 构建服务器地址结构
    // 使用提取的 IP 地址和端口，填充 server_addr 结构体的字段
    // server_addr 用于后续 connect 函数的调用，指定连接的目标服务器
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port); 
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());

    // 连接服务器,使用connect函数尝试连接Rpc服务节点,如果连接失败,关闭套接字并设置错误信息,然后返回
    if(-1 == connect(clientfd, (struct sockaddr*)&server_addr, sizeof(server_addr)))
    {
        // std::cout << "create error!errno:"<< errno << std::endl;
        close(clientfd);
        char errtxt[512] = {0};
        sprintf(errtxt, "connect error!errno:%d",errno);
        controller->SetFailed(errtxt);
        return;
    }    

    // 发送rpc请求,使用send函数将序列化后的请求数据'send_rpc_str'发送给服务器
    if(-1 == send(clientfd, send_rpc_str.c_str(), send_rpc_str.size(), 0))
    {
        // std::cout << "send error!errno:"<< errno << std::endl;
        close(clientfd);
        char errtxt[512] = {0};
        sprintf(errtxt, "send error!errno:%d", errno);
        controller->SetFailed(errtxt);
        return;
    }

    // 接收RPC响应,使用recv函数从服务器接收响应数据,存储在recv_buf中
    char recv_buf[1024] = {0};
    int recv_size = 0;
    if(-1 == (recv_size = recv(clientfd, recv_buf, 1024, 0)))
    {
        // std::cout << "recv error! error:" << std::endl;
        close(clientfd);
        char errtxt[512] = {0};
        sprintf(errtxt, "recv error!errno:%d", errno);
        controller->SetFailed(errtxt);
        return;
    }

    // 反序列化rpc调用的响应数据
    // 使用'ParseFromArray'函数将从服务器接收到的数据'recv_buf'反序列化为响应消息'response'
    // std::string response_str(recv_buf, 0, recv_size); // bug出现问题,recv_buf中遇到\0后面的数据就存不下来了,导致反序列化失败
    // if(!response->ParseFromString(response_str))
    if(!response->ParseFromArray(recv_buf, recv_size))
    {
        // std::cout << "parse error! response_str:" << recv_buf << std::endl;
        close(clientfd); 
        char errtxt[512] = {0};
        sprintf(errtxt, "parse error!errno:%s", recv_buf);
        controller->SetFailed(errtxt);
        return;
    }
}