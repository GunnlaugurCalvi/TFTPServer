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
#include <stdbool.h>

#define  FILESIZE  512

struct RRQ{
	short opC;
	char fileName[FILESIZE];
	short bt;
	char mode[FILESIZE];
};
struct DATA{
	short opC;
	short blockNr;
	char databuf[FILESIZE];
};
struct ERROR{
	short opC;
	short errC;
	char errMsg[FILESIZE];
};
struct ACK{
	short opC;
	short blockNr;
};

int packetSender(int socketfd, struct DATA datablock, int byteCount, struct sockaddr_in *clientAddr, socklen_t len);

int main(int argc, char *argv[])
{
	
	int sock;
	struct sockaddr_in server, client;
	int portNumber;
	//ssize_t pack;
	struct DATA data;
	struct ACK ack;
	struct RRQ request;
	int nextBlock = 1;
	char buf[PATH_MAX + 1];
	char *dir = argv[2];
	char *res = realpath(dir, buf);
	char RT [3];
	char RRQ [] = "RRQ";
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
		perror("Socket Error");
		exit(EXIT_FAILURE);
	}
	//Allocate memory for server
	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET; //WE ARE FAMI LY
	server.sin_addr.s_addr = htonl(INADDR_ANY); //Get long integer, convert to Network byte order
	server.sin_port = htons(portNumber); //Set port number for server
	
	//Bind Socket to socket address, exit if fail
	if(bind(sock, (struct sockaddr *) &server, sizeof(struct sockaddr_in)) < 0){
	    	perror("Bind error");
		exit(EXIT_FAILURE);
	}

	while(1){

		fd_set readfds;
		//struct timeval tv;
		ssize_t val;
		FD_ZERO(&readfds);
		FD_SET(sock, &readfds);
		//tv.tv_sec = 5;
		//tv.tv_usec = 0;
		//val = select(sock + 1, &readfds, NULL,NULL, &tv);
		char fileData[FILESIZE];
		int fDataRead = 0;
		bool isReading = true;
			
		
		socklen_t clientlen = (socklen_t) sizeof(client);
	        //Recieve request from client
		if((val = recvfrom(sock, (void *) &request, sizeof(request), 0, (struct sockaddr *) &client, &clientlen)) < 0){
			perror("Request recieve");
			exit(EXIT_FAILURE);		
		}
		//Get RRQ and mode
		if(ntohs(request.opC) == 1){
			strncpy(RT, RRQ, sizeof(RRQ));
			strncpy(request.mode, "octet", sizeof(request.mode));
		}		
		 	
		//message[pack] = '\0';
		char *clientIP = inet_ntoa(client.sin_addr);
		unsigned short sPort = ntohs(client.sin_port);
		fprintf(stdout, "file '%s' requested from %s:%d\n", res, clientIP, sPort);
			
			//If realpath doesnt exist
			/*if(!res){		
				perror("realpath");
				exit(EXIT_FAILURE);				
			}*/
		fflush(stdout);
			//packetSender(sock, message, (size_t) pack, &client, clientlen);
			//sendto(sock, message, (size_t) pack, 0, (struct sockaddr *)&client, clientlen);
			
		printf("Fyrir File");
			//Open the requested file
		FILE *filep = fopen(buf, "rb");
		if(!filep){
			perror("The file could naaat be opened!!\n");
			
		}
			/*if((val = sendto(sock,&fileData, (size_t) fDataRead + 4,0, (struct sockaddr *) &client,clientlen)) < 0){
					perror("filedata sendto() failed like shit brrrr\n");
					exit(EXIT_FAILURE);
			}*/
		memset(&data, 0, sizeof(data));
		memset(&ack, 0, sizeof(ack));
		printf("Fyrir while");
			
		while(1){
			printf("Seinni while");
			data.opC = htons(3);
			data.blockNr = htons(nextBlock);
			if(isReading){
				printf("You is Reading");
				memset(data.databuf, 0, FILESIZE);
				fDataRead = fread(&fileData, 1, FILESIZE, filep);
			}
			isReading = false;
			
			if((val = sendto(sock, &fileData, (size_t) fDataRead + 4, 0, (struct sockaddr *) &client, clientlen)) < 0){
				perror("Error in sending file\n");
				exit(EXIT_FAILURE);
			}
			memset(&ack, 0, sizeof(ack));
			if((val = recvfrom(sock, (void *) &ack, FILESIZE, 0, (struct sockaddr *) &client, &clientlen)) < 0){
				perror("Error in recieving file\n");
				exit(EXIT_FAILURE);
			}
				
			ack.opC = ntohs(4);
			ack.blockNr = ntohs(nextBlock);
				
			if(ack.opC == 4 && ack.blockNr == nextBlock){

				nextBlock++;
				isReading = true;
				if(fDataRead < FILESIZE){
					break;
				}
			}
			else if(ack.opC == 5){
				perror("Error Acknowledgement");
				break;
			}
		}
		fclose(filep);
						
	}
			
	return 0;
}
int packetSender(int socketfd, struct DATA datablock, int byteCount, struct sockaddr_in *clientAddr, socklen_t len){
	
	return sendto(socketfd, &datablock, byteCount, 0, (struct sockaddr *) &clientAddr, len);
}

