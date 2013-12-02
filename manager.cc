#include "project3.h"

using namespace std;

int main(int argc, const char *argv[]) {

	srand(time(NULL));	

	stringstream netData;
	fd_set fdSet;
	int listenSock;
	int port = getRandomPort();
	std::ofstream manOut;


	if ( argc == 2)
	{
		manOut.open("manager.out", ofstream::trunc);
		manOut<<"Starting Manager on port "<<port<<endl;
		manOut<<endl;
		manOut.close();
		listenSock = initNetManager(argv[1], netData, port);
		
		//add listening socket to master set
		FD_SET(listenSock, &fdSet);
	}
	else
	{
		cout<<"Usage:\nmanager [topologyFile]"<<endl;
		return 1;
	}

	
	//cout<<netData.str()<<endl;
	
	//create network routers
	int netSize;
	netData>>netSize;
	manOut.open("manager.out", ofstream::app);
	manOut<<"Initializing Routers... "<<endl;
	manOut<<endl;
	manOut.close();
	initNetRouters(netSize, port);

	//run the manager.
	manOut.open("manager.out", ofstream::app);
	manOut<<"Starting Network Simulation... "<<endl;
	manOut<<endl;
	manOut.close();
	runNetManager(netSize, netData, fdSet, listenSock);
	
	return 0;
}
