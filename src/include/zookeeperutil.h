#pragma once

#include <semaphore.h>
#include <zookeeper/zookeeper.h>
#include <string>

class ZkClient
{
public:
    ZkClient();
    ~ZkClient();
    // zkclient启动连接zkserver
    void Start();
    // 在zkserver上根据指定的path创建znode节点
    void Create(const char* path, const char *data, int datalen, int state=0);
    // 根据参数指定的znode节点路径,获取znode节点的值
    // 用于获取指定节点路径的节点数据,它会返回一个字符串，表示该节点的数据
    std::string GetData(const char *path);
private:
    // zk的客户端句柄
    zhandle_t* m_zhandle;
};