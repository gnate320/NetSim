#include "project3.h"

using namespace std;

int main(int argc, const char *argv[]) {

	srand(time(NULL));	

	stringstream netData;
	fd_set fdSet;
	int listenSock;
	int port = getRandomPort();

	if ( argc == 2)
	{
		listenSock = initNetManager(argv[1], netData, port);
		
		//add listening socket to master set
		FD_SET(listenSock, &fdSet);
	}
	else
	{
		cout<<"Usage:\nmanager [topologyFile]"<<endl;
	}

	
	//cout<<netData.str()<<endl;
	
	//create network routers
	int netSize;
	netData>>netSize;
	initNetRouters(netSize, port);

	//run the manager.
	runNetManager(netSize, netData, fdSet, listenSock);
	
	return 0;
}
