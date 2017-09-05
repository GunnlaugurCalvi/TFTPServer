#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
int main(int argc, char *argv[])
{
	
	int sock;
	struct sockaddr_in server, client;
	int portNumber;
        ssize_t pack;
        char message[512];
	if(argc < 2 ||argc > 3){
	    return 0;
	}
	if(argc == 2){
	    portNumber = atoi(argv[1]);
	}
	if(argc == 3){
	    portNumber = 63479; //Default port Frilla
	}
	//Create the socket, check if success
	if((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		//close(sock);
		exit(0);
	}
	else{
		 printf("Socket created");
	}
	//Allocate memory for server
	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET; //WE ARE FAMI LY
	server.sin_addr.s_addr = htonl(INADDR_ANY); //Get long integer, convert to Network byte order
	server.sin_port = htons(portNumber); //Set port number for server
	
	//Bind Socket to socket address, exit if fail
	if(bind(sock, (struct sockaddr *) &server, sizeof(struct sockaddr_in)) < 0){
	    //close(sock);
		exit(0);
	}
	else{
		printf("Binded that biiiiiiiiiii");
	}

	while(1){

		socklen_t clientlen = sizeof(struct sockaddr_in);
	    
		pack = recvfrom(sock, message, 1024, 0, (struct sockaddr *)&client, &clientlen);
	    
		if(pack < 0){
			printf("Sendin this biiiii no MO");
	      //Pakki nadist ekki
	      //tharf ad returna error
			exit(0);
	        }
	        message[sizeof(pack)-1] = '\0';
		fprintf(stdout, "Recieved: \n%s\n", message);
		fflush(stdout);
	        sendto(sock, message, (size_t) pack, 0, (struct sockaddr *)&client, clientlen);
	
		if(pack < 0){
			printf("I aint sendin this biiii no MO");
			exit(0);
		}

        }
	return 0;
}
