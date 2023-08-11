#include "rpcprovider.h"
#include "mprpcapplication.h"
#include "rpcheader.pb.h"  
#include "logger.h" 
#include "zookeeperutil.h"  

/*
service_name => service描述
                    => service* 记录服务对象
                    => method_name => method方法对象
json(基于文本存储)  protobuf(相比json有些成本,但是更好用-基于二进制存储)
*/
// 这里是框架提供给外部使用的,可以发布rpc方法的函数接口 提供了一种基于react的非常不错的网络服务
// 注册一个需要发布的RPC服务,它获取服务的描述信息和方法描述,并将它们存储在m_serviceMap中,以备后续网络调用
void RpcProvider::NotifyService(google::protobuf::Service *service)
{
    ServiceInfo service_info;

    // 获取服务对象的描述信息 | 这里为什么用const?
    const google::protobuf::ServiceDescriptor *pserviceDesc = service->GetDescriptor();
    // 获取服务的名字
    std::string service_name = pserviceDesc->name();
    // 获取服务对象service的方法的数量
    int methodCnt = pserviceDesc->method_count(); // 暂时只有1个,即为Login方法

    // std::cout << "service_name:" << service_name << std::endl;
    LOG_INFO("service_name:%s", service_name.c_str());
    
    for(int i = 0; i<methodCnt; ++i){
        // 获取了服务对象指定下标的服务方法的描述(抽象的描述) UserService Login
        // 在框架层调用它来指向应用层的业务
        const google::protobuf::MethodDescriptor* pmethodDesc =  pserviceDesc->method(i);
        std::string method_name = pmethodDesc->name();
        service_info.m_methodMap.insert({method_name, pmethodDesc});
        // std::cout << "method_name:" <<service_name << std::endl;
        LOG_INFO("method_name:%s", method_name.c_str());
    } 
    service_info.m_service = service;
    m_serviceMap.insert({service_name, service_info});

}

// 启动rpc服务节点,开始提供rpc远程网络调用服务
// 创建一个muduo::net::TcpServer对象,并设置连接回调和消息读写回调,最后启动网络服务
// 将RPC服务节点启动，并且通过 muduo 网络库来处理连接和消息的收发，同时将服务注册到 ZooKeeper 中以便客户端能够发现并调用该服务
void RpcProvider::Run()
{
    std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");  // 从配置中心加载RPC服务节点的IP地址 
    uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str()); // 从配置中心加载RPC服务节点的端口号，并将字符串转换为整数
    muduo::net::InetAddress address(ip, port); // 创建一个 muduo::net::InetAddress 对象，用来表示RPC服务节点的地址（IP地址和端口号）

    // 创建TcpSerer对象->创建一个 muduo::net::TcpServer 对象，将之前创建的 m_eventLoop 事件循环和服务地址传递给它，同时给它一个名称 "RpcProvider"
    muduo::net::TcpServer server(&m_eventLoop, address, "RpcProvider");
    
    // 绑定连接回调和消息读写回调方法 | muduo库分离网络代码和业务代码
    server.setConnectionCallback(std::bind(&RpcProvider::OnConnection, this, std::placeholders::_1));
    // placeholder代表预留的参数
    server.setMessageCallback(std::bind(&RpcProvider::OnMessage, this, std::placeholders::_1,
                            std::placeholders::_2, std::placeholders::_3));

    // 设置muduo库的线程数量 1个IO 3个工作线程 类似基于react模型的服务器 epoll+多线程
    server.setThreadNum(4);

    // 把当前rpc节点上要发布的服务全部注册到zk上面,让rpc client可以从zk上发现服务
    // session timeout 30s zkclient 网络I/O线程 1/3*timeout 时间发送ping消息
    ZkClient zkCli;
    zkCli.Start();
    // service_name为永久性节点 method为临时性节点
    for(auto &sp:m_serviceMap)
    {
        // /service_name /UserServiceRpc
        std::string service_path="/"+sp.first;
        zkCli.Create(service_path.c_str(), nullptr,0);
        for(auto &mp: sp.second.m_methodMap)
        {
            // /service_name/method_name /UserServiceRpc/Login 存储当前这个rpc服务节点主机的ip和port
            std::string method_path = service_path + "/" + mp.first;
            char method_path_data[128]={0};
            sprintf(method_path_data, "%s:%d", ip.c_str(),port);
            // ZOO_EPHEMERAL表示znode是一个临时性节点
            zkCli.Create(method_path.c_str(), method_path_data, strlen(method_path_data), ZOO_EPHEMERAL);
        }
    }

    // rpc服务端准备启动,打印信息
    std::cout<<"RpcProvider start service at ip:" << ip << "port" << port <<std::endl;

    // 启动网络服务
    server.start();     // 监听指定地址和端口号
    m_eventLoop.loop(); // 调用m_eventLoop.loop()进入事件循环,等待处理连接和消息的到来
}

// 新的socket连接回调-moduo负责调用,建立连接时被调用
void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr &conn)
{
    if(!conn->connected())
    {
        // 和rpc client的连接断开了
        conn->shutdown();
    }
}

/*
在框架内部,RpcProvider和RpcConsumer协商好之间通信用的protobuf数据类型
service_name method_name args 定义proto的message类型,进行数据的序列化和反序列化
                                    service_name method_name args_size
header_size(4个字节) + header_str + arg_str
10 "10"
100 "100"
std::string insert和copy方法
*/
// 已建立连接用户的读写事件回调 如果远程有一个rpc服务的调用请求,那么OnMessage方法就会响应-moduo负责调用
void RpcProvider::OnMessage(const muduo::net::TcpConnectionPtr& conn, 
                                muduo::net::Buffer* buffer,
                                muduo::Timestamp)
{   
    // 从网络上接收的远程rpc调用请求的字符流,以字符串形式存储在'recv_buf'中
    std::string recv_buf = buffer->retrieveAllAsString(); 

    // 从字符流中读取前4个字节的内容,数据头大小信息,用于解析后续的数据
    uint32_t header_size = 0;
    recv_buf.copy((char*)&header_size, 4, 0);

    // 根据header_size读取数据头的原始字符流,反序列化数据,得到rpc请求的详细信息
    std::string rpc_header_str = recv_buf.substr(4, header_size);
    mprpc::RpcHeader rpcHeader;
    std::string service_name;
    std::string method_name;
    uint32_t args_size;
    if(rpcHeader.ParseFromString(rpc_header_str))
    {
        // 数据头反序列化成功 
        service_name = rpcHeader.service_name(); 
        method_name = rpcHeader.method_name();
        args_size = rpcHeader.args_size();
    }
    else
    {
        // 数据头反序列化失败
        std::cout << "rpc_header_str:" << rpc_header_str << "parse error!" << std::endl;
        return;
    }

    // 获取rpc方法参数的字符流数据
    std::string args_str = recv_buf.substr(4 + header_size, args_size);

    // 打印调试信息,显示收到的数据头和信息
    std::cout << "===================================" << std::endl;
    std::cout << "header_size:" << header_size << std::endl;
    std::cout << "rpc_header_str:" << rpc_header_str << std::endl;
    std::cout << "service_name:" << service_name << std::endl;
    std::cout << "method_name:" << method_name << std::endl;
    std::cout << "args_str:" << args_str << std::endl;
    std::cout << "===================================" << std::endl;

    // 获取service对象和method对象
    auto it = m_serviceMap.find(service_name);
    if(it == m_serviceMap.end())
    {
        std::cout << service_name << "is not exist " << std::endl;
        return; 
    }

    auto mit = it->second.m_methodMap.find(method_name);
    if(mit == it->second.m_methodMap.end())
    {
        std::cout << service_name << ":" <<method_name << "is not exist!" << std::endl;
        return; 
    }

    google::protobuf::Service *service = it->second.m_service;      // 获取service对象  new UserService
    const google::protobuf::MethodDescriptor *method = mit->second; // 获取method对象 Login

    // 生成rpc方法调用的请求request和响应response参数
    // 从框架层面中较为抽象类型中获得请求类型和响应类型
    google::protobuf::Message *request = service->GetRequestPrototype(method).New();
    if(!request->ParseFromString(args_str))
    {
        std::cout << "request parse error, content:" << args_str << std::endl;
        return;
    }
    google::protobuf::Message *response = service->GetResponsePrototype(method).New(); 

    // 给下面的mthod方法的调用,绑定一个Closure的回调函数
    // 希望这里给的回调函数就是一个成员方法->类似于产生一个对象(绑定器该怎么理解?)
    google::protobuf::Closure *done = google::protobuf::NewCallback<RpcProvider, 
                                                        const muduo::net::TcpConnectionPtr&, 
                                                        google::protobuf::Message*>
                                                        (this, 
                                                        &RpcProvider::SendRpcResponse,
                                                        conn, response);

    // 在框架上根据远端rpc请求,调用当前rpc节点上发布的方法
    // 等价于:new UserService().Login(controller, request, response, done)
    service->CallMethod(method, nullptr, request, response, done);
}

// Closure的回调操作,用于序列化rpc的响应和网络发送
// 设计如何进行序列化->将RPC方法调用的响应结果进行序列化并通过网络发送给RPC调用方的操作
void RpcProvider::SendRpcResponse(const::muduo::net::TcpConnectionPtr& conn, google::protobuf::Message* response)
{ 
    std::string response_str; // 序列化后的内容存储在response_str中
    if(response->SerializeToString(&response_str)) // response进行序列化 | response响应对象的序列化
    {
        // 序列化成功后,通过网络把rpc方法执行的结果发送给rpc的调用方
        conn->send(response_str);
    }
    else
    {
        std::cout << "serialize response_str error!" << std::endl;
    }
    conn->shutdown(); // 模拟http的短连接服务,由rpcprovider主动断开连接
}




