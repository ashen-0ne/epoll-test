# ifndef TEST_H
# define TEST_H

#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <string.h>
#include <thread>

void epoll_test(bool is_server)
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
    
};

#endif