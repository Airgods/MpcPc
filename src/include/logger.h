#pragma once
#include "lockqueue.h"
#include <string.h>
 
// 定义日志的两个级别
enum Loglevel
{
    INFO,   // 普通信息
    ERROR,  // 错误信息
};

// Mprpc框架提供的日志系统-单例模式
class Logger
{
public:
    // 获取日志的单例,静态方法,确保只有一个日志实例
    static Logger& GetInstance(); 
    // 设置日志级别 
    void SetLogLevel(Loglevel level);
    // 写日志,将日志信息添加到日志缓冲队列中
    void Log(std::string msg);
private:
    int m_loglevel;                     // 记录日志级别
    LockQueue<std::string> m_lckQueue;  // 日志缓冲队列 

    // 这里设计为单例模式,但是单例模式该怎么设置
    Logger();
    Logger(const Logger&) = delete;     // 拷贝构造-利用拷贝构造生成新对象
    Logger(Logger&&) = delete;          // 移动构造
};

// 定义宏 LOG_INFO("xxx %d %s",20,"xxxx")
// do{1.获取logger类的单例实例,以便后续调用日志记录功能 2.创建数组,用于存储格式化后的日志消息
//    3.将格式化后的日志消息保存在数组c中 4.将格式化后的日志消息添加到日志缓冲队列中}
#define LOG_INFO(logmsgformat, ...) \
    do \ 
    {   \
        Logger &logger = Logger::GetInstance(); \  
        logger.SetLogLevel(INFO); \
        char c[1024] = {0};       \
        snprintf(c, 1024, logmsgformat, ##__VA_ARGS__);      \
        logger.Log(c); \
    } while(0);

#define LOG_ERR(logmsgformat, ...) \
    do \ 
    {   \
        Logger &logger = Logger::GetInstance(); \
        logger.SetLogLevel(ERROR); \
        char c[1024] = {0};       \
        snprintf(c, 1024, logmsgformat, ##__VA_ARGS__);      \
        logger.Log(c); \
    } while(0);