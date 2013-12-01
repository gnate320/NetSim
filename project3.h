#ifndef NG_PROJ3_457
#define NG_PROJ3_457 0

#define MAX_BUFFER 255
#define MAX_INPUT 4096
#define PORT_MIN 1024
#define PORT_MAX 65535
#define MANA_MAX_Q 10
#define RDAT_COLS 3
#define RDAT_ID 0
#define RDAT_FD 1
#define RDAT_S	2
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

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <unistd.h>
#include <sstream>
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

int initRouter()
{
	return 0;
}

int runNetRouter(int servPort)
{	
	char fromServ[MAX_BUFFER];
	bool running = true;	

	//TODO CREATE UDP Socket;
		
	//CREATE TCP Socket for comminucation to manager
	std::stringstream ss;
	ss << servPort;
	int servSock = prepConnectedSocket(NULL, ss.str().c_str());
		
	// TODO ?? SEND CONFIRM TO MANAGER TO GET to spawned state ??
	//send(servSock, CON_VAL, CON_S, 0);
		
	//RECV ID from Manager on TCP socket
	memset(fromServ, '\0', MAX_BUFFER);
	int numbytes;
	if ((numbytes = recv(servSock, fromServ, MAX_BUFFER-1, 0)) == -1) {
        //TODO: print error to log
    }

	//SEND CONFIRM GO TO ID STATE
	send(servSock, CON_VAL, CON_S, 0);

	//RECV the LS INFO FROM Manager
	memset(fromServ, '\0', MAX_BUFFER);
	if ((numbytes = recv(servSock, fromServ, MAX_BUFFER-1, 0)) == -1) {
        //TODO: print error to log
    }

	//SEND CONFIRM GO TO LS STATE
	send(servSock, CON_VAL, CON_S, 0);
	
	//TODO DO LS for SPT
	
	//CONFIRM GO TO RDY STATE
	send(servSock, CON_VAL, CON_S, 0);

	//TODO get messages form server
	while(running)
	{
		//RECV COMMAND FROM SERVER
		memset(fromServ, '\0', MAX_BUFFER);
		int numbytes;
		if ((numbytes = recv(servSock, fromServ, MAX_BUFFER-1, 0)) == -1) {
        	//TODO: print error to log
    	}
			
		if (strncmp(fromServ, DIE_VAL, DIE_S) == 0) 
			running = false;

		else
		{
			//ROUTE a PACKET.
			//get my shortest path to des.
			//sent message to next node in path to dest
		}
	}
	
	//CONFIRM GO TO DONE STATE
	send(servSock, CON_VAL, CON_S, 0);
	
	return 0; 
}

int initNetManager(std::string fname, std::stringstream &data, int myPort) {	

	//read input file
	std::string line;
	std::ifstream mFile(fname.c_str());
     	
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
	
	std::stringstream ss;
	ss << myPort;
	int sock = prepListenSocket(ss.str().c_str());

	//Start listening
	if (listen(sock, MANA_MAX_Q) == -1)
	{
		std::cerr<<"Manager unable to listen for incoming connections."<<std::endl;
		return -1;
	}

	return sock;
}

//Fork n time to create the router proccesses.  
int initNetRouters(int numNodes, int sPort) {
	
	int i = 0;
	int pid = -1;           //pID of the forked process. 
	std::ofstream pool;
	pool.open("pool.out");		//file to record all the pid of children processes

	while (i < numNodes) {
		
		//spawn child
		pid = fork();
		
		if (pid == 0)  //in child
		{		
			//kick the child out of loop it should not fork.
			break;
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

	// If we are in the child
	if (pid == 0)
	{
		//initialize the router.
		initRouter();
		
		//call a "run net router" method that returns when the 
		//Manager send a kill message or the proccess terminates.
		runNetRouter(sPort); 

		//exit this method terminating the process.
		_exit(0);
	}

	//After parent has spawned all the children he continues.
	return 0;	
}

int runNetManager(int size, std::stringstream &data, fd_set &masterSet, int maxfd){

	bool finished = false;
	bool closing = false;
	bool routing = false;
	//bool done = false;
	fd_set fdRdable;	
	struct sockaddr_storage remoteaddr;
	socklen_t addrlen;
	int newfd;
	int Rdat[size+1][RDAT_COLS];			//router data
	char fromClient[MAX_BUFFER];

	int listener = maxfd;

	while (!finished)
	{
		fdRdable = masterSet;
		if (select(maxfd+1, &fdRdable, NULL, NULL, NULL) == -1) {
			std::cerr<<"select error "<<std::endl;
		}
		
		//loop through reabable socket file descriptors
		for (int i = 0; i < maxfd+1; i++)
		{
			//if the current FD is reabable.  IT has a message
			if (FD_ISSET(i, &fdRdable))
			{
				//if the current fd is the listener
				if (i == listener)  
				{
					addrlen = sizeof remoteaddr;
					newfd = accept(listener, 
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
						Rdat[newfd][RDAT_ID] = newfd;
						Rdat[newfd][RDAT_FD] = newfd;
						Rdat[newfd][RDAT_S] = RDAT_S_SPAWNED;
						
						//TODO PRINT to LOG information about router connection
					}
				}
				//current FD is not the listener, but readable must 
				//be a router socket confirming
				else
				{
					//recv from the socket...
					memset(fromClient, '\0', MAX_BUFFER);					
					int numbytes;
					if ((numbytes = recv(i, fromClient, MAX_BUFFER-1, 0)) == -1) 
					{
        				//TODO: LOG ERROR to Manager.out
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
					else
					{
						//check that this is a valid confirm
						if (strncmp(fromClient, CON_VAL, CON_S) == 0)
						{
							++Rdat[i][RDAT_S];
						}
						else
						{
							//TODO log that router sent mangled confirm.	
						}
						
					}
				}
			}
			
			//if it is not in the reads set we can send it a message
			else
			{
				//TODO check the Writeable Router STATus
					
				if (Rdat[i][RDAT_S] == RDAT_S_SPAWNED)
				{
					//send ID
					std::stringstream ID;
					ID << Rdat[i][RDAT_ID];
					send(i, ID.str().c_str(), ID.str().length(), 0); 
					//print to log router request ID
				}
				else if (Rdat[i][RDAT_S] == RDAT_S_ID)
				{
					//get the node ID from recved data.
					//parse out entries where node ID is the source.
					//send link data to router.
					//print to log router requested LS				
				}
				else if ( (Rdat[i][RDAT_S] == RDAT_S_RDY) && routing)
				{
					//send the nextMessage to router.
					//sleep or wait for completion confirmation.
					//get the next message 
				} 
				else if ( (Rdat[i][RDAT_S] == RDAT_S_RDY) && closing)
				{
					//send done signal
					send(i, DIE_VAL, DIE_S, 0);
					//update Rdat table to show this Router is Done
					Rdat[i][RDAT_S] = RDAT_S_DONE;
				}
			}
		}

		//CHECK AND UPDATE THE STATUS OF SIMULATION!!!!
		routing = true;
		//check if the string stream still has data.
		if (!closing && data.peek() == '-')
		{
			int delim;
			data >> delim;
			if (delim == -1)
			{
				closing = true;
				finished = true;
			}
		}
		else if (closing)
		{
			finished = true;
		}
		for (int i = 0; i < maxfd+1; i++)
		{
			//if (i != listener && FD_ISSET(i, &masterSet) )
			//{
			//	setEmpty = false;
			//}
			if (i != listener && Rdat[i][RDAT_S] < RDAT_S_RDY)
			{
				routing = false;
			}
			if (closing && i != listener && (Rdat[i][RDAT_S] < RDAT_S_DONE))
			{
				finished = false;
			}
		}	
	
		//if (setEmpty)
		//{
		//	running = false;
		//}
	}

	return 0;
}

#endif
