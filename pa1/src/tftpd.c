#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>

//Define operation code for packets
#define RRQ_OPC 1
#define WRQ_OPC 2
#define DATA_OPC 3
#define ACK_OPC 4
#define ERR_OPC 5

//Define size of file to be transmitted everytime
#define FILESIZE 512

//Define Error codes
#define ERROR_NOT_DEFINED 0
#define ERROR_FILE_NOT_FOUND 1
#define ERROR_ACCESS_VIOLATION 2
#define ERROR_DISK_FULL 3
#define ERROR_ILLEGAL_TFTP_OP 4
#define ERROR_UNKNOWN_TRANSFER_ID 5
#define ERROR_FILE_EXISTS 6
#define ERROR_NO_SUCH_USER 7

//The structure of the TFTP packets
struct RRQ{
	short opCode;
	char fileName[FILESIZE];
	char mode[FILESIZE];
};
struct DATA{
	short opCode;
	short blockNr;
	char databuf[FILESIZE];
};
struct ERROR{
	short opCode;
	short errCode;
	char errMsg[FILESIZE];
};
struct ACK{
	short opCode;
	short blockNr;
};

void getOpCode(struct RRQ *clientRequest, struct ERROR *errorblock, int sock, struct sockaddr_in *client, socklen_t clength, int val);

int main(int argc, char *argv[])
{
	//Initialize and declare variables
	int sock;
	struct sockaddr_in server, client;
	int portNumber;
	struct DATA data;
	struct ACK ack;
	struct RRQ request;
	struct ERROR err;
	int nextBlock = 0;
	char buf[PATH_MAX + 1];
	char *dir = argv[2];
	char *res = realpath(dir, buf);
	char fullPath[PATH_MAX];
	bool transfering = true;
	FILE *filep = NULL;

	//Check if the input from client is valid
	if(argc != 3){
	    printf("Invalid input! \n");
		exit(EXIT_FAILURE);
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
	//Get long integer, convert to Network byte order
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	//Set port number for server
	server.sin_port = htons(portNumber);
	
	//Bind Socket to socket address, exit if fail
	if(bind(sock, (struct sockaddr *) &server, sizeof(struct sockaddr_in)) < 0){
	    	perror("Bind error");
			exit(EXIT_FAILURE);
	}

	while(true){
		ssize_t val;
		int fDataRead = 0;
		nextBlock = 1;	
		//Allocate memory for request from client
		memset(&request, 0, sizeof(request));		
		socklen_t clientlen = (socklen_t) sizeof(client);
	       
		//Recieve request from client, throw error if it fails
		if((val = recvfrom(sock, (void *) &request, sizeof(request), 0, (struct sockaddr *) &client, &clientlen)) < 0){
			err.opCode = htons(ERR_OPC);
			err.errCode = ERROR_NOT_DEFINED ;
			strcpy(err.errMsg, "Request failed\n");
			if((val = sendto(sock, &err, sizeof(err.errMsg) + 4, 0, (struct sockaddr *) &client, clientlen)) < 0){
				perror("Error packet failed to send\n");
			}
		}
			
		 
		//Check if request from client is valid, get mode if request is valid, otherwise throw error
		getOpCode(&request, &err, sock, &client, clientlen, val);

		//Allocate memory for fullpath to file, concat fullpath with filename of request from client	
		memset(&fullPath, 0, sizeof(fullPath));
		strcpy(fullPath, res);
		strcat(fullPath, "/");
		strcat(fullPath, request.fileName);
	
		if(strstr(fullPath, "/..") != NULL || strstr(fullPath, res) == NULL){
			err.opCode = htons(ERR_OPC);
			err.errCode = ERROR_ACCESS_VIOLATION;
			strcpy(err.errMsg, "You cannot reach this file!\n");
			if((val = sendto(sock, &err, sizeof(err.errMsg) + 4, 0, (struct sockaddr *) &client, clientlen)) < 0){
				perror("Error packet failed to send\n");
			}
			continue;
		}	
			
		//Get Ip address of client, and which port the client is using
		char *clientIP = inet_ntoa(client.sin_addr);
		unsigned short cPort = ntohs(client.sin_port);
		fprintf(stdout, "File '%s' requested from %s:%d\n", request.fileName, clientIP, cPort);
		fflush(stdout);

		//Open the requested file according to what the client requires, throw error if it is "mail"
		if(strcmp(request.mode, "octet") == 0){
			filep = fopen(fullPath, "rb");
		}
		else if(strcmp(request.mode, "netascii") == 0){
			filep = fopen(fullPath, "r");
		}
		else if(strcmp(request.mode, "mail") == 0){
			err.opCode = htons(ERR_OPC);
			err.errCode = ERROR_ILLEGAL_TFTP_OP;
			strcpy(err.errMsg, "Mail mode not supported\n");
			if((val = sendto(sock,  &err, sizeof(err.errMsg) + 4, 0, (struct sockaddr *) &client, clientlen)) < 0){
				perror("Error packet failed to send\n");
			}
		}
		//Check if opening the file was successfull, throw error if not
		if(!filep){
			err.opCode = htons(ERR_OPC);
			strcpy(err.errMsg, "Failed to read file\n"); 
			if((val = sendto(sock, &err, sizeof(err.errMsg) + 4, 0, (struct sockaddr *) &client, clientlen)) < 0){
				perror("Error packet failed to send");
			}
			continue; 
		}
		//Allocate memory for data and acknowledgment packets
		memset(&data, 0, sizeof(data));
		memset(&ack, 0, sizeof(ack));
			
		//Transfer loop
		while(true){
			//Set op code and block number for the data packet
			data.opCode = htons(DATA_OPC);
			data.blockNr = htons(nextBlock);
			//Check if the file is still being read for transfer
			if(transfering){
				//Allocate memory for the pack being transmitted and read into fDataRead
				memset(data.databuf, 0, FILESIZE);
				fDataRead = fread(&data.databuf, 1, FILESIZE, filep);
			}
			transfering = false;
			//Send the data packet, throw error if unsuccessfull
			if((val = sendto(sock, &data, (size_t) fDataRead + 4, 0, (struct sockaddr *) &client, clientlen)) < 0){
				err.opCode = htons(ERR_OPC);
				strcpy(err.errMsg, "Failed sending file");
				if((val = sendto(sock, &err, sizeof(err.errMsg) + 4, 0, (struct sockaddr *) &client, clientlen)) < 0){
					perror("Error packet failed to send");
				}  	
				continue;
			}

			//Recieve acknowledgment packet, throw error if unsuccessfull
			memset(&ack, 0, sizeof(ack));
			if((val = recvfrom(sock, (void *) &ack, FILESIZE, 0, (struct sockaddr *) &client, &clientlen)) < 0){
				err.opCode = htons(ERR_OPC);
				strcpy(err.errMsg, "Acknowledgement error\n");
				if((val = sendto(sock, &err, sizeof(err.errMsg) + 4, 0, (struct sockaddr *) &client, clientlen)) < 0){
					perror("Error packet failed to send");
				}
			}
			//Set the blocknumber of the acknowledgment to the next block
			ack.opCode = ntohs(ACK_OPC);
			ack.blockNr = htons(nextBlock);
			ack.blockNr = ntohs(ack.blockNr);
			//Check if the blocknumber of the acknowledgment packet is equal to the next block, if successfull, increment nextBlock
			if(ntohs(ack.opCode) == ACK_OPC && ack.blockNr == nextBlock){
				nextBlock++;
				transfering = true;
				//Check if server has reached last block of byte to transfer
				if(fDataRead < FILESIZE){
					break;
				}
			}
			//Otherwise throw acknowledgment error
			else if(ntohs(ack.opCode) != ACK_OPC || ack.blockNr != nextBlock){
				err.opCode = htons(ERR_OPC);
				strcpy(err.errMsg, "Acknowledgement error\n");
				if((val = sendto(sock, (void *) &err, sizeof(err.errMsg) + 4, 0, (struct sockaddr *) &client, clientlen)) < 0){
					perror("Error packet failed to send");
				}
			}
		}
		//Close file, transfer was successfull
		fclose(filep);
		printf("File sent to %s:%d!\n", clientIP, cPort);
		fflush(stdout);
						
	}
			
	return 0;
}
void getOpCode(struct RRQ *clientRequest, struct ERROR *errorblock, int sock, struct sockaddr_in *client, socklen_t clength, int val){
	if(ntohs(clientRequest->opCode) == RRQ_OPC){
		strncpy(clientRequest->mode, strchr(clientRequest->fileName, '\0') + 1, sizeof(clientRequest->mode));
	}
	else if(ntohs(clientRequest->opCode) != RRQ_OPC){
		errorblock->opCode = htons(ERR_OPC);
		errorblock->errCode = ERROR_ILLEGAL_TFTP_OP;
		strcpy(errorblock->errMsg, "Illegal TFTP operation\n");
		if((val = sendto(sock, &errorblock, sizeof(errorblock->errMsg) + 4, 0, (struct sockaddr *) &client, clength)) < 0){
			perror("Error packet failed to send\n");
		}
	}
}

