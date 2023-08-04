#include "test.h"
#include <getopt.h>

bool is_server{true};
enum TRANSPORT_KIND
{
	UDP = 0,
	TCP,
	FTP
};
TRANSPORT_KIND trans_kind = UDP;

void parseArgv(int argc,char ** argv)
{
	int opt;
	const char * optstring = "s:m:";
	opterr = 0;
	
	while((opt = getopt(argc,argv,optstring)) != -1)
	{
		switch(opt)
		{
			case 's':
			{
				printf("option = s, optarg = %s \n", optarg);
				if(optarg[0] == 'c')
                {
                    is_server = false;
                }
				break;
			}
			case 'm':
			{
				printf("option = m, optarg = %s \n", optarg);
				if(std::string(optarg) == "tcp")
                {
                    trans_kind = TCP;
                }
				else if(std::string(optarg) == "ftp")
				{
					trans_kind = FTP;
				}
				break;
			}
			case '?':
			{
				printf("option = ?, optarg = %s \n", optarg);
				break;
			}
			default:
				break;
		}
	}
}

int main(int argc,char ** argv)
{
	parseArgv(argc,argv);
	if(is_server)
	{
		std::cout<<"is server"<<std::endl;
	}
	else
	{
		std::cout<<"is client"<<std::endl;
	}

	epoll_test t;
	if(trans_kind == UDP)
	{
		t.udp_test(is_server);
	}
	else if(trans_kind == TCP)
	{
		t.tcp_test(is_server);
	}
	else
	{
		t.file_transport_test(is_server);
	}

    return 0;
}