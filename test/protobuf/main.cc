#include "test.pb.h"
#include <iostream>
#include <string>
using namespace fixbug; //项目写的少是因为会操作冲突

// 如何处理protobuf上列表代码的编写
int main()
{
    // LoginResponse rsp;
    // ResultCode* rc = rsp.mutable_result();
    // rc->set_errcode(1);
    // rc->set_errmsg("登录处理失败了");

    GetFriendListResponse rsp;
    ResultCode* rc = rsp.mutable_result();
    rc->set_errcode(0);  // 无错误不需要设置message

    User *user1 =  rsp.add_friend_list();
    user1->set_name("zhang san");
    user1->set_age(20);
    user1->set_sex(User::MAN);

    User *user2 =  rsp.add_friend_list();
    user2->set_name("zhang san");
    user2->set_age(20);
    user2->set_sex(User::MAN);

    std::cout<<rsp.friend_list_size() <<std::endl;

    return 0;
}


int main1()
{   
    // 封装login请求对象的数据
    LoginRequest req;
    req.set_name ("zhang san\n");
    req.set_pwd ("123456");

    // 对象数据序列化->char*
    std::string send_str;
    if(req.SerializeToString(&send_str)){
        std::cout << send_str.c_str() << std::endl;
    }

    // 从send_str反序列化一个login请求对象
    LoginRequest reqB;
    if(reqB.ParseFromString(send_str))
    {   
        std::cout<<reqB.name()<<std::endl;
        std::cout<<reqB.pwd()<<std::endl;
    }
    return 0;
}