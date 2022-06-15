/**
 * client.c
 *
 * @author Andrew Kirrane
 *
 * USD COMP 375: Computer Networks
 * Project 1
 *
 * client.c is a program that mimics the linux client for the sensor server.
 * The components of this file consist of a main loop to deal with user inputs, a
 * connection function which initiates a TCP connection with both servers, a
 * prompt function to display the menu and accept user inputs, and a connect
 * to host function which lays out the groundwork for a socket connection.
 * These functions work cohesively to give a smooth user experience in which
 * they can get real time information on temperature, humidity, and wind
 * speeds captured by the sensor. 
 */

#define _XOPEN_SOURCE 600

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

#define BUFF_SIZE 1024

char buff[BUFF_SIZE]; // buffer to capture information sent back by the servers
char req_buff[BUFF_SIZE]; // buffer to send information from the client side to the servers
long prompt();
int connectToHost(char *hostname, char *port);
void mainLoop();
void connection(char *req_buff);

/**
 * Main function, calls the main loop after printing out the header to introduce the user to the program
 *
 * @return 0 upon completion
 */
int main() {	
	printf("WELCOME TO THE COMP375 SENSOR NETWORK\n\n\n");
	mainLoop();
	return 0;
}

/**
 * Loop to keep asking user what they want to do and calling the appropriate
 * function to handle the selection.
 */
void mainLoop() {
	while (1) {
		long selection = prompt();
		memset(buff, 0, BUFF_SIZE); // reset buffer
		int temp, humidity, wind_speed; // initialize variables for data returned by servers
		long time;
			switch (selection) {
				case 1: // user selects (1)
					strcpy(req_buff, "AIR TEMPERATURE\n"); // keywords for request of air temperature
					connection(req_buff); // call a TCP connection with the requested keywords
					sscanf(buff, "%ld %d F\n", &time, &temp); // save returned data to variables
					printf("\nThe Last AIR TEMPERATURE reading was %d F, taken at %s\n", temp, ctime(&time)); // print out returned data in a readable format		
					memset(buff, 0, BUFF_SIZE); // reset buffer
		 			break;
				case 2: // user selects (2)
					strcpy(req_buff, "RELATIVE HUMIDITY"); // keywrods for request of humidity
					connection(req_buff); // call a TCP connection with the requested keywords
					sscanf(buff, "%ld %d %%\n", &time, &humidity); // save returned data to variables
					printf("\nThe Last RELATIVE HUMIDITY reading was %d %%, taken at %s\n", humidity, ctime(&time)); // print out returned data in a readable format	
					memset(buff, 0, BUFF_SIZE); // reset buffer
		 			break;
				case 3: // user selects (3)
					strcpy(req_buff, "WIND SPEED"); // keywords for request of wind speed
					connection(req_buff); // call a TCP connection with the requested keywords
					sscanf(buff, "%ld %d MPH\n", &time, &wind_speed); // save returned data to variables
					printf("\nThe Last WIND SPEED reading was %d MPH, taken at %s\n", wind_speed, ctime(&time)); // print out returned data in a readable format	
					memset(buff, 0, BUFF_SIZE); // reset buffer
		 			break;
				case 4: // user selects (4)
		 			printf("GOODBYE!\n");
					exit(1); // exit program
				default: // if an invalid option is entered, return error message
		 			fprintf(stderr, "ERROR: Invalid selection\n");
		 			break;
			}
	}
}

/**
 * Facilitates the connection from the client to each of the two
 * servers using the different passwords and port numbers. It saves the
 * requested data to a buffer, then concludes by closing the connections to
 * both servers.
 *
 * @param *req_buff is passed in to supply the keywords needed to communicate
 * with the server based on the user input
 */
void connection(char *req_buff){
	int port_num = 0; // variable to store sensor port number as an integer
	char port[10]; // variable to read sensor port number as a string from server response
	int server_fd = connectToHost("main.esmarttech.com", "47789"); // initiate connection with the first server
	if(send(server_fd, "AUTH password123\n", sizeof("AUTH password123\n"), 0) < 1){ // send login credentials to the first server
		memset(buff, 0, BUFF_SIZE);
		memset(req_buff, 0, BUFF_SIZE);
		exit(1);
	}
	if(recv(server_fd, buff, BUFF_SIZE, 0) < 1){ // recieve response from login
		memset(buff, 0, BUFF_SIZE);
		memset(req_buff, 0, BUFF_SIZE);
		exit(1);
	}
	sscanf(buff, "CONNECT sensor.esmarttech.com %d sensorpass321\n", &port_num); // read in sensor port number from server response
	sprintf(port, "%d", port_num); // convert sensor port number to an integer
	int sensor_fd = connectToHost("sensor.esmarttech.com", port); // initiate connection to sensor
	if(send(sensor_fd, "AUTH sensorpass321\n", sizeof("AUTH sensorpass321\n"), 0) < 1){	// send login credentials to the sensor
		memset(buff, 0, BUFF_SIZE);
		memset(req_buff, 0, BUFF_SIZE);
		exit(1);
	}
	if(recv(sensor_fd, buff, BUFF_SIZE, 0) < 1){ // recieve response from login
		memset(buff, 0, BUFF_SIZE);
		memset(req_buff, 0, BUFF_SIZE);
		exit(1);
	}
	memset(buff, 0, BUFF_SIZE); // reset buffer
	if(send(sensor_fd, req_buff, sizeof(req_buff), 0) < 1){ // send user request to sensor	
		memset(buff, 0, BUFF_SIZE);
		memset(req_buff, 0, BUFF_SIZE);
		exit(1);
	}
	if(recv(sensor_fd, buff, BUFF_SIZE, 0) < 1){ // recieve response about the user request, store in buffer
		memset(buff, 0, BUFF_SIZE);
		memset(req_buff, 0, BUFF_SIZE);
		exit(1);
	}
	close(server_fd); // close connections
	close(sensor_fd);
}


/** 
 * Print command prompt to user and obtain user input.
 *
 * @return The user's desired selection, or -1 if invalid selection.
 */
long prompt() {
	printf("Which sensor would you like to read:\n\n");
	printf("\t(1) Air temperature\n\t(2) Relative humidity\n\t(3) Wind speed\n\t(4) Quit Program\n\n");
	printf("Selection: ");
	// Read in a value from standard input
	char input[10];
	memset(input, 0, 10); // set all characters in input to '\0' (i.e. nul)
	char *read_str = fgets(input, 10, stdin);

	// Check if EOF or an error, exiting the program in both cases.
	if (read_str == NULL) {
		if (feof(stdin)) {
			exit(0);
		}
		else if (ferror(stdin)) {
			perror("fgets");
			exit(1);
		}
	}

	// get rid of newline, if there is one
	char *new_line = strchr(input, '\n');
	if (new_line != NULL) new_line[0] = '\0';

	// convert string to a long int
	char *end;
	long selection = strtol(input, &end, 10);

	if (end == input || *end != '\0') {
		selection = -1;
	}

	return selection;
}

/**
 * Socket implementation of connecting to a host at a specific port.
 *
 * @param hostname The name of the host to connect to (e.g. "foo.sandiego.edu")
 * @param port The port number to connect to
 * @return File descriptor of new socket to use.
 */
int connectToHost(char *hostname, char *port) {
	// Step 1: fill in the address info in preparation for setting 
	//   up the socket
	int status;
	struct addrinfo hints;
	struct addrinfo *servinfo;  // will point to the results

	memset(&hints, 0, sizeof hints); // make sure the struct is empty
	hints.ai_family = AF_INET;       // Use IPv4
	hints.ai_socktype = SOCK_STREAM; // TCP stream sockets

	// get ready to connect
	if ((status = getaddrinfo(hostname, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		exit(1);
	}

	// Step 2: Make a call to socket
	int fd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
	if (fd == -1) {
		perror("socket");
		exit(1);
	}

	// Step 3: connect!
	if (connect(fd, servinfo->ai_addr, servinfo->ai_addrlen) != 0) {
		perror("connect");
		exit(1);
	}

	freeaddrinfo(servinfo); // free's the memory allocated by getaddrinfo

	return fd;
}
