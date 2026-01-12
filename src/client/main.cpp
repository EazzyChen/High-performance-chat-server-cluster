#include "chatclient.hpp"
void showmenu()
{
    int i = 1;
    std::cout << "\n===== 聊天客户端命令菜单 =====" << std::endl;
    
    std::cout << i++ << ". login [用户ID] [密码]" << std::endl;
    std::cout << i++ << ". register [用户名] [密码]" << std::endl;
    std::cout << i++ << ". addfriend [好友ID]" << std::endl;
    std::cout << i++ << ". getfriends" << std::endl;
    std::cout << i++ << ". onechat [对方ID] [消息内容]" << std::endl;
    std::cout << i++ << ". creategroup [群名] [群介绍]" << std::endl;
    std::cout << i++ << ". joingroup [群ID]" << std::endl;
    std::cout << i++ << ". getgrouplist" << std::endl;
    std::cout << i++ << ". getgroupuser [群ID]" << std::endl;
    std::cout << i++ << ". groupchat [群ID] [群消息内容]" << std::endl;
    std::cout << i++ << ". exit  -退出客户端" << std::endl;
    std::cout << i++ << ". help  -查看菜单" << std::endl;

    std::cout << "===============================" << std::endl;
    
}
int main()
{
    EventLoop loop;
    InetAddress serverAddr("127.0.0.1", 8000); // Nginx 服务器地址和端口
    ChatClient client(&loop, serverAddr);

    client.connect();

    // 启动线程处理控制台输入
    std::thread inputThread([&]()
                            {
        showmenu();
        std::cout << "请输入指令: " << std::endl;
        std::string cmd;
        while (true)
        {

            
            
            std::getline(std::cin, cmd);
            std::istringstream iss(cmd);
            std::string op;
            iss >> op;

            if (op == "login")
            {
                int id;
                std::string pwd;
                iss >> id >> pwd;
                client.login(id, pwd);
            }
            else if (op == "register")
            {
                std::string name, pwd;
                iss >> name >> pwd;
                client.registerUser(name, pwd);
            }
            else if (op == "onechat")
            {
                int fromid, toid;
                fromid = client.getMyid();
                std::string msg;
                iss >> toid;
                std::getline(iss >> std::ws, msg);
                client.oneChat(fromid, toid, msg);
            }
            else if (op == "addfriend")
            {
                int userid, friendid;
                userid = client.getMyid();
                iss >> friendid;
                client.addFriend(userid, friendid);
            }
            else if (op == "creategroup")
            {
                int userid = client.getMyid();
                std::string groupname, groupdesc;
                iss >> groupname;
                std::getline(iss >> std::ws, groupdesc);
                client.createGroup(userid, groupname, groupdesc);
            }
            else if (op == "joingroup")
            {
                int userid, groupid;
                userid = client.getMyid();
                iss >> groupid;
                client.joinGroup(userid, groupid);
            }
            else if (op == "getgrouplist")
            {
                int userid = client.getMyid();
                client.getGroupList(userid);
            }
            else if (op == "getgroupuser")
            {
                int groupid;
                iss >> groupid;
                client.getGroupUserInfo(groupid);
            }
            else if (op == "groupchat")
            {
                int fromid, groupid;
                std::string msg;
                fromid = client.getMyid();
                iss >> groupid;
                std::getline(iss >> std::ws, msg);
                client.groupChat(fromid, groupid, msg);
            }
            else if(op == "getfriends")
            {
                int id = client.getMyid();
                client.getfriends(id);
            }
            else if (op == "exit")
            {
                client.disconnect();
                std::cout << "客户端已退出..." << std::endl;
                break;
            }
            else if (op == "help"){
                showmenu();
            }
            else
            {
                std::cerr << "输入的指令无效，请重新输入！" << std::endl;
            }
            std::cout << "请输入指令: " << std::endl;
        } });

    loop.loop();
    inputThread.join();
    return 0;
}