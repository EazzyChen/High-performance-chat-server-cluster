This is an advanced C++ project that implements a high-performance chat server cluster based on the muduo network library, MySQL database, Nginx proxy server, and Redis message queue.

# Running Instructions

## 1. Environment Setup

1. C++ Version: C++11 or higher

2. Dependent Development Libraries (Ubuntu System): libboost-dev, muduo, libmysqlclient-dev, hiredis. For other operating systems, install the corresponding libraries.

3. Dependent Services: MySQL, Nginx, Redis

## 2. Configuration Modification

1. Modify the Nginx port number in `src/client/main.cpp` to the actual port number of your Nginx server.

2. Add the `stream` block to `nginx.conf` and update it to the actual port of your ChatServer:

   ```
   ```

   

   ​    `stream{
     upstream MyServer {
   ​    server 127.0.0.1:6000 weight=1 max_fails=3 fail_timeout=30s;
   ​    server 127.0.0.1:6002 weight=1 max_fails=3 fail_timeout=30s;
     }
     server {
   ​    proxy_connect_timeout 3s;
   ​    #proxy_timeout 3s;
   ​    listen 8000;
   ​    proxy_pass MyServer;
   ​    tcp_nodelay on;
     }
     }`

3. Modify the MySQL configuration information in`src/db/db.cpp`.

## 3. Compilation and Execution

```bash
# Run the following commands in the chat directory to generate executable files ChatClient and ChatServer in the bin directory
mkdir build
cd build
cmake ..
make
```

Note: Ensure the context remains fluent after modifications.



这是一款C++进阶项目，实现基于muduo网络库，mysql数据库，nginx代理服务器，redis消息队列的高性能聊天服务器集群。

# 运行方式：

## 1. 环境搭建：

1. C++版本：11及以上

2. 依赖开发库(Ubuntu系统)：libboost-dev, muduo，libmysqlclient-dev，hiredis。 其他系统需要安装对应的库

3. 依赖服务：mysql，nginx，redis

## 2. 配置修改

1. 修改src/client/main.cpp中的nginx端口号为你的nginx服务器实际端口号
2. nginx.conf中添加stream，改为你的实际的ChatServer服务器端口

```
stream{
      upstream MyServer {
        server 127.0.0.1:6000 weight=1 max_fails=3 fail_timeout=30s;
        server 127.0.0.1:6002 weight=1 max_fails=3 fail_timeout=30s;
      }
      server {
        proxy_connect_timeout 3s;
        #proxy_timeout 3s;
        listen 8000;
        proxy_pass MyServer;
        tcp_nodelay on;
      }
  }
```

3. 修改src/db/db.cpp中的mysql配置信息

## 3. 编译运行

```bash
# 在chat目录下输入下面的命令,生成bin目录下的可执行文件ChatClient和ChatServer
mkdir build
cd build
cmake ..
make
```



