# ifndef TEST_H
# define TEST_H

#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <string.h>
#include <thread>
#include <vector>
#include <future>
#include <fcntl.h>

class epoll_test
{
public:
    void udp_test(bool is_server);
    void tcp_test(bool is_server);
};

#endif

