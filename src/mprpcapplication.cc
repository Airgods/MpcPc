#include "mprpcapplication.h"
#include <iostream>
#include <unistd.h>
#include <string>

MprpcConfig MprpcApplication::m_config; // 类的静态成员做类外初始化,用于存储应用程序的配置信息的对象

void ShowArgsHelp()  // 在命令行参数错误时显示帮助信息
{
     std::cout << "format: command -i <configfile>" << std::endl;
}

void MprpcApplication::Init(int argc, char **argv) // 初始化Mprpc应用程序
{
    if(argc<2)
    {
        ShowArgsHelp();
        exit(EXIT_FAILURE);
    }
    
    int c = 0;
    std::string config_file;
    while((c = getopt(argc, argv, "i:")) != -1) // i是便于编译使用的
    {
        switch(c)
        {
        case 'i':
            config_file = optarg;
            break;
        case '?':
            std::cout << "invalid args!"<<std::endl;
            ShowArgsHelp();
            exit(EXIT_FAILURE);
        case ':':
            std::cout << "need <configfile>!"<<std::endl;
            ShowArgsHelp();
            exit(EXIT_FAILURE);
        default:
            break;
        }
    }
    // 关于这一块的逻辑需要梳理下
    // 开始加载配置文件 rpcserver_ip = \ rpcserver_port = \ zookeeper_ip = \zookeeper_port =   
    // 有配置文件后可以通过配置文件读取相应的设置
    m_config.LoadConfigFile(config_file.c_str());
    
    std::cout << "rpcserverip: "<< m_config.Load("rpcserverip") << std::endl;
    std::cout << "rpcserverport: "<< m_config.Load("rpcserverport") << std::endl;
    std::cout << "zookeeperip: "<< m_config.Load("zookeeperip") << std::endl;
    std::cout << "zookeeperport: "<< m_config.Load("zookeeperport") << std::endl;
}    

MprpcApplication& MprpcApplication::GetInstance() // 实现单例模式?
{
        static MprpcApplication app;
        return app;
}

// 返回存储应用程序配置的‘m_config’对象,以便其他部分可以访问和使用配置信息
MprpcConfig& MprpcApplication::GetConfig()
{
    return m_config;
}