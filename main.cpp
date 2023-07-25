#include "test.h"
#include <getopt.h>

bool is_server{true};

void parseArgv(int argc,char ** argv)
{
	int opt;
	const char * optstring = "m:";
	opterr = 0;
	
	while((opt = getopt(argc,argv,optstring)) != -1)
	{
		switch(opt)
		{
			case 'm':
			{
				printf("option = m, optarg = %s \n", optarg);
				if(optarg[0] == 'c')
                {
                    is_server = false;
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

    epoll_test(is_server);

    return 0;
}