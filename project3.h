#ifndef NG_PROJ3_457
#define NG_PROJ3_457 0

#define MAX_BUFFER 255
#define MAX_INPUT 4096
#define PORT_MIN 1024
#define PORT_MAX 65535
#define MANA_MAX_Q 10
#define RDAT_COLS 4
#define RDAT_ID 0
#define RDAT_FD 1
#define RDAT_IN 2
#define RDAT_S	3
#define RDAT_IN_W 0
#define RDAT_IN_R 1
#define RDAT_S_SPAWNED 0
#define RDAT_S_ID 1
#define RDAT_S_LS 2
#define RDAT_S_RDY 3
#define RDAT_S_DONE 4
#define RDAT_S_CLOSED 5
#define CON_S 7
#define CON_VAL "CONFIRM"
#define DIE_S 3
#define DIE_VAL "DIE"
#define LIVE_S 4
#define LIVE_VAL "LIVE"
#define ROUTE_S 5
#define ROUTE_VAL "ROUTE"
#define DELM_S 2
#define DELM_VAL "-1"

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <sstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include <string.h>    //for the damned memset....

//Returns a radom number between PORT_MIN and PORT_MAX.
int getRandomPort()
{
	return rand() % (PORT_MAX-PORT_MIN) + PORT_MIN;
}

//Returns a Socket file descriptor to a TCP socket for accepting connections
//Parameters:  port - the port number tp be used.
int prepListenSocket(const char* port)
{
	struct addrinfo hints;
    struct addrinfo *servResults, *rp;	
	int sockFD = -1;
	int yes = 1;

	memset(&hints, 0, sizeof(struct addrinfo));	
	hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if (getaddrinfo(NULL, port, &hints, &servResults) != 0)
	{
		std::cout<<"Error establshing socket for accepting connections on port "<<port<<std::endl;;
		return -1;
	} 
	
	for (rp = servResults; rp != NULL; rp = rp->ai_next)
	{
		sockFD = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sockFD == -1)
		{
			std::cout<<"Could not bind host to a socket. "<<std::endl;
			continue;
		}
			
		if (setsockopt(sockFD, SOL_SOCKET, SO_REUSEADDR,
			&yes, sizeof(int)) == -1)
		{
			std::cout<<"Could not set the socket options."<<std::endl;
			close(sockFD);
			return -1;
		}
		
		if (bind(sockFD, rp->ai_addr, rp->ai_addrlen) == -1)
		{	
			close(sockFD);
			std::cout<<"Could not bind to port "<<port<<"."<<std::endl;
			continue;
		}
				
		break;
	}
	
	if (rp == NULL) 
	{
		std::cout<<"Unable to create listening socket on port "<<port<<"."<<std::endl;
		return -1;
	}

	freeaddrinfo(servResults);
	
	return sockFD;
}

int prepConnectedSocket(const char* hostname, const char* port)
{
	struct addrinfo hints;
    struct addrinfo *myAddrResults, *rp;	
	int sockFD = -1;
	int yes = 1;

	memset(&hints, 0, sizeof(struct addrinfo));	
	hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

	
	if (getaddrinfo(hostname, port, &hints, &myAddrResults) != 0)
	{
		printf("Error establshing socket to host named %s on port %s", 
			hostname, port);
		return -1;
	} 
	
	for (rp = myAddrResults; rp != NULL; rp = rp->ai_next)
	{
		sockFD = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sockFD == -1)
		{
			printf("Could not bind host to a socket.");
			continue;
		}
				
		if (connect(sockFD, rp->ai_addr, rp->ai_addrlen) == -1) 
		{
            //close(sockFD);
            printf("Could not connect to %s to port %s.", hostname, port);
            continue;
        }	
				
		break;
	}
	
	if (rp == NULL) 
	{
		printf("could not locate host %s", hostname);
		return -1;
	}

	freeaddrinfo(myAddrResults);
	
	return sockFD;
}

int initRouter(int proc)
{

	std::ofstream rOut;
	std::stringstream id;
	id<<"router"<<proc<<".out";
	rOut.open(id.str().c_str());	
	
	rOut<<"Router "<<proc<<" spawned. \n"<<std::endl;
	
	rOut.close();

	return 0;
}

int runNetRouter(int servPort, int proc)
{	
	char fromServ[MAX_BUFFER];
	bool running = true;	

	//open output file();
	std::ofstream rOut;
	std::stringstream id;
	id<<"router"<<proc<<".out";
	rOut.open(id.str().c_str());	

	//TODO CREATE UDP Socket;
		
	//CREATE TCP Socket for comminucation to manager
	std::stringstream ss;
	ss << servPort;
	rOut<<"Connecting to server on port: "<<ss.str()<<"...\n"<<std::endl;
	int servSock = prepConnectedSocket(NULL, ss.str().c_str());
	
	if (servSock == -1)
	{
		rOut<<"Connection failed!"<<std::endl;
		rOut.close();
		return 1;
	}
	else
	{
		rOut<<"Connected.\n"<<std::endl;
	}
		

	rOut<<"Waiting for network ID from manager...\n"<<std::endl;
	//RECV ID from Manager on TCP socket
	memset(fromServ, '\0', MAX_BUFFER);
	int numbytes;
	if ((numbytes = recv(servSock, fromServ, MAX_BUFFER-1, 0)) == -1) {
        //TODO: print error to log
    }

	std::string myNetID(fromServ);
	rOut<<"My ID from manager is: "<<myNetID<<std::endl<<std::endl; 	

	//SEND CONFIRM GO TO ID STATE
	send(servSock, CON_VAL, CON_S, 0);
	rOut<<"Confirmed to manager. Go to identified state.\n"<<std::endl;
	rOut<<"Waiting for Link information form manager...\n";
	
	//RECV the LS INFO FROM Manager
	memset(fromServ, '\0', MAX_BUFFER);
	if ((numbytes = recv(servSock, fromServ, MAX_BUFFER-1, 0)) == -1) {
        //TODO: print error to log
    }
	
	std::string links(fromServ);
	rOut<<"Link information reveived:\n";
	rOut<<links<<std::endl<<std::endl; 

	//SEND CONFIRM GO TO LS STATE
	send(servSock, CON_VAL, CON_S, 0);
	rOut<<"Confirmed to manager. Go to LS state.\n"<<std::endl;
	
	//TODO DO LS for SPT
	rOut<<"Link STATE algorithm UNIMPLEMENTED! \n"<<std::endl;

	
	//CONFIRM GO TO RDY STATE
	send(servSock, CON_VAL, CON_S, 0);
	rOut<<"FALSE-Confirmed to manager. LS not actually complete! Go to ready state.\n"<<std::endl;
	

	rOut<<"Waiting for simulation packets...\n"<<std::endl;
	//TODO get messages form server
	while(running)
	{
		//RECV COMMAND FROM SERVER

		
		memset(fromServ, '\0', MAX_BUFFER);
		int numbytes;
		if ((numbytes = recv(servSock, fromServ, MAX_BUFFER-1, 0)) == -1) {
        	//TODO: print error to log
    	}
	
		std::string msg(fromServ);
		rOut<<"Recieved message from manger: "<<msg<<std::endl;		
		
		//message says to end simulation.
		if (strncmp(fromServ, DIE_VAL, DIE_S) == 0) 
		{
			running = false;
			rOut<<"Manager told me to die. Ending process..."<<std::endl;
		}
		//message says to keep alive.
		else if (strncmp(fromServ, LIVE_VAL, LIVE_S) == 0)
		{
			
		}

		else if (strncmp(fromServ, ROUTE_VAL, ROUTE_S) == 0)
		{
			rOut<<"SUDO-Routing packet.  No routing implemented!\n"<<std::endl;
			//ROUTE a PACKET.
			//get my shortest path to des.
			//sent message to next node in path to dest
		}
	}
	
	//CONFIRM GO TO DONE STATE
	send(servSock, CON_VAL, CON_S, 0);
	rOut<<"Confirmed to manager. Going to done state.\n"<<std::endl;
	rOut<<"Shutting down router.\n"<<std::endl;
	//close output file
	rOut.close();
	return 0; 
}

int initNetManager(std::string fname, std::stringstream &data, int myPort) {	

	//read input file
	std::string line;
	std::ifstream mFile(fname.c_str());
	std::ofstream manOut;
     	
	if (mFile.is_open())
	{
		while (getline(mFile, line) )
		{
			data<<line + "\n";
		}
		mFile.close();
	}	
	else
	{
		std::cerr<<"Unable to open Topology file "<<fname<<"."<<std::endl;
		//ToDO try harder not to crash...
		return -1;
	}

	//Setup TCP socket for listening.
	
	manOut.open("manager.out", std::ofstream::app);
	manOut<<"Creating TCP socket for accepting connections..."<<std::endl;
	manOut<<std::endl;
	std::stringstream ss;
	ss << myPort;
	int sock = prepListenSocket(ss.str().c_str());
	manOut<<"Accepting connections on sock number "<<sock<<std::endl;
	manOut<<std::endl;

	manOut<<"Listening for incoming connections..."<<std::endl;
	manOut<<std::endl;

	//Start listening
	if (listen(sock, MANA_MAX_Q) == -1)
	{
		manOut<<"Manager unable to listen for incoming connections."<<std::endl;
		manOut<<std::endl;
		return -1;
	}

	manOut.close();
	return sock;
}

//Fork n time to create the router proccesses.  
int initNetRouters(int numNodes, int sPort) {
	
	int i = 0;
	int pid = -1;           //pID of the forked process. 
	std::ofstream pool;
	pool.open("pool.out", std::ofstream::trunc);		//file to record all the pid of children processes

	std::ofstream manOut;
	manOut.open("manager.out", std::ofstream::app);

	while (i < numNodes) {
		
		//spawn child
		manOut<<"Forking router "<<i<<std::endl;
		manOut<<std::endl;

		pid = fork();
		
		if (pid == 0)  //in child
		{		
			//kick the child out of loop it should not fork.
			//initialize the router.
			initRouter(i);
		
			//call a "run net router" method that returns when the 
			//Manager send a kill message or the proccess terminates.
			runNetRouter(sPort, i); 

			//exit this method terminating the process.
			_exit(0);
		}
		else if (pid > 0)  //still in parent
		{
			//record pid of child in
			pool << pid <<"\n";	
		}
		else 
		{
			std::cerr<<"Error spawning child "<<i<<" "<<errno<<std::endl;
		}
		
		i++;		//next fork
	}
	
	pool.close();
	manOut.close();

	//After parent has spawned all the children he continues.
	return 0;	
}

int runNetManager(int size, std::stringstream &data, fd_set &masterSet, int maxfd){

	bool finished = false;
	bool closing = false;
	bool routing = false;
	//bool done = false;
	fd_set fdRdable;
	fd_set fdWrable;	
	struct sockaddr_storage remoteaddr;
	socklen_t addrlen;
	char fromClient[MAX_BUFFER];
	std::ofstream manOut;
	manOut.open("manager.out", std::ofstream::app);

	//create a table of router information and include the listening socket.	
	int Rdat[size+1][RDAT_COLS];			//router data
	int listener = 0;
	int fdCount = 1;				//current number of socket File Descriptors
	int newfd;						//newest connection from a router
	//the incoming connection socket has no network ID
	Rdat[listener][RDAT_ID] = -1;
	//the FD for the listener was passed in as initial maxfd
	Rdat[listener][RDAT_FD] = maxfd;
	//set the incoming status for listener but it should not be used.
	Rdat[listener][RDAT_IN] = 1;
	//set the status of the listener to closed so that it does not
	//stop the manager from ending simulation.
	Rdat[listener][RDAT_S] = RDAT_S_CLOSED;		//its not really closed!

	//Create table of router Link States.
	std::vector<std::string> links;
	std::string line;
	manOut<<"Building Link State Table:\n";
	getline(data, line);
	manOut<<line<<std::endl;
	while ( strncmp( line.c_str(), DELM_VAL, DELM_S ) != 0) 
	{
		links.push_back(line);
		getline(data, line);
		manOut<<line<<std::endl;
	}
	int linkIndex = 0;
	int linkCount = links.size();
	manOut<<linkCount<<"\n"<<std::endl;

	//Create table of Packets to route in simulation.
	std::vector<std::string> packages;
	getline(data, line);
	manOut<<"Building Package Table:\n";
	manOut<<line<<std::endl;
	while (strncmp( line.c_str(), DELM_VAL, DELM_S ) != 0)
	{
		packages.push_back(line);
		getline(data, line);
		manOut<<line<<std::endl;
	}
	int packageIndex = 0;
	int packageCount = packages.size();
	manOut<<packageCount<<"\n"<<std::endl;
	
	while (!finished)
	{
		fdRdable = masterSet;
		fdWrable = masterSet;
		if (select(maxfd+1, &fdRdable, &fdWrable, NULL, NULL) == -1) {
			std::cerr<<"select error "<<std::endl;
		}
		
		//loop through all socket file descriptors
		for (int i = 0; i < fdCount; i++)
		{
			//if the current FD is readable.  IT has a message
			if (FD_ISSET(Rdat[i][RDAT_FD], &fdRdable))
			{
				//if the current fd is the listener
				if (i == listener)  
				{
					addrlen = sizeof remoteaddr;
					newfd = accept(Rdat[listener][RDAT_FD], 
						(struct sockaddr *)&remoteaddr,
						&addrlen);

					if (newfd == -1)
					{
						std::cerr<<"Manager unable to establish connection with router"<<std::endl;
					}
					else 
					{
						//add the new connection to master list
						FD_SET(newfd, &masterSet);

						//record the new max FD
						if (newfd > maxfd)
							maxfd = newfd;

						//set the Rdat table values
						Rdat[fdCount][RDAT_ID] = fdCount-1;
						Rdat[fdCount][RDAT_FD] = newfd;
						Rdat[fdCount][RDAT_IN] = RDAT_IN_W;
						Rdat[fdCount][RDAT_S] = RDAT_S_SPAWNED;
						fdCount += 1;
						
						//PRINT to LOG information about router connection
						manOut<<"Accepted Connection from node: ";
						manOut<<Rdat[fdCount-1][RDAT_ID];
						manOut<<" on FD: "<<newfd<<" Max FD: "<<maxfd<<std::endl;
						manOut<<std::endl;
					}
				}
				//current FD is not the listener, but readable must 
				//be a router socket confirming
				else
				{
					//recv from the socket...
					memset(fromClient, '\0', MAX_BUFFER);					
					int numbytes;
					if ((numbytes = recv(Rdat[i][RDAT_FD], fromClient, MAX_BUFFER-1, 0)) == -1) 
					{
        				//LOG ERROR to Manager.out
						manOut<<"Unable to read from node "<<Rdat[i][RDAT_ID];
						manOut<<std::endl<<std::endl;
					}

					if (numbytes == 0) 
					{
						//TODO
						//connection closed
						//close(i);
						//FD_CLR(i, &masterSet);
						//print message to log router ended its self?
						Rdat[i][RDAT_S] = RDAT_S_DONE;
							
					}
					else if (numbytes > 0)
					{
						//check that this is a valid confirm
						if (strncmp(fromClient, CON_VAL, CON_S) == 0)
						{
							manOut<<"Node "<<Rdat[i][RDAT_ID];
							manOut<<" CONFIRMED! bumping status from ";
							manOut<<Rdat[i][RDAT_S];	
							++Rdat[i][RDAT_S];
							Rdat[i][RDAT_IN] = RDAT_IN_W;
							manOut<<" to "<<Rdat[i][RDAT_S]<<std::endl<<std::endl;
						}
						else
						{
							//TODO log that router sent mangled confirm.
							manOut<<"Mangled confirm from node "<<Rdat[i][RDAT_ID];
							manOut<<std::endl<<std::endl;	
						}
						
						
					}
				}
			}
			
			//if it is not in the reads set we can send it a message
			else if (FD_ISSET(Rdat[i][RDAT_FD], &fdWrable) && 
						Rdat[i][RDAT_IN] == RDAT_IN_W)
			{
				bool sent = false;
				// check the Writeable Router's Status	
				if (Rdat[i][RDAT_S] == RDAT_S_SPAWNED)
				{
					//send ID
					std::stringstream ID;
					ID << Rdat[i][RDAT_ID];
					send(Rdat[i][RDAT_FD], ID.str().c_str(), ID.str().length(), 0); 
					//print to log router request ID
					manOut<<"Sent ID: "<<ID.str()<<" to node ";
					manOut<<Rdat[i][RDAT_ID]<<std::endl;
					manOut<<std::endl;
					sent = true;
				}
				else if (Rdat[i][RDAT_S] == RDAT_S_ID)
				{
					//get the node ID.
					std::stringstream node;
					node << Rdat[i][RDAT_ID];
						
					std::string LS = "";
					//parse out entries where node ID is the source.
					int count = 0;
					for (linkIndex = 0; linkIndex < linkCount; ++linkIndex)
					{
						if (strncmp(links[linkIndex].c_str(), node.str().c_str(), 1)
							== 0)
						{	
							LS.append(links[linkIndex] + "\n");
							count += 1;
						}
					}	

					//send link data to router.
					std::stringstream c;
					c<<count;
					LS.append(c.str()+"\n");
					send(Rdat[i][RDAT_FD], LS.c_str(), LS.length(), 0);

					//print to log router requested LS
					manOut<<"Sent LS to node "<<Rdat[i][RDAT_ID];
					manOut<<std::endl;
					manOut<<"Sent: "<<LS<<std::endl<<std::endl;
					sent = true;				
				}
				else if ( (Rdat[i][RDAT_S] == RDAT_S_RDY) && routing)
				{
					//send the nextMessage to router.
					std::stringstream node;
					node << Rdat[i][RDAT_ID];
					//if this is the SRC node of the next pack to route
					if (packageIndex < packageCount &&
							strncmp(node.str().c_str(), packages[packageIndex].c_str(), 
							node.str().length()) == 0)
					{
						//get the destination.
						
						
						//TODO Send package to node
					    
						//TODO sleep or wait for completion confirmation.
						//LOG package transmit strated
						manOut<<"Initiating traffic from node "<<Rdat[i][RDAT_ID];
						manOut<<"\n"<<std::endl;	
						packageIndex += 1;
					}
					//this is not SRC of next package to route
					//send keep alive
					else
					{
						send(Rdat[i][RDAT_FD], LIVE_VAL, LIVE_S, 0);
						//manOut<<"Sent LIVE message to node "<<Rdat[i][RDAT_ID];
						//manOut<<std::endl;
						//sent = true;
					}		

				} 
				else if ( (Rdat[i][RDAT_S] == RDAT_S_RDY) && closing)
				{
					//send done signal
					send(Rdat[i][RDAT_FD], DIE_VAL, DIE_S, 0);
					//update Rdat table to show this Router is Done
					Rdat[i][RDAT_S] = RDAT_S_DONE;
					//print log message
					manOut<<"Sent DIE message to node "<<Rdat[i][RDAT_ID];
					manOut<<std::endl;
					sent = true;
				}

				if (sent)
					Rdat[i][RDAT_IN] = RDAT_IN_R;
			}
		}

		//CHECK AND UPDATE THE STATUS OF SIMULATION!!!!
		routing = true;
		//check if the string stream still has data.
		if (packageIndex == packageCount)
		{
			closing = true;
			manOut<<"Finished all packets to route"<<std::endl<<std::endl;
		}
		if (closing)
		{
			finished = true;
		}
		for (int i = 0; i < fdCount; i++)
		{
			if (i != listener && Rdat[i][RDAT_S] < RDAT_S_RDY)
			{
				routing = false;
			}
			if (closing && i != listener && (Rdat[i][RDAT_S] < RDAT_S_DONE))
			{
				finished = false;
			}
		}	
		if (closing)
			routing = false;
	}
	

	manOut<<"Ending Simulation!"<<std::endl<<std::endl;
	manOut.close();
	return 0;
}

#endif
