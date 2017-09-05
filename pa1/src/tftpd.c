/* your code goes here. */
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
int main(int argc, char *argv[])
{
	
	int socket;
	struct sockaddr_in server, client;
	int portNumber;
	if(argz < 2 ||argz > 3){
	    return 0;
	}
	if(argz == 2){
	    portNumber = atoi(argv[1]);
	}
	if(argz == 3){
	    portNumber = 
	}
	//Create the socket, check if success
	if((socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		close(socket);
		exit(0);
	}
	//Allocate memory for server
	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr
	server.sin_port = htons(portNumber);
	return 0;
}
