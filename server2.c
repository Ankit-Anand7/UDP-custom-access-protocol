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


#define LENGTH 10 //Length of Subscriber ID

// Reply code definition
#define PAID 0XFFFB
#define NOT_PAID 0XFFF9
#define DOES_NOT_EXIST 0XFFFA
#define TECHNOLOGY_MISMATCH 0XFFFC



// Response packet data structure
//Unsigned variables are used as values are supposed to be positive always
struct ResponsePacket_t {
	uint16_t start_packetID;				//To store start ID of packet
	uint8_t client_ID;						//To store client ID
	uint16_t type;							//To store response code
	uint8_t seg_No;							//To store segment number of packet
	uint8_t length;							//To store length of payload
	uint8_t technology;						//To store type of cellular technology
	unsigned long Source_subscriber_No;		//To store subscriber number 
	uint16_t end_ID;						//To store end ID of packet
};


// Request packet data structure
struct RequestPacket_t{
	uint16_t start_packetID;				//To store start ID of packet
	uint8_t client_ID;						//To store client ID
	uint16_t Acc_Per;						//To store access permission
	uint8_t seg_No;							//To store segment number of packet
	uint8_t length;							//To store length of payload
	uint8_t technology;						//To store type of cellular technology
	unsigned long Source_subscriber_No;		//To store subscriber number 
	uint16_t end_ID;						//To store end ID of packet
};


//Data structure to store retrieved information from DB
struct Subscriber_Database_t {
	unsigned long sub_no;					//To store subscriber number retrieved from database
	uint8_t technology;						//To store technology retrieved from database
	int status;								//To store status of payment/subscription retrieved from database
};


// To print received packet details
void print_packet(struct RequestPacket_t RequestPacket_t ) {
	printf("\nPacket ID \t\t: %#X\n",RequestPacket_t.start_packetID);
	printf("Client ID \t\t: %#X\n",RequestPacket_t.client_ID);
	printf("Access Permission \t: %#X\n",RequestPacket_t.Acc_Per);
	printf("Segment number \t\t: %d \n",RequestPacket_t.seg_No);
	printf("Length of the Packet \t: %d\n",RequestPacket_t.length);
	printf("Technology \t\t: %d \n", RequestPacket_t.technology);
	printf("Subscriber number \t: %lu \n",RequestPacket_t.Source_subscriber_No);
	printf("End of Packet ID \t: %#X \n",RequestPacket_t.end_ID);
}


// Function to generate response packet
struct ResponsePacket_t generateResponsePacket_t(struct RequestPacket_t RequestPacket_t) {
	struct ResponsePacket_t ResponsePacket_t;
	ResponsePacket_t.start_packetID = RequestPacket_t.start_packetID;
	ResponsePacket_t.client_ID = RequestPacket_t.client_ID;
	ResponsePacket_t.seg_No = RequestPacket_t.seg_No;
	ResponsePacket_t.length = RequestPacket_t.length;
	ResponsePacket_t.technology = RequestPacket_t.technology;
	ResponsePacket_t.Source_subscriber_No = RequestPacket_t.Source_subscriber_No;
	ResponsePacket_t.end_ID = RequestPacket_t.end_ID;
	return ResponsePacket_t;
}

// Function to read DB file and store in defined data structure for comparison
// and store it in the Subscriber_Database_t array.
void read_file(struct Subscriber_Database_t Subscriber_Database_t[]) {

	char line_buffer[30];									// Buffer to store each line read from the file
	int i = 0;												// Variable to track the current index in the array
	FILE *file_ptr;											// File pointer to open and read the file
	file_ptr = fopen("Verification_Database.txt", "rt");	// To open the file in read text mode ("rt")

	// To check if the file is opened successfully and return if any error
	if(file_ptr == NULL)
	{
		printf("Error in opening the file\n");
		return;
	}
	// Loop to read each line of the file untill the end of file is reached
	while(fgets(line_buffer, sizeof(line_buffer), file_ptr) != NULL)
	{
		char *token=NULL;											// Pointer to store each token after splitting
		token = strtok(line_buffer," ");							// Using strtok to split the line into different tokens separated by space
		Subscriber_Database_t[i].sub_no =(unsigned) atol(token);    // To extract first token ie Subscriber number and convert it to long unsigned int
		token = strtok(NULL," ");									// To move the token to next line
		Subscriber_Database_t[i].technology = atoi(token);			// To extract second token ie technology and convert it into int
		token = strtok(NULL," ");									// To move the token to next line
		Subscriber_Database_t[i].status = atoi(token);				// To extract third token ie status and convert it into int
		i++;
	}
	fclose(file_ptr);												// To close the file
}


//  To compare subscriber details and match with DB
int check_database(struct Subscriber_Database_t Subscriber_Database_t[],unsigned int sub_no,uint8_t technology) {

	int value = -1;		// To store result status, -1 is not found

	// Loop to compare subscriber number and technology 
	// Returns status as mentioned in DB if both subscriber number and technology are matched
	// Returns 2 to indicate technology mismatch

	for(int j = 0; j < LENGTH;j++) {
		if(Subscriber_Database_t[j].sub_no == sub_no && Subscriber_Database_t[j].technology == technology) {
			return Subscriber_Database_t[j].status;
		}
        else if (Subscriber_Database_t[j].sub_no == sub_no && Subscriber_Database_t[j].technology != technology) return 2;
	}
	return value;
}


int main(int argc, char**argv){
	
    struct RequestPacket_t RequestPacket_t;			// To store request packet
	struct ResponsePacket_t ResponsePacket_t;		// To store response packet
	
    struct Subscriber_Database_t Subscriber_Database_t[LENGTH];  // An array of Subscriber_Database_t to store subscriber data

	//To check CLA, will generate error if CLA is not as required by Program and will exit from the program
    //Here the arguments will be ./"xyz" and port number on which the server will LISTEN

	if (argc != 2)
    {
        fprintf(stderr, "Type: ./server port_number to deploy server\n");
        exit(1);
    }

	read_file(Subscriber_Database_t);											// TO Read data from the file and store it in the Subscriber_Database_t array
    int sockfd,n;																// To store bytes received over network and socket file descriptor
	struct sockaddr_in server_address;											// To store server's address information
	struct sockaddr_storage server_storage;										// To store server's storage information
	socklen_t addr_size;														// To store the size of the server's address structure
	sockfd=socket(AF_INET,SOCK_DGRAM,0);										// To create UDP socket
    bzero(&server_address,sizeof(server_address));								// To set all the bytes in server_address to zero
	server_address.sin_family = AF_INET;										// To set address family to IPV4
	server_address.sin_addr.s_addr=htonl(INADDR_ANY);							// To set server to listen from all network interfaces
	server_address.sin_port=htons(atoi(argv[1]));								// To convert port number to big-endian network byte order
    bind(sockfd,(struct sockaddr *)&server_address,sizeof(server_address));		// To bind the socket to port number given by user in CLA
	addr_size = sizeof server_address;											// To store the size of server_address, to be used in recvfrom() method
	printf("\nServer has been deployed and is running successfully\n");			// To print the status of deployment of server
	
	// Infinite loop to receive and process incoming packets
    for (;;) {
        // To receive the packet over network and store it into RequestPacket_t
		n = recvfrom(sockfd,&RequestPacket_t,sizeof(struct RequestPacket_t),0,(struct sockaddr *)&server_storage, &addr_size);
		printf(" \n ---------Start of a New Packet-------- \n");
		print_packet(RequestPacket_t);

		// To exit program if there are more than 10 segments
		if(RequestPacket_t.seg_No == 11) {
			exit(0);
		}

		// To check if the Subscriber has valid access permission
		if(n > 0 && RequestPacket_t.Acc_Per == 0XFFF8) {

			// To generate response packet
			ResponsePacket_t = generateResponsePacket_t(RequestPacket_t);

			// To check and compare Subsciber data and DB and return and print result accordingly
			int value = check_database(Subscriber_Database_t,RequestPacket_t.Source_subscriber_No,RequestPacket_t.technology);
			if(value == 0) {
				
				ResponsePacket_t.type = NOT_PAID;
				printf("\n.............................\n");
				printf("Subscriber has not subscribed\n");
				printf(".............................\n");
			}
			else if(value == 1) {
				printf("\n......................................\n");
				printf("Subscriber permitted to access network\n");
				printf("......................................\n");
				ResponsePacket_t.type = PAID;
			}

			else if(value == -1) {
                printf("\n.........................\n");
				printf("No such Subscriber exists\n");
				printf(".........................\n");
				ResponsePacket_t.type = DOES_NOT_EXIST;
			}
                        
            else{
                printf("\n...................................\n");                
                printf("Mismatch in Subscriber's technology\n");
				printf("...................................\n");
            	ResponsePacket_t.type = TECHNOLOGY_MISMATCH;
            }                        
			// To transmit response packet to client
			sendto(sockfd,&ResponsePacket_t,sizeof(struct ResponsePacket_t),0,(struct sockaddr *)&server_storage,addr_size);
		}
		n = 0; // Initialized to zero to recvfrom() from next packet
		
	}
}