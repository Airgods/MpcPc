 #include "zookeeperutil.h"
 #include "mprpcapplication.h"
 #include <semaphore.h>
 #include <iostream>

 // 全局的watcher观察器(监视器回调函数),用于处理与 ZooKeeper 会话相关的事件
 // 当会话状态变为 ZOO_CONNECTED_STATE 时，会通过信号量 sem 发送一个信号，
 // 以表示与 ZooKeeper 服务器的连接已经成功建立
 void global_watcher(zhandle_t *zh, int type,
                    int state, const char *path, void *watcherCtx)
 {
    if(type == ZOO_SESSION_EVENT)        // 回调的消息类型是和会话相关的消息类型
    {
        if(state == ZOO_CONNECTED_STATE) // zkclient和zkserver连接成功
        {   
            sem_t *sem = (sem_t*)zoo_get_context(zh);
            sem_post(sem);
        }
    }
 }

// 构造函数
 ZkClient::ZkClient():m_zhandle(nullptr)
 {

 }

// 析构函数->释放m_zhandle的资源,关闭与 ZooKeeper 服务器的连接
 ZkClient::~ZkClient()
 { 
    if(m_zhandle != nullptr)
    {
        zookeeper_close(m_zhandle); // 关闭句柄,释放资源 MySQL_Conn
    }
 }
 
 // 连接zkserver
 void ZkClient::Start()
 {
    std::string host = MprpcApplication::GetInstance().GetConfig().Load("zookeeperip");   // 获取zookeeper服务器的主机名 
    std::string port = MprpcApplication::GetInstance().GetConfig().Load("zookeeperport"); // 获取zookeeper服务器的端口号
    std::string connstr = host + ":" + port; // 创建字符串

    /*
    zookeeper_mt:多线程版本
    zookeeper的API客户端程序提供了三个线程
    API调用线程 
    网络I/O线程 pthread_create poll
    watcher 回调线程 zookeeper_init
    */
    // 调用 zookeeper_init 函数来建立与 ZooKeeper 服务器的连接 
    m_zhandle = zookeeper_init(connstr.c_str(),global_watcher, 30000, nullptr, nullptr, 0);
    if(nullptr == m_zhandle)  // 检查连接是否成功,如果连接句柄为nullptr,表示连接初始化失败 
    {
        std::cout << "zookeeper_init error!" << std::endl;
        exit(EXIT_FAILURE);
    }

    // 创建信号量sem,用于等待连接成功的信号
    // 将信号量的地址设置为连接句柄的上下文,
    // 这样在全局的监视器回调函数中，当会话状态变为 ZOO_CONNECTED_STATE 时，
    // 会通过信号量 sem 发送一个信号，表示连接已经成功建立。
    sem_t sem; 
    sem_init(&sem, 0, 0);
    zoo_set_context(m_zhandle, &sem);

    // 用信号量等待连接建立成功的信号。
    // 在全局的监视器回调函数中，如果会话状态变为 ZOO_CONNECTED_STATE，会触发发送信号，从而使程序继续执行
    sem_wait(&sem);
    std::cout << "zookeeper_init success!" << std::endl;
 }

// 每一个节点都存在一个path
 void ZkClient::Create(const char *path, const char *data, int datalen, int state)
 {
    char path_buffer[128];
    int bufferlen = sizeof(path_buffer);
    int flag;
    // 先判断path表示的znode节点是否存在,如果存在,就不再重复创建了
    flag = zoo_exists(m_zhandle, path, 0, nullptr);
    if(ZNONODE == flag) // 表示path的znode节点不存在
    {   
        // 创建指定的path的znode节点了
        flag == zoo_create(m_zhandle, path, data, datalen,
            &ZOO_OPEN_ACL_UNSAFE, state, path_buffer,bufferlen);
        if(flag == ZOK)
        {
            std::cout << "znode create success...path:" << path << std::endl;
        }
        else
        {
            std::cout << "flag:" << flag << std::endl;
            std::cout << "znode create error...path:" << path << std::endl;
            exit(EXIT_FAILURE);
        }
    }
 }

// 根据指定的path,获取znode节点的值
std::string ZkClient::GetData(const char *path)
{
    char buffer[64];
    int bufferlen = sizeof(buffer);
    int flag = zoo_get(m_zhandle, path, 0, buffer, &bufferlen, nullptr);
    if(flag!=ZOK)
    {
        std::cout << "get znode error...path:"<< path << std::endl;
        return "";
    }
    else
    {
        return buffer;
    }
}