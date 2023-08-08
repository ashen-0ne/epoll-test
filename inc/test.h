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
#include <fstream>
#include "shmbuffer.h"

class epoll_test
{
public:
    void udp_test(bool is_server);
    void tcp_test(bool is_server);
    void file_transport_test(bool is_server);
    void shm_test(bool is_server);
};

#endif

