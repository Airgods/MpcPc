#pragma once

#include <unordered_map>
#include <string>

// rpcserverip rpcserverport  zookeeperip zookeeperport
// 框架读取配置文件类-配置文件一般读取一次即可
class MprpcConfig
{
public:
    // 负责解析加载配置文件
    void LoadConfigFile(const char* config_file); // 指定函数的接口
    // 查询配置项信息
    std::string Load(const std::string &key);
private:
    std::unordered_map<std::string, std::string> m_configMap;
    // 去掉字符串前后空格(因为使用多次所以封装起来)
    void Trim(std::string &src_buf);  
}; 