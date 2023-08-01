#include "test.h"

void epoll_test::tcp_test(bool is_server)
{
    if(is_server)
    {
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr("192.168.2.100");
        addr.sin_port = ntohs(3000);

        int tcpsocket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
        if(bind(tcpsocket,reinterpret_cast<sockaddr *>(&addr),sizeof(addr)) == -1)
        {
            std::cout<<"bind failed!"<<std::endl;
        }

        std::vector<int> conn_vec;

        int epoll_conn_fd = epoll_create(100);
        auto recv_func = [&epoll_conn_fd]()->void{
        struct epoll_event ep_events[100];
            while(true)
            {
                int nfds = epoll_wait(epoll_conn_fd,ep_events,100,-1);
                if(nfds != -1)
                {
                    for(int i = 0;i < nfds;++i)
                    {
                        char temp[10];
                        while(true)
                        {
                            int len = recv(ep_events[i].data.fd,temp,10,0);
                            if(len > 0)
                            {
                                std::cout<<std::string(temp)<<std::endl;
                            }
                            else if(len == 0)
                            {
                                std::cout<<"recv len == 0"<<std::endl;
                                break;
                            }
                            else
                            {
                                std::cout<<"recv error!"<<std::endl;
                                break;
                            }
                        }
                    }
                }
            }
            return;
        };

        auto fu = std::async(std::launch::async,recv_func);

        struct epoll_event ep_event,ep_events[100];
        ep_event.data.fd = tcpsocket;
        ep_event.events = EPOLLIN | EPOLLET;

        int epoll_fd = epoll_create(100);
        epoll_ctl(epoll_fd,EPOLL_CTL_ADD,tcpsocket,&ep_event);

        listen(tcpsocket,10);

        while(true)
        {
            int nfds = epoll_wait(epoll_fd,ep_events,100,-1);
            if(nfds != -1)
            {
                for(int i = 0;i < nfds;++i)
                {
                    if(ep_events[i].data.fd == tcpsocket && ep_events[i].events == EPOLLIN)
                    {
                        struct sockaddr_in sock_addr_temp;
                        socklen_t sock_len = sizeof(sockaddr_in);
                        int conn_fd = accept(tcpsocket,reinterpret_cast<sockaddr*>(&sock_addr_temp),&sock_len);
                        if(conn_fd != -1)
                        {
                            std::cout<<"accept connection!"<<std::endl;
                            std::cout<<"remote ip:"<<inet_ntoa(sock_addr_temp.sin_addr)<<std::endl;
                            std::cout<<"remote port:"<<htons(sock_addr_temp.sin_port)<<std::endl;

                            struct epoll_event ep_event_temp;
                            ep_event_temp.data.fd = conn_fd;
                            ep_event_temp.events = EPOLLIN | EPOLLET;

                            epoll_ctl(epoll_conn_fd,EPOLL_CTL_ADD,conn_fd,&ep_event_temp);
                        }
                    }
                }
            }
        }
        fu.get();
    }
    else
    {
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr("192.168.2.100");
        addr.sin_port = ntohs(3001);

        struct sockaddr_in remote_addr;
        remote_addr.sin_family = AF_INET;
        remote_addr.sin_addr.s_addr = inet_addr("192.168.2.100");
        remote_addr.sin_port = ntohs(3000);

        int tcpsocket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
        if(bind(tcpsocket,reinterpret_cast<sockaddr *>(&addr),sizeof(addr)) == -1)
        {
            std::cout<<"bind failed!"<<std::endl;
        }
        
        if(connect(tcpsocket,reinterpret_cast<sockaddr*>(&remote_addr),sizeof(sockaddr)) == 0)
        {
            std::string temp("012345678901234567890123456789");
            while(true)
            {
                if(send(tcpsocket,temp.c_str(),temp.size(),0) != -1)
                {
                    std::cout<<"send successful!"<<std::endl;
                }
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }
    }
}

void epoll_test::udp_test(bool is_server)
{
    if(is_server)
    {
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr("192.168.2.100");
        addr.sin_port = ntohs(2000);

        int udpsocket = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
        if(bind(udpsocket,reinterpret_cast<sockaddr *>(&addr),sizeof(addr)) == -1)
        {
            std::cout<<"bind failed!"<<std::endl;
            return;
        }

        struct epoll_event ep_event,ep_events[100];
        ep_event.events = EPOLLIN;
        ep_event.data.fd = udpsocket;

        int epoll_fd = epoll_create(100);
        epoll_ctl(epoll_fd,EPOLL_CTL_ADD,udpsocket,&ep_event);

        while(true)
        {
            int nfds = epoll_wait(epoll_fd,ep_events,100,-1);
            if( nfds!= -1)
            {
                for(int i = 0;i < nfds;++nfds)
                {
                    char buffer[100];
                    sockaddr_in client_addr;
                    socklen_t client_addr_len;
                    if(ep_events[i].events == EPOLLIN && ep_events[i].data.fd == udpsocket)
                    {
                        recvfrom(ep_events[i].data.fd,buffer,100,0,reinterpret_cast<sockaddr*>(&client_addr),&client_addr_len);
                        std::cout<<buffer<<std::endl;
                    }
                }
            }
            else
            {
                continue;
            }
        }
    }
    else
    {
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr("192.168.2.100");
        addr.sin_port = ntohs(2000);

        int udpsocket = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);

        struct epoll_event ep_event,ep_events[100];
        ep_event.data.fd = udpsocket;
        ep_event.events = EPOLLOUT;

        int epoll_fd = epoll_create(100);
        epoll_ctl(epoll_fd,EPOLL_CTL_ADD,udpsocket,&ep_event);
        char buffer[100];
        memset(buffer,100,0);

        while(true)
        {
            int nfds = epoll_wait(epoll_fd,ep_events,100,-1);
            if(nfds != -1)
            {
                for(int i = 0;i < nfds;++i)
                {
                    if(ep_events[i].data.fd == udpsocket && ep_events[i].events == EPOLLOUT)
                    {
                        std::string temp = "123456";
                        sendto(udpsocket,temp.c_str(),100,0,reinterpret_cast<sockaddr*>(&addr),sizeof(sockaddr));
                        std::this_thread::sleep_for(std::chrono::seconds(1));
                    }
                }
            }
        }
    }
}