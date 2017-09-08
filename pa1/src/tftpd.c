#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <limits.h>
#include <stdbool.h>

#define FILESIZE 512

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


//void concatPath(char *serverFolder, char *filePath, char fullPath[PATH_MAX]);

int main(int argc, char *argv[])
{
	
	int sock;
	struct sockaddr_in server, client;
	int portNumber;
	struct DATA data;
	struct ACK ack;
	struct RRQ request;
	int nextBlock = 1;
	char buf[PATH_MAX + 1];
	char *dir = argv[2];
	char *res = realpath(dir, buf);
	char RT [3];
	char RRQ [] = "RRQ";
	char fullPath[PATH_MAX];
	bool isReading = true;
	
	//DEBUG PRINT
	printf("%d\n", argc);
	
	if(argc != 3){
	    printf("Invalid input! \n");
		return 0;
	}
	if(argc == 3){
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

	while(true){
		ssize_t val;
		int fDataRead = 0;
		nextBlock = 1;	
		
		memset(&request, 0, sizeof(request));		
		socklen_t clientlen = (socklen_t) sizeof(client);
	        
		//Recieve request from client
		if((val = recvfrom(sock, (void *) &request, sizeof(request), 0, (struct sockaddr *) &client, &clientlen)) < 0){
			perror("Request recieve");
			exit(EXIT_FAILURE);		
		}
		 
		//Get RRQ and mode
		if(ntohs(request.opC) == 1){
			printf("do I even get here bro with dis shit -> %d  -> %d\n", request.opC, ntohs(request.opC));
			fflush(stdout);
			strncpy(RT, RRQ, sizeof(RRQ));
			strncpy(request.mode, "octet", sizeof(request.mode));
		}
		else
		{
			perror("REQUEST OPCODE ERROR!");
			fflush(stdout);
		}	
		memset(&fullPath, 0, sizeof(fullPath));
		strcpy(fullPath, res);
		strcat(fullPath, "/");
		strcat(fullPath, request.fileName);	
		char *clientIP = inet_ntoa(client.sin_addr);
		unsigned short sPort = ntohs(client.sin_port);
		fprintf(stdout, "file '%s' requested from %s:%d\n", fullPath, clientIP, sPort);
			
		fflush(stdout);
			
		printf("Fyrir File\n");
			//Open the requested file
		FILE *filep = fopen(fullPath, "rb");
		if(!filep){
			perror("The file could naaat be opened!!\n");
			
		}

		memset(&data, 0, sizeof(data));
		memset(&ack, 0, sizeof(ack));
		
		printf("%s\n%s\n%s\n",(char *)RT, (char *)request.mode, (char *)request.fileName);
		fflush(stdout);	

		while(true){

			printf("Seinni while\n");
			fflush(stdout);
			data.opC = htons(3);
			data.blockNr = htons(nextBlock);
			if(isReading){
				printf("You is Reading\n");
				memset(data.databuf, 0, FILESIZE);
				fDataRead = fread(&data.databuf, 1, FILESIZE, filep);
			}
			isReading = false;
			//send that datablock
			if((val = sendto(sock, &data, (size_t) fDataRead + 4, 0, (struct sockaddr *) &client, clientlen)) < 0){
				perror("Error in sending file\n");
				exit(EXIT_FAILURE);
			}

			//get dat acknowledgement
			memset(&ack, 0, sizeof(ack));
			if((val = recvfrom(sock, (void *) &ack, FILESIZE, 0, (struct sockaddr *) &client, &clientlen)) < 0){
				perror("Error in recieving file\n");
				exit(EXIT_FAILURE);
			}
			ack.blockNr = htons(nextBlock);	
			ack.opC = ntohs(4);
			ack.blockNr = ntohs(ack.blockNr);
			printf("%d - %d - %d - %d - %d\n", ack.opC, ack.blockNr, nextBlock,ntohs(ack.opC),ntohs(ack.blockNr));
			if(ntohs(ack.opC) == 4 && ack.blockNr == nextBlock){
				printf("im in this ackackack\n");
				fflush(stdout);
				nextBlock++;
				isReading = true;
				printf("%d", fDataRead);
				if(fDataRead < FILESIZE){
					printf("bout to BREAK\n");
					fflush(stdout);
					break;
				}
			}
			else if(ack.opC == 5){
				perror("Error Acknowledgement\n");
				break;
			}
		}
		fclose(filep);
		printf("YOU TIGHT AF\n");
		fflush(stdout);
						
	}
			
	return 0;
}


/*void concatPath(char *serverFolder, char *filePath, char fullPath[PATH_MAX]){
	
	strcpy(fullPath, serverFolder);
	strcat(fullPath, "/");
	strcat(fullPath, filePath);

}*/

