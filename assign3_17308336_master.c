/* 
	Module: Comp20200 Unix Programming
	Assignment 3
	Author: Shane Malone
	Student Number: 17308336
	Program: Master
	
	This program is the master process for matrix multiplication
	It takes the size of the matrices and number of workers as arguments and randomly creates 2 matrices A and B
	It accepts connections from each worker
	It sends each worker the size of the matices A and B and how many other workers there are
	It then sends to each worker a portion of A and all of B
	The workers each calculate a portion of matrix C and send these back to this program
	This can then be printed however printing for large matrices is not advisable as it is illegible
	It then closes the connection and exits
	
	Performance:
	Since I declare my matrices using malloc the program can handle matrices of size up to a few thousand
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>

int main(int argc, char *argv[])
{
	int i, j;
	int mysocket, portno;
	socklen_t clilen;
	char buffer[256];
	int worker;
	
	struct sockaddr_in serv_addr, cli_addr; //structure containing server address and client address
	
	if(argc<3){
		printf("Usage error\nNeed Size and Number of workers\n");
		exit(0);
	}	
	srand(time(NULL)); //seed rand function so we get different arrays each time

	int n = atoi(argv[1]); //size of matrix
	int p = atoi(argv[2]); //number of worker processes
	int sockfd[p];
	
	if(n%p != 0){
		printf("cannot divide matrix amongst workers\n");
		exit(0);
	}
	int portions = n/p; //number of horizontal portions to send to each worker
	

	/*populate matrix A and B*/
	int **matrix_A = (int **)malloc(n * sizeof(int *)); //create matrix A on the heap
    	for (i=0; i<n; i++)
        	matrix_A[i] = (int *)malloc(n * sizeof(int));

	for(j=0; j<n; j++)
		for(i=0; i<n; i++)
			matrix_A[j][i] = rand()%10;
	
	int **matrix_B = (int **)malloc(n * sizeof(int *)); //create matrix B on the heap
    	for (i=0; i<n; i++)
        	matrix_B[i] = (int *)malloc(n * sizeof(int));
        	
	for(j=0; j<n; j++)
		for(i=0; i<n; i++)
			matrix_B[j][i] = rand()%10;
	
	long int **matrix_C = (long int **)malloc(n * sizeof(long int *)); //create matrix C on the heap
    	for (i=0; i<n; i++)
        	matrix_C[i] = (long int *)malloc(n * sizeof(long int));



	/*set up socket to listen at port 32980*/
	mysocket = socket(AF_INET, SOCK_STREAM, 0); //create socket for communication

	if (mysocket < 0){
		printf("ERROR opening socket\n");
		exit(1);
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = 32980;

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	     
	if (bind(mysocket, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){ //assigns pointer to socket
		printf("ERROR on binding\n");
		exit(1);
	}
	listen(mysocket, p); //listens for connections
	clilen = sizeof(cli_addr);
	for(i=0; i<p; i++){
		sockfd[i] = accept(mysocket, (struct sockaddr *) &cli_addr, &clilen);
		if (sockfd[i] < 0){
			printf("ERROR on accept\n");
			exit(1);
		}
		printf("Connected worker %d\n", i);
	}
	
	
	
	/*send size of matrices A and B to worker and number of portions*/
	bzero(buffer,256);
	strcpy(buffer, argv[1]);
	strcat(buffer, " ");
	strcat(buffer, argv[2]);
	
	
	
	/*send message to worker*/
	for(worker=0; worker<p; worker++){
		if(write(sockfd[worker], buffer, 255) < 0) //send n and p to worker
			printf("error sending message\n");
		/*send p slices of Matrix A to worker*/
		for(j=0; j<portions; j++){
			if(write(sockfd[worker], &matrix_A[j + worker*portions][0], sizeof(int)*n) < 0) //only sends slices for each process
				printf("error sending matrix A\n");
		}
		/*send all of matrix B to worker*/
		for(i=0; i<n; i++){
			if(write(sockfd[worker], &matrix_B[i][0], sizeof(int)*n) < 0)
				printf("error sending matrix B\n");
		}
		//next worker will be sent next portion
	}
	printf("Sent all matrices\n");



	/*read matrix C*/
	printf("Matrix C:\n");
	worker = 0;
	for(j=0; j<n; j++){
		if(j%portions==0 && j!= 0) //if we have received all we need from a worker
			worker++;
		if (read(sockfd[worker] , &matrix_C[j][0] , sizeof(long int)*n) < 0)
				printf("error reading Matrix C\n");
		for(i=0; i<n; i++){
			printf("%ld ", matrix_C[j][i]);
		}
		printf("\n");
	}
	
	
	/*Close connections with workers*/
	for(i=0; i<p; i++){
		close(sockfd[i]);
		printf("disconnected from worker %d\n", i);
	}
	close(mysocket);
	printf("All connections closed\nExiting...\n");

	/*free memory on heap*/
	for(int i = 0; i < n; i++)
		free(matrix_A[i]);
	free(matrix_A);
	for(int i = 0; i < n; i++)
		free(matrix_C[i]);
	free(matrix_C);
	for(int i = 0; i < n; i++)
		free(matrix_B[i]);
	free(matrix_B);

     return 0; 
}
