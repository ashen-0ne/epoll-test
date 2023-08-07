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

        if(tcpsocket == -1)
        {
            std::cerr<<"tcpsocket create failed!"<<std::endl;
            return;
        }

        if(bind(tcpsocket,reinterpret_cast<sockaddr *>(&addr),sizeof(addr)) == -1)
        {
            std::cerr<<"bind failed!"<<std::endl;
            return;
        }

        int epoll_conn_fd = epoll_create(100);

        if(epoll_conn_fd == -1)
        {
            std::cerr<<"epoll_create failed!"<<std::endl;
            return;
        }

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
                                if(epoll_ctl(epoll_conn_fd,EPOLL_CTL_DEL,ep_events[i].data.fd,nullptr) != -1)
                                {
                                    std::cout<<"remote leave connection successful!"<<std::endl;
                                }
                                break;
                            }
                            else
                            {
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
        ep_event.events = EPOLLIN;

        int epoll_fd = epoll_create(100);
        if(epoll_fd == -1)
        {
            std::cerr<<"epoll_create failed!"<<std::endl;
            return;
        }
        
        if(epoll_ctl(epoll_fd,EPOLL_CTL_ADD,tcpsocket,&ep_event) == -1)
        {
            std::cerr<<"epoll_ctl failed!"<<std::endl;
            return;
        }

        if(listen(tcpsocket,10) == -1)
        {
            std::cerr<<"listen failed!"<<std::endl;
            return;
        }

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
                        int flag, old_flag;
                        old_flag = flag = fcntl(conn_fd, F_GETFL, 0);
                        flag |= O_NONBLOCK;
                        fcntl(conn_fd, F_SETFL, flag);
                        if(conn_fd != -1)
                        {
                            std::cout<<"accept connection!"<<std::endl;
                            std::cout<<"remote ip:"<<inet_ntoa(sock_addr_temp.sin_addr)<<std::endl;
                            std::cout<<"remote port:"<<htons(sock_addr_temp.sin_port)<<std::endl;

                            struct epoll_event ep_event_temp;
                            ep_event_temp.data.fd = conn_fd;
                            ep_event_temp.events = EPOLLIN | EPOLLET;

                            if(epoll_ctl(epoll_conn_fd,EPOLL_CTL_ADD,conn_fd,&ep_event_temp) == -1)
                            {
                                std::cout<<"epoll_ctl add failed!"<<std::endl;
                            }
                            else
                            {
                                std::cout<<"epoll_ctl add successful!"<<std::endl;
                            }
                        }
                    }
                }
            }
        }
        fu.get();
    }
    else
    {
        struct sockaddr_in remote_addr;
        remote_addr.sin_family = AF_INET;
        remote_addr.sin_addr.s_addr = inet_addr("192.168.2.100");
        remote_addr.sin_port = ntohs(3000);

        int tcpsocket1 = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
        int tcpsocket2 = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
        
        if(connect(tcpsocket1,reinterpret_cast<sockaddr*>(&remote_addr),sizeof(sockaddr)) == -1)
        {
            std::cout<<"tcpsocket1 connect failed!"<<std::endl;
            return;
        }
        if(connect(tcpsocket2,reinterpret_cast<sockaddr*>(&remote_addr),sizeof(sockaddr)) == -1)
        {
            std::cout<<"tcpsocket1 connect failed!"<<std::endl;
            return;
        }
        {
            std::string temp1("012345678901234567890123456789");
            std::string temp2("abcdefghijabcdefghijabcdefghij");

            // for(int i = 0;i < 5;++i)
            while(true)
            {
                if(send(tcpsocket1,temp1.c_str(),temp1.size(),0) != -1)
                {
                    std::cout<<"tcpsocket1 send successful!"<<std::endl;
                }
                if(send(tcpsocket2,temp2.c_str(),temp2.size(),0) != -1)
                {
                    std::cout<<"tcpsocket2 send successful!"<<std::endl;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }

            // shutdown(tcpsocket1,SHUT_RDWR);

            // for(int i = 0;i < 10;++i)
            // {
            //     if(send(tcpsocket2,temp2.c_str(),temp2.size(),0) != -1)
            //     {
            //         std::cout<<"tcpsocket2 send successful!"<<std::endl;
            //     }
            //     std::this_thread::sleep_for(std::chrono::seconds(1));
            // }

            // shutdown(tcpsocket2,SHUT_RDWR);
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

void epoll_test::file_transport_test(bool is_server)
{
    if(is_server)
    {
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr("192.168.2.100");
        addr.sin_port = ntohs(3000);

        int tcpsocket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

        if(tcpsocket == -1)
        {
            std::cerr<<"tcpsocket create failed!"<<std::endl;
            return;
        }

        if(bind(tcpsocket,reinterpret_cast<sockaddr *>(&addr),sizeof(addr)) == -1)
        {
            std::cerr<<"bind failed!"<<std::endl;
            return;
        }

        int epoll_conn_fd = epoll_create(100);

        if(epoll_conn_fd == -1)
        {
            std::cerr<<"epoll_create failed!"<<std::endl;
            return;
        }

        auto recv_func = [](int conn_fd,std::string ipandport)->bool{
            struct epoll_event ep_events[100];
            bool is_connecting = true;

            int epoll_conn_fd = epoll_create(100);

            if(epoll_conn_fd == -1)
            {
                std::cerr<<"epoll_create failed!"<<std::endl;
                return false;
            }

            struct epoll_event ep_event_temp;
            ep_event_temp.data.fd = conn_fd;
            ep_event_temp.events = EPOLLIN | EPOLLET;
        
            if(epoll_ctl(epoll_conn_fd,EPOLL_CTL_ADD,conn_fd,&ep_event_temp) == -1)
            {
                std::cerr<<"epoll add failed!"<<std::endl;
                return false;
            }
            
            bool is_recving{false};
            int64_t file_length{0};
            int64_t offset{0};

            std::fstream if_temp;
            int file_name_num = 0;

            while(is_connecting)
            {
                int nfds = epoll_wait(epoll_conn_fd,ep_events,100,-1);
                std::cout<<"after epoll_wait"<<std::endl;
                if(nfds != -1)
                {
                    for(int i = 0;i < nfds;++i)
                    {
                        if(!is_recving)
                        {
                            is_recving = true;

                            int len = recv(ep_events[i].data.fd,&file_length,sizeof(int64_t),0);
                            std::cout<<"file length is "<<file_length<<std::endl;

                            if(len == 0)
                            {
                                if(offset != file_length)
                                {
                                    std::cout<<"recv not finish and connection end"<<std::endl;
                                }
                                is_connecting = false;
                                break;
                            }
                            
                            std::string file_name = ipandport + std::string("_") + std::to_string(file_name_num);
                            if_temp.open(file_name,std::ios::binary | std::ios::out);

                            if(if_temp.is_open())
                            {
                                std::cout<<"file:"<<file_name<<" create successful!"<<std::endl;
                            }
                            else
                            {
                                std::cerr<<"file:"<<file_name<<" create failed!"<<std::endl;
                                return false;
                            }

                            char temp[10000];
                            bool this_round_running{true};
                            while(this_round_running)
                            {
                                memset(temp,0,10000);
                                int remain = file_length - offset;
                                int act_len = {(remain < 10000) ? remain : 10000};
                                int len = recv(ep_events[i].data.fd,temp,act_len,0);
                                if(len > 0)
                                {
                                    if_temp.write(temp,len);
                                    offset += len;
                                    std::cout<<"offset == "<<offset<<std::endl;
                                }
                                else if(len == 0)
                                {
                                    if(offset != file_length)
                                    {
                                        std::cout<<"recv not finish and connection end"<<std::endl;
                                    }
                                    this_round_running = false;
                                }
                                else
                                {
                                    std::cout<<"offset == "<<offset<<" file_length == "<<file_length<<std::endl;
                                    std::cout<<"recv continuing"<<std::endl;
                                    this_round_running = false;
                                }

                                if(offset == file_length)
                                {
                                    std::cout<<"recv finished!"<<std::endl;
                                    file_name_num++;
                                    if(if_temp.is_open())
                                    {
                                        if_temp.close();
                                    }
                                    this_round_running = false;
                                    is_recving = false;
                                    offset = 0;
                                    file_length = 0;
                                }
                            }
                        }
                        else
                        {
                            char temp[10000];
                            bool this_round_running{true};
                            while(this_round_running)
                            {
                                memset(temp,0,10000);
                                int remain = file_length - offset;
                                int act_len = {(remain < 10000) ? remain : 10000};
                                int len = recv(ep_events[i].data.fd,temp,act_len,0);
                                if(len > 0)
                                {
                                    if_temp.write(temp,len);
                                    offset += len;
                                    std::cout<<"offset == "<<offset<<std::endl;
                                }
                                else if(len == 0)
                                {
                                    if(offset != file_length)
                                    {
                                        std::cout<<"recv not finish and connection end"<<std::endl;
                                    }
                                    this_round_running = false;
                                }
                                else
                                {
                                    std::cout<<"offset == "<<offset<<" file_length == "<<file_length<<std::endl;
                                    std::cout<<"recv continuing"<<std::endl;
                                    this_round_running = false;
                                }

                                if(offset == file_length)
                                {
                                    std::cout<<"recv finished!"<<std::endl;
                                    file_name_num++;
                                    if(if_temp.is_open())
                                    {
                                        if_temp.close();
                                    }
                                    this_round_running = false;
                                    is_recving = false;
                                    offset = 0;
                                    file_length = 0;
                                }
                            }
                        }  
                    }
                }
            }
            return true;
        };

        struct epoll_event ep_event,ep_events[100];
        ep_event.data.fd = tcpsocket;
        ep_event.events = EPOLLIN;

        int epoll_fd = epoll_create(100);
        if(epoll_fd == -1)
        {
            std::cerr<<"epoll_create failed!"<<std::endl;
            return;
        }
        
        if(epoll_ctl(epoll_fd,EPOLL_CTL_ADD,tcpsocket,&ep_event) == -1)
        {
            std::cerr<<"epoll_ctl failed!"<<std::endl;
            return;
        }

        if(listen(tcpsocket,10) == -1)
        {
            std::cerr<<"listen failed!"<<std::endl;
            return;
        }

        std::vector<std::future<bool>> fu_vec;

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
                        int flag, old_flag;
                        old_flag = flag = fcntl(conn_fd, F_GETFL, 0);
                        flag |= O_NONBLOCK;
                        fcntl(conn_fd, F_SETFL, flag);
                        if(conn_fd != -1)
                        {
                            std::cout<<"accept connection!"<<std::endl;
                            std::cout<<"remote ip:"<<inet_ntoa(sock_addr_temp.sin_addr)<<std::endl;
                            std::cout<<"remote port:"<<htons(sock_addr_temp.sin_port)<<std::endl;

                            std::string ipandport = inet_ntoa(sock_addr_temp.sin_addr) + std::string(":") + std::to_string(htons(sock_addr_temp.sin_port));

                            fu_vec.push_back(std::async(std::launch::async,recv_func,conn_fd,ipandport));
                        }
                    }
                }
            }
        }
        for(int i = 0;i < fu_vec.size();++i)
        {
            fu_vec[i].get();
        }
    }
    else
    {
        struct sockaddr_in remote_addr;
        remote_addr.sin_family = AF_INET;
        remote_addr.sin_addr.s_addr = inet_addr("192.168.2.100");
        remote_addr.sin_port = ntohs(3000);

        int tcpsocket1 = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
        
        if(connect(tcpsocket1,reinterpret_cast<sockaddr*>(&remote_addr),sizeof(sockaddr)) == -1)
        {
            std::cerr<<"tcpsocket1 connect failed!"<<std::endl;
            return;
        }
        else
        {
            std::cout<<"tcpsocket1 connect successful!"<<std::endl;

            for(int i = 0;i < 10;++i)
            {
                std::ifstream f_temp_1("nohup.out",std::ios::binary | std::ios::in | std::ios::ate);
                std::streampos pos = f_temp_1.tellg();
                int64_t file_length;

                if(pos <= std::numeric_limits<int64_t>::max())
                {
                    file_length = static_cast<int64_t>(pos);
                    std::cout<<"file size is "<<file_length<<std::endl;
                }
                else
                {
                    std::cerr<<"file size is too large!"<<std::endl;
                    return;
                }

                if(f_temp_1.is_open())
                {
                    f_temp_1.close();
                }

                if(send(tcpsocket1,&file_length,sizeof(int64_t),0) != -1)
                {
                    std::cout<<"tcpsocket1 send file_length successful!"<<std::endl;
                }

                std::ifstream f_temp("nohup.out",std::ios::binary | std::ios::in);

                if(!f_temp.is_open())
                {
                    std::cerr<<"f_temp open failed!"<<std::endl;
                    return;
                }
                else
                {
                    std::cout<<"f_temp open successful!"<<std::endl;
                }

                char buffer[10000];
                int j = 0;
                while(!f_temp.eof())
                {
                    memset(buffer,0,10000);
                    f_temp.read(buffer,sizeof(buffer));
                    size_t act_len = f_temp.gcount();
                    if(send(tcpsocket1,buffer,act_len,0) != -1)
                    {
                        std::cout<<"tcpsocket1 send successful!"<<std::endl;
                    }
                }
                if(f_temp.is_open())
                {
                    f_temp.close();
                }
                // std::this_thread::sleep_for(std::chrono::seconds(5));
            }
            
            shutdown(tcpsocket1,SHUT_RDWR);
        }
    }
}

void epoll_test::shm_test(bool is_server)
{
    int fd = shm_open("zbwtest",O_CREAT | O_RDWR,0777);
    if(fd < 0)
    {
        std::cerr<<"shm_open failed!"<<std::endl;
    }

    if(ftruncate(fd,static_cast<off_t>(4096)) < 0)
    {
        std::cerr<<"ftruncate failed!"<<std::endl;
        close(fd);
        return;
    }

    void * ptr = mmap(nullptr,4096,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);

    if(ptr == nullptr)
    {
        std::cerr<<"mmap failed!"<<std::endl;
        close(fd);
        return;
    }

    if(is_server)
    {
        while(true)
        {
            printf("data:%s \n",static_cast<char *>(ptr));
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    else
    {
        sprintf(static_cast<char*>(ptr),"Hello World!");
        std::this_thread::sleep_for(std::chrono::seconds(1));
        sprintf(static_cast<char*>(ptr),"Deep Dark Fantasy!");
    }
}