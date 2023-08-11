#include "mprpcconfig.h"

#include <iostream>
#include <string.h>

// 负责解析加载配置文件并解析其中的配置项
void MprpcConfig::LoadConfigFile(const char* config_file)
{
    FILE *pf = fopen(config_file, "r");
    if(nullptr == pf)
    {
        std::cout << config_file << "is not exist!" << std::endl;
        exit(EXIT_FAILURE);
    }

    // 1.注释 2.正确的配置项 3.去掉开头的多余的空格 
    while(!feof(pf))
    {
        char buf[512] = {0};
        fgets(buf, 512, pf);

        // 去除字符串前多余的空格
        std::string read_buf(buf);
        Trim(read_buf);

        // 判断#的注释
        if(read_buf[0] == '#' || read_buf.empty()){
            continue;
        }

        // 解析配置项
        int idx = read_buf.find('=');
        if(idx == -1 ){
            // 配置项不合法 - 这里可以加日志
            continue;
        }

        std::string key;
        std::string value;
        key = read_buf.substr(0, idx);
        Trim(key);
        // rpcserverip=127.0.0.1\n 这里\n到底是什么意思
        int endidx = read_buf.find('\n', idx);
        value = read_buf.substr(idx+1, endidx-idx-1); // 这里减去-1后输出就没有空行了
        Trim(value); // 去除多余的空格
        m_configMap.insert({key, value}); // 将最终的键值对插入
    }
    fclose(pf);
};

// 查询配置项信息
std::string MprpcConfig::Load(const std::string &key)
{
    // return m_configMap[key];  // !不使用中括号-否则如果不存在就往map表里增加东西 
    auto it = m_configMap.find(key);
    if(it == m_configMap.end()){
        return " ";
    }
    return it->second;
}

// 去除字符串前后空格(封装好的)
void MprpcConfig::Trim(std::string &src_buf)
{  
    int idx = src_buf.find_first_not_of(' '); //找到第一个不为空字符的并返回idx
    if(idx!=-1)
    {
        // 说明字符串前面有空格
        src_buf = src_buf.substr(idx, src_buf.size()-idx);
    }

    // 去掉字符串后面的空格
    idx = src_buf.find_last_not_of(' ');
    if(idx!=-1)
    {
        // 说明字符串后面有空格
        src_buf = src_buf.substr(0, idx+1); // 起始位置 | 长度
    }
}
