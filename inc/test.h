# ifndef TEST_H
# define TEST_H

#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void epoll_test()
{
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("192.168.2.100");
    addr.sin_port = ntohs(2000);

    int udpsocket = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    struct epoll_event ep_event;
    ep_event.events = EPOLLIN | EPOLLET;

    int epoll_fd = epoll_create(100);
    epoll_ctl(epoll_fd,EPOLL_CTL_ADD,udpsocket,&ep_event);
}



#endif