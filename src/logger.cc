#include "logger.h"
#include <iostream>
#include <time.h>

// 获取日志的单例,使用静态局部变量的方式实现单例模式,确保在程序的整个生命周期内只存在一个 Logger 实例
Logger& Logger::GetInstance()
{
    static Logger logger;
    return logger;
}

Logger::Logger()
{
    // 启动专门的写日志线程,目的在于不断地从日志队列中取出日志消息，然后将其写入对应的日志文件
    std::thread writeLogTask([&](){
        for(;;)
        {
            // 获取当前的日期,然后取日志信息,写入相应的日志文件当中
            time_t now = time(nullptr);   // 按s算,1970到现在的秒数
            tm *nowtm = localtime(&now);  // 将时间戳转换为tm结构体

            // 构建当前日期对应的日志文件名:使用 sprintf 函数将当前日期信息格式化为文件名
            char file_name[128];
            sprintf(file_name, "%d-%d-%d-log.txt", nowtm->tm_year+1900, nowtm->tm_mon+1, nowtm->tm_mday);

            // 打开文件,使用追加模式，以便将日志信息添加到文件末尾
            FILE *pf = fopen(file_name, "a+");
            if(pf == nullptr)
            {
                std::cout << "logger file: "<< file_name <<"open error!" << std::endl;
                exit(EXIT_FAILURE);
            }

            // 从日志队列中弹出一个日志消息
            std::string msg = m_lckQueue.Pop();
            
            // 格式化当前时间和日志级别,将其与日志消息关联起来,然后将整合后的消息添加到日志文件
            char time_buf[128] = {0};
            sprintf(time_buf, "%d:%d:%d=>[%s]",
                             nowtm->tm_hour, 
                             nowtm->tm_min, 
                             nowtm->tm_sec,
                             (m_loglevel == INFO ? "info": "error")); // 关联上日志的级别
            msg.insert(0, time_buf);
            msg.append("\n");

            fputs(msg.c_str(),pf);  // 将格式化后的消息写入日志文件
            fclose(pf);             // 关闭文件
        }
    });
    // 设置分离线程,守护线程

    writeLogTask.detach();          // 分离日志线程,使其成为守护线程,不会阻塞主线程的退出
}

// 设置日志级别
void Logger::SetLogLevel(Loglevel level)
{
    m_loglevel = level;
}
// 写日志,把日志信息写入lockqueue缓冲区当中
void Logger::Log(std::string msg)
{   
    m_lckQueue.Push(msg);
}   
