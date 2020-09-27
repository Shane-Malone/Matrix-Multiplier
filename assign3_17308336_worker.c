/* 
	Module: Comp20200 Unix Programming
	Assignment 3
	Author: Shane Malone
	Student Number: 17308336
	Program: Worker
	
	This is the worker program for the assignment
	It takes the IP address of the master as argument or if none given uses 127.0.0.1 (The same computer)
	It receives the size of the matrices from the master along with the number of workers
	Using these it allocates enough memory to store the matrices
	It then receives its portion of A and all of B
	It computes a third matrix C
	It sends matrix C to the master then closes the connection and exits
	
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

void parse(char*, char**);

int main(int argc, char *argv[])
{
	char **msg_parse = malloc(sizeof(char*) * 10);
	int i, j, k;
	int n, portions; //parameters of the matrices
	int mysocket, portno;
	int sum = 0;
	char buffer[256];
	
	struct sockaddr_in serv_addr;
	struct hostent *server;
	
	/*Set up ip address of master*/
	if (argc < 2)
		server = gethostbyname("127.0.0.1");
	else
		server = gethostbyname(argv[1]);

	portno = 32980; //Assume master is listening on port 32980

	/*set up socket for communication*/
	if ((mysocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
		printf("ERROR opening socket\n");
	
	if (server == NULL) {
		fprintf(stderr,"ERROR, no such host\n");
		exit(0);
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(portno);

	if (connect(mysocket,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
		printf("ERROR connecting\n");
	
	printf("Connected to master\n");



	/*read size of matrix and number of workers*/
	bzero(buffer,256);
	if (read(mysocket,buffer, 255) < 0) 
		 printf("ERROR reading from socket\n");
	parse(buffer, msg_parse); //Seperates string from master by spaces into individual strings
	
	n = atoi(msg_parse[0]);
	portions = n/atoi(msg_parse[1]);
	printf("size: %d\nportions of A: %d\n", n, portions);
	
	/*create space for matrices*/
	int **matrix_A = (int **)malloc(portions * sizeof(int *)); //create matrix A on the heap
    	for (i=0; i<portions; i++)
        	matrix_A[i] = (int *)malloc(n * sizeof(int));
        
        int **matrix_B = (int **)malloc(n * sizeof(int *)); //create matrix B on the heap
    	for (i=0; i<n; i++)
        	matrix_B[i] = (int *)malloc(n * sizeof(int));
	
	long int **matrix_C = (long int **)malloc(portions * sizeof(long int *)); //create matrix C on the heap
    	for (i=0; i<portions; i++)
        	matrix_C[i] = (long int *)malloc(n * sizeof(long int));

	/*read n/p slices of matrix A*/
	for(j=0; j<portions; j++){
		if (read(mysocket , &matrix_A[j][0] , sizeof(int)*n) < 0)
			printf("error reading Matrix A\n");
	}
	
	/*read all of matrix B*/
	for(j=0; j<n; j++){
		if (read(mysocket, &matrix_B[j][0] , sizeof(int)*n) < 0)
				printf("error reading Matrix B\n");
	}
	
	/*compute matrix C*/
	for(j=0; j<portions; j++)
	{
		for(i=0; i<n; i++)
		{
			for(k=0; k<n; k++)
			{
				sum += matrix_A[j][k]*matrix_B[k][i];
			}
			matrix_C[j][i] = sum;
			sum=0;
		}
	}
	
	/*send matrix C to master*/
	for(j=0; j<portions; j++){
		if(write(mysocket, &matrix_C[j][0], sizeof(long int)*n) < 0)
				printf("error sending matrix C\n");
	}
	
	/*free memory for matrices*/
	for(i = 0; i < portions; i++)
		free(matrix_A[i]);

	free(matrix_A);
	for(i = 0; i < portions; i++)
		free(matrix_C[i]);
	
	free(matrix_C);
	for(i = 0; i < n; i++)
		free(matrix_B[i]);
	
	free(matrix_B);
	
	
	
	printf("Sent our %d portions of Matrix C to master\nExiting...\n", portions);

	close(mysocket);
	return 0;
}

void parse(char* input_str, char** arguments) //function to parse input string from master
{
	char* temp = NULL;
	int count=0;
	temp = strtok(input_str, " "); //seperates strings with spaces
  	while(temp != NULL){
  		arguments[count] = temp;
  		strcpy(arguments[count], temp);
		count++;
		temp = strtok(NULL, " ");	
  	}
	
	for(int i=0; arguments[count-1][i]!='\0'; i++){ //removes trailing \n from last argument
		if(arguments[count-1][i] == '\n')
			arguments[count-1][i] = '\0';
	}
}
