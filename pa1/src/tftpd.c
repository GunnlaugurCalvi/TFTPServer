#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/select.h>
#include <limits.h>

int main(int argc, char *argv[])
{
	
	int sock;
	struct sockaddr_in server, client;
	int portNumber;
    ssize_t pack;
    char message[512];
	printf("%d\n", argc);
	if(argc < 2 ||argc > 3){
	    printf("Invalid input! \n");
		return 0;
	}
	if(argc == 2 || argc == 3){
	    portNumber = atoi(argv[1]);
		printf("using portnumber %d\n", portNumber);
	}
	//Create the socket, check if success
	if((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		//close(sock);
		exit(0);
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

	while(1){

		fd_set readfds;
		struct timeval tv;
		int val;
		FD_ZERO(&readfds);
		FD_SET(sock, &readfds);
		tv.tv_sec = 5;
		tv.tv_usec = 0;
		val = select(sock + 1, &readfds, NULL,NULL, &tv);
		char buf[PATH_MAX + 1];
		char fileData[512];
		int fDataRead = 0;
		if(val < 0){
			perror("ERROR");
		}
		else if(val > 0){
			
		
			socklen_t clientlen = sizeof(struct sockaddr_in);
	    
			pack = recvfrom(sock, message, 1024, 0, (struct sockaddr *)&client, &clientlen);
			
		 	
	        message[pack] = '\0';
			char *clientIP = inet_ntoa(client.sin_addr);
			unsigned short sPort = ntohs(client.sin_port);

			fprintf(stdout, "file '%s' requested from %s:%d\n", &message[2], clientIP, sPort);
			
			char *res = realpath(&message[2], buf);
			//If realpath doesnt exist
			if(!res){		
				perror("realpath");
				exit(EXIT_FAILURE);				
			}
			fflush(stdout);

	        sendto(sock, message, (size_t) pack, 0, (struct sockaddr *)&client, clientlen);
			
			FILE *filep = fopen(buf, "r");
			if(!filep){
				perror("The file could naaat be opened!!\n");
			}
			else{
				printf("file is being opened that what she said\n");
				if(1){
					memset(fileData,0,512);
					fDataRead = fread(&fileData,1,512,filep);
				}
				if((val = sendto(sock, &fileData, (size_t) fDataRead + 4,0, (struct sockaddr *) &client,clientlen) < 0)){
					perror("filedata sendto() failed like shit brrrr\n");
					exit(EXIT_FAILURE);
				} 
			}
				fclose(filep);
			
			}
			
			else{
				fprintf(stdout, "NO CONNECTION\n");
				fflush(stdout);
						
			}
		
	}
	return 0;
}
