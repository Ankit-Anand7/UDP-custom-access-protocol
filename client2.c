/*  
    COEN 233 Programming Assignment #2
    Name : Ankit Anand
    Student ID : W1652456
*/

// Libraries to be included
#include <sys/socket.h>			//For socket related functions and datatypes
#include <netinet/in.h>			//For structures and functions for working with internet addresses
#include <stdio.h>				//Standard i/o functions
#include <stdint.h>				//For fixed width integer types
#include <stdlib.h>				//For standard library functions like atoi,exit,bzero,etc.
#include <unistd.h>				//For timer methods
#include <string.h>				//For various string functions


// Reply code definition
#define PAID 0XFFFB
#define NOT_PAID 0XFFF9
#define DOES_NOT_EXIST 0XFFFA
#define TECHNOLOGY_MISMATCH 0XFFFC


// Response packet data structure
// Unsigned variables are used as values are supposed to be positive always
struct ResponsePacket_t {
	uint16_t start_packetID;				//To store start ID of packet
	uint8_t client_ID;						//To store client ID
	uint16_t type;							//To store response code
	uint8_t seg_No;							//To store segment number of packe
	uint8_t LENGTH;							//To store length of payload
	uint8_t technology;						//To store type of cellular technology
	unsigned long Source_subscriber_Noo;	//To store subscriber numbe
	uint16_t end_ID;						//To store end ID of packet
};

// Request packet data structure
struct RequestPacket_t {
	uint16_t start_packetID;					//To store start ID of packet
	uint8_t client_ID;							//To store client ID
	uint16_t Acc_Per;							//To store access permission
	uint8_t seg_No;								//To store segment number of packet
	uint8_t LENGTH;								//To store length of payload
	uint8_t technology;							//To store type of cellular technology
	unsigned long long Source_subscriber_Noo;	//To store subscriber number 
	uint16_t end_ID;							//To store end ID of packet
};



// To print sent packet details
void print_packet(struct RequestPacket_t RequestPacket_t) {
	printf("\nPacket ID \t\t: %#X\n",RequestPacket_t.start_packetID);
	printf("Client ID \t\t: %#X\n",RequestPacket_t.client_ID);
	printf("Access Permission \t: %#X\n",RequestPacket_t.Acc_Per);
	printf("Segment number \t\t: %d \n",RequestPacket_t.seg_No);
	printf("Length of the packet \t: %d\n",RequestPacket_t.LENGTH);
	printf("Technology \t\t: %d \n", RequestPacket_t.technology);
	printf("Subscriber number \t: %llu \n",RequestPacket_t.Source_subscriber_Noo);
	printf("End of Packet ID \t: %#X \n",RequestPacket_t.end_ID);
}


// Function to initialize request packet
struct RequestPacket_t Initialize () {
	struct RequestPacket_t RequestPacket_t;
	RequestPacket_t.start_packetID = 0XFFFF;
	RequestPacket_t.client_ID = 0XFF;
	RequestPacket_t.Acc_Per = 0XFFF8;
	RequestPacket_t.end_ID = 0XFFFF;
	return RequestPacket_t;

}


// Log function that writes server packet data to a log file
void log_server_packet(const struct ResponsePacket_t* packet) {
    FILE* log_file = fopen("server_packet_log.txt", "a");

	// I have used fprintf() method as it is a good practice to use fprintf() while logging down something
    if (log_file != NULL) {
        fprintf(log_file, "Response Packet:\n");
        fprintf(log_file, "Start Packet ID \t: %#X\n", packet->start_packetID);
        fprintf(log_file, "Client ID \t\t: %#X\n", packet->client_ID);
        fprintf(log_file, "Type \t\t\t: %#X\n", packet->type);
        fprintf(log_file, "Segment Number \t\t: %d\n", packet->seg_No);
        fprintf(log_file, "Length of the packet \t: %d\n", packet->LENGTH);
        fprintf(log_file, "Technology \t\t: %d\n", packet->technology);
        fprintf(log_file, "Subscriber Number \t: %lu\n", packet->Source_subscriber_Noo);
        fprintf(log_file, "End of Packet ID \t: %#X\n", packet->end_ID);
        fprintf(log_file, "---------------------------------------------------\n");
        fclose(log_file);
    }
}

int main(int argc, char**argv){

	//To check CLA, will generate error if CLA is not as required by Program and will exit from the program
    //Here the arguments will be ./"xyz" localhost and port number on which the server will LISTEN
	if (argc != 3)
    {
        printf("Type: ./client server_name port_number(localhost 8080)\n");
        exit(1);
    }


	struct RequestPacket_t RequestPacket_t;			// To store request packet
	struct ResponsePacket_t ResponsePacket_t;		// To store response packet
	char line_buffer[30];							// Buffer to store each line read from the file
	int i = 1;
	FILE *file_ptr;									// File pointer to open and read the file
	int sockfd,n = 0;								// To store bytes to be sent over network and socket file descriptor
	struct sockaddr_in client_address;				// To store client's address information
	socklen_t addr_size;							// To store the size of the client's address structure
	sockfd = socket(AF_INET,SOCK_DGRAM,0);			// To create UDP socket
	struct timeval timeValue;						// To set ack_timer
	timeValue.tv_sec = 3;           				// To set timer to 3seconds
	timeValue.tv_usec = 0;							// To set timer to 0 ms

	// To check if socket creation was successful
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeValue,sizeof(struct timeval));
	int count = 0;
	if(sockfd < 0) {
		printf("Connection has Failed\n");
	}
	bzero(&client_address,sizeof(client_address));			// To set all the bytes in client_address to zero
	client_address.sin_family = AF_INET;					// To set address family to IPV4
	client_address.sin_addr.s_addr = htonl(INADDR_ANY);		// To set server to listen from all network interfaces
	client_address.sin_port=htons(atoi(argv[2]));			// To convert port number to big-endian network byte order
	addr_size = sizeof client_address ;						// To store the size of client_address, to be used in recvfrom() method


	// To initialize request packet
	RequestPacket_t = Initialize();

	// To open the file in read text mode ("rt")
	file_ptr = fopen("Subscriber_data.txt", "rt");

	// To check if the file is opened successfully and return if any error
	if(file_ptr == NULL)
	{
		printf("Error in opening the file\n");
	}

	// Loop to read each line of the file untill the end of file is reached
	while(fgets(line_buffer, sizeof(line_buffer), file_ptr) != NULL) {
		count = 0;																					// To keep track of number of attempts
		n = 0;																						// To store number of bytes received from recvfrom() method
		printf(" \n ---------Start of a New Packet-------- \n");
		char * token;																				// Pointer to store each token after splitting line_buffer
		token = strtok(line_buffer," ");															// Using strtok to split the line into different tokens separated by space
		RequestPacket_t.LENGTH = strlen(token);														// To store the length of each token in length field
		RequestPacket_t.Source_subscriber_Noo = (unsigned long) strtol(token, (char **)NULL, 10);	// To convert first token ie subscriber number and store it in RequestPacket_t
		token = strtok(NULL," ");																	// To move token to next line
		RequestPacket_t.LENGTH += strlen(token);													// To update the 'LENGTH' field by adding the length of the second token (technology) to it
		RequestPacket_t.technology = atoi(token);													// To convert second token ie technology and store it in RequestPacket_t
		token = strtok(NULL," ");																	// To move token to next line
		RequestPacket_t.seg_No = i;																	// To update segment number
		int attempt=1;																				// To keep track of failed response from server

		// Function to print packet details
		print_packet(RequestPacket_t);
		
		// Loop to send packets to server
		// Any packet will be attempted for transmission for three times only 
		// Maximum segments supported by server is 10, implemented in server code

		while(n <= 0 && count < 3) { 

			// Packet transmission to server
			sendto(sockfd,&RequestPacket_t,sizeof(struct RequestPacket_t),0,(struct sockaddr *)&client_address,addr_size);

			// To store received packet in ResponsePacket_t
			n = recvfrom(sockfd,&ResponsePacket_t,sizeof(struct ResponsePacket_t),0,NULL,NULL);

			// If there is no response from server, it will try three times and print the no of attempts
			if(n <= 0 ) {
				printf("No response from server. Ack_timer expired. Attempt: %d\n",attempt++);
				count ++;
			}

			// Upon successfull reception of response from server, all test cases are checked here
			else if(n > 0) {
				printf("Status = ");
				if(ResponsePacket_t.type == NOT_PAID) {
					printf("\n.............................\n");
					printf("Subscriber has not subscribed\n" );
					printf(".............................\n");
				}
				else if(ResponsePacket_t.type == TECHNOLOGY_MISMATCH) {
					printf("\n...................................\n");
					printf("Mismatch in Subscriber's technology\n");
					printf("...................................\n");

				}
				else if(ResponsePacket_t.type == DOES_NOT_EXIST ) {
					printf("\n.........................\n");
					printf("No such Subscriber exists\n");
					printf(".........................\n");
				}
				else if(ResponsePacket_t.type == PAID) {
					printf("\n.....................................\n");
					printf("Subscriber permitted to access network\n");
					printf("......................................\n");

				}
				// To log the received server packet into a text file
            	log_server_packet(&ResponsePacket_t);
				
			}
		}
		// To exit from program in case of no response from server aftre three failed attempts
		if(count >= 3 ) {
			printf("Server is not responding.");
			exit(0);
		}
		i++;			// To update segment number
		
	}
	fclose(file_ptr);	// To close the open file
};