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
	if(argc < 2 ||argc > 3){
	    return 0;
	}
	if(argc == 2){
	    portNumber = atoi(argv[1]);
	}
	if(argc == 3){
	    portNumber = 63479;
	}
	//Create the socket, check if success
	if((socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		close(socket);
		exit(0);
	}
	//Allocate memory for server
	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr = htonl(INADDR_ANY); //Get long integer, convert to Network byte order
	server.sin_port = htons(portNumber); //Set port number for server
	
	//Bind Socket to socket address, exit if fail
	if(bind(socket, (struct sockaddr *) &server, sizeof(struct sockaddr_in)) < 0){
	    close(socket);
	    exit(0);
	}

	return 0;
}
