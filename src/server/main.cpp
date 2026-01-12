#include "chatserver.hpp"
#include <iostream>
#include <csignal>

using std::cin;
using std::cout;
using std::endl;

EventLoop* g_loop = nullptr;

// Signal callback: catch SIGINT/SIGTERM
void quitHandler(int sig)
{
    g_loop->quit();
}

int main()
{
    EventLoop loop;
    g_loop = &loop;

    // Register exit signals
    signal(SIGINT, quitHandler);
    signal(SIGTERM, quitHandler);
    int port = 0;
    cout << "输入运行的端口号: ";
    cin >> port;
    InetAddress addr("127.0.0.1", port);
    ChatServer server(&loop, addr, "ChatServer");

    cout << "server starting..." << endl;
    cout << "server address: " << addr.toIpPort() << endl;
    server.start();
    loop.loop();

    return 0;
}
