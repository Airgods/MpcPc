# aux_source_directory(. SRC_LIST)        # 当前目录下所有源文件放入SRC_LIST ->库链接错误是因为开始时makefile中并不包含之后编译的内容,但是这种编写方式只要CMakelists不做改变,那么其不会添加新内容
set(SRC_LIST 
    mprpcapplication.cc 
    mprpcconfig.cc 
    rpcheader.pb.cc 
    rpcprovider.cc 
    mprpcchannel.cc
    mprpccontroller.cc
    logger.cc
    zookeeperutil.cc
    ) # 仅针对源文件下目录小的情况
add_library(mprpc ${SRC_LIST})          # 生成动态库 为什么动态库不行?
# add_library(mprpc SHARED ${SRC_LIST}) # 生成静态库
target_link_libraries(mprpc muduo_net muduo_base pthread zookeeper_mt)     # 生成框架时依赖于这种muduo网络库,mt是多线程版本