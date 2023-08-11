# 定义了一个名为 provider 的可执行文件，
# 并将 userservice.cc 和 ../user.pb.cc 这两个源文件添加到构建中
# set(SRC_LIST userservice.cc ../user.pb.cc)
set(SRC_LIST friendservice.cc ../friend.pb.cc)
add_executable(provider ${SRC_LIST})
target_link_libraries(provider mprpc protobuf) # 链接我们的动态库 生成的provider依赖于mprpc、protobuf
