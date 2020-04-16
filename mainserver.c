/*
Author Sajid Choudhry	Feb 2020
Based on Dr.Carey Williamson's code

-Main program-- shell script running version
Master server that interacts through TCP with clients,
and through UDP with the microservices.
It receives the string and transform option from the client,
forwards the message to the correct UDP microserver,
receives the response from the microserver, and forwards that
response back to the TCP client.

Update: This version has full functionality including reentering new sentences.

Usage:
	Run the bash script 'run' in the current directory

References:

Dr.Carey Williamson - University of Calgary
https://stackoverflow.com/questions/2496302/how-can-i-obtain-the-local-tcp-port-and-ip-address-of-my-client-program?noredirect=1&lq=1
https://stackoverflow.com/questions/16645583/how-to-copy-a-char-array-in-c
https://stackoverflow.com/questions/2876024/linux-is-there-a-read-or-recv-from-socket-with-timeout
https://pages.cpsc.ucalgary.ca/~sina.keshvadi1/cpsc441/
*/

/* Include files for C socket programming and stuff */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <signal.h>
#include <string.h>
#include <arpa/inet.h>		//networking
#include <sys/socket.h>		//networking

/* Global manifest constants */
#define MAX_MESSAGE_LENGTH 100
int port = 8080;				//sets the TCP server port
int udpport = 8081;				//set the UDP server port that this UDP dynamic client will connect to
/*
On a server, you can have a TCP and UDP socket listening in on the same port. 
They are different protocols
See second answer on here as to why
https://stackoverflow.com/questions/6437383/can-tcp-and-udp-sockets-use-the-same-port
*/

/* Optional verbose debugging output */
#define DEBUG 1

/* Global variable */
int childsockfd;

/* This is a signal handler to do graceful exit if needed */
void catcher(int sig)
{
	close(childsockfd);
	exit(0);
}

/* Main program for server */
int main()
{
	
/////////////////////
////TCP setup///////
////////////////////
	struct sockaddr_in serverTCP;		 //struct object of type sockaddr_in called server; fill its attributes in later
	char messagein[MAX_MESSAGE_LENGTH];  //will store the message coming in to the server from the client
	char messageout[MAX_MESSAGE_LENGTH]; //will store the message sent out from the server to the client
	char transformin[MAX_MESSAGE_LENGTH];//stores options that TCP client provides
	int parentsockfd;					 //TCP listening socket for initial connection to client
	int pid;
	int mspid;							 //process id for forking microservers
	static struct sigaction act;		 //for weird error handling in TCP setup


	/* Set up a signal handler to catch some weird termination conditions. */
	act.sa_handler = catcher;
	sigfillset(&(act.sa_mask));
	sigaction(SIGPIPE, &act, NULL);

	/* 1a- Initialize server sockaddr structure */
	memset(&serverTCP, 0, sizeof(serverTCP));	  //fill/clear in the memory area the server struct holds, with 0's
	serverTCP.sin_family = AF_INET;				  //server attribute set as IPV4
	serverTCP.sin_port = htons(port);			  //port 8080
	serverTCP.sin_addr.s_addr = htonl(INADDR_ANY); //address is any available address on the interface

	/* 1b- set up the transport-level end point to use TCP ,aka listening socket*/
	if ((parentsockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) //PF_INET same as AF_INET; IPv4, TCP socket type, 0 is TCP protocol
	{
		fprintf(stderr, "master server: socket() call failed!\n");
		exit(1);
	}

	/* 2- bind a specific address and port to the end point */
	if (bind(parentsockfd, (struct sockaddr *)&serverTCP, sizeof(struct sockaddr_in)) == -1)
	{
		fprintf(stderr, "master server: bind() call failed!\n");
		exit(1);
	}

	/*3- start listening for incoming connections from clients */
	if (listen(parentsockfd, 5) == -1)
	{
		fprintf(stderr, "master server: listen() call failed!\n");
		exit(1);
	}

	/* initialize message strings just to be safe (null-terminated) */
	bzero(messagein, MAX_MESSAGE_LENGTH);
	bzero(messageout, MAX_MESSAGE_LENGTH);

	fprintf(stderr, "Master server started!\n");
	fprintf(stderr, "Server listening on TCP port %d...\n\n", port);
/////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////
////UDP SETUP////////
/////////////////////

	/*1A- UDP clients initial variables*/
	#define SERVER_IP "127.0.0.1" 		/* loopback interface */ //; ip of master server to connect to
	struct sockaddr_in si_server;		//server struct
	struct sockaddr *server;			//ease of use pointer
	int s; 								//listening socketid for server responses
	int len = sizeof(si_server);
	char buf[MAX_MESSAGE_LENGTH];		//sends to and receives from UDP microservices
	int readBytes; 						//# of bytes received from microservices response

	//1B- set up server structures attributes
	memset((char *)&si_server, 0, sizeof(si_server));
	si_server.sin_family = AF_INET;							//IPv4
	si_server.sin_port = htons(port);						//port that receives UDP responses
	server = (struct sockaddr *)&si_server; 				//eas of use variable pointing to struct


	/*
	1C- convert and copy SERVER_IP into si_server struct attribute;
	Can now start UDP sockets on master server for sending 
	and receiving (later in loop) to microservers
	*/
	if (inet_pton(AF_INET, SERVER_IP, &si_server.sin_addr) == 0)
	{
		printf("inet_pton() failed\n");
		return 1;
	}
/////////////////////////////////////////////////////////////////////////////////////////////



//////////////////////////////
////TCP and UDP Communication/
//////////////////////////////

	/* Main loop: master server loops forever listening for client requests */
	for (;;)
	{
		/* TCP-accept a connection from client;
		Store client port and IP info in childsockfd;
		Can now use childsockfd to TCP send-rec between server and client */
		if ((childsockfd = accept(parentsockfd, NULL, NULL)) == -1)
		{
			fprintf(stderr, "master server: accept() call failed!\n");
			exit(1);
		}

		/* try to create a child process to deal with this new client;
		done once the client sucessfully connects to the TCP server;
		parent listens again, child handles the connected client*/
		pid = fork();

		/* use process id (pid) returned by fork to decide what to do next;
		In parent process pid, continue checking for clients;
		In child process pid, communicate with microservices */
		/*error check against pid*/
		if (pid < 0)
		{
			fprintf(stderr, "master server: fork() call failed!\n");
			exit(1);
		}
		else if (pid == 0)
		{
			/* the child process is the one doing the "then" part */


			/* don't need the parent listener socket that was inherited;
			but the parent process will still have it and listen for new clients */
			close(parentsockfd);


			/*receive option selection from main client*/
			char selin[MAX_MESSAGE_LENGTH];
			while( (recv(childsockfd, selin, MAX_MESSAGE_LENGTH,0)) > 0 )
			{
								
								//client chose to enter a sentence
								if(selin[0] == '1')
								{
									//clear old buffers
									bzero(messagein, MAX_MESSAGE_LENGTH);									
									bzero(selin, MAX_MESSAGE_LENGTH);

									//receive sentence from client; store in messagei
									printf("Child process gets ready for sentence from client...\n");
									recv(childsockfd, messagein, MAX_MESSAGE_LENGTH,0);
									strncpy(buf, messagein, MAX_MESSAGE_LENGTH);							//make a copy of sentence to maintain original sentence
									printf("Sentence received...\n");
									
									continue;	//read in next option selection from user			
								}
								

								//client chose to transform current message
								if(selin[0] == '2')
								{
										//clear option buffer just in case
										bzero(selin, MAX_MESSAGE_LENGTH);




										/* obtain the null-terminated transform
										key message from this client; store in transformin;
										recv is blocking syscall- waits at this
										line until message is recieved from client;
										*/
										recv(childsockfd, transformin, MAX_MESSAGE_LENGTH, 0);		//receiving transform key's
										



												printf("Child process received requested transformation: %s\n", transformin);

												/*perform concatenated or single transformations to messagein*/
												for(int i = 0; i < strlen(transformin); i++)
												{

														
														char *mname;					//exec filepath

														if( transformin[i] == '1')
															{
																/*Identity*/
																mname = "./identity.out";
																
															}
															else if( transformin[i] == '2')
															{
																/*Reverse*/
																mname = "./reverse.out";
																
															}
															else if( transformin[i] == '3')
															{
																/*Upper*/
																mname = "./upper.out";
																
															}
															else if( transformin[i] == '4')
															{
																/*Lower*/
																mname = "./lower.out";
																
															}
															else if( transformin[i] == '5')
															{
																/*Caesar*/
																mname = "./caesar.out";
																
															}
															else if( transformin[i] == '6')
															{
																/*Yours*/
																mname = "./yours.out";
																
															}
															else
															{
																break;
															}

															/*set port (in master server UDP struct)for communiation with microserver*/
															si_server.sin_port = htons(udpport);




															/*start remote microserver in a forked process*/
															mspid = fork();
															if( mspid  == 0)
															{
																	//child
																	
																	char *args[] = {mname, NULL};
																	//execute microserver
																	if (execvp(args[0],args) == -1)			//child terminates this process and is running server now;
																	{
																		printf("\nerror reached\n");
																	}										
															}
															else
															{
																//parent waits for child to finish, otherwise parent will be 'sending' before microserver child is listening
																sleep(1);
															}

																		/*microserver now waiting - Back to parent process*/






															/*create listening socket descriptor id for responses*/
															/*AF_INET: IPv4 protocol/  /SOCK_DRAM: socket type is UDP/   /IPPROTO_UDP: UDP Protocol/*/
															if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
															{
																printf("Could not set up a socket!\n");
																return 1;
															}

															fprintf(stderr, "UDP client online in master server!\n\n");
														


															/*send messagein from master server client  to microserver server;*/
															if (sendto(s, buf, strlen(messagein), 0, server, sizeof(si_server)) == -1)
															{
															printf("sendto failed\n");
															return 1;
															}

															/*clear buffer before receiving message back*/
															bzero(buf, MAX_MESSAGE_LENGTH);

															/*receive message from microservice into buf of master server*/
															if ((readBytes=recvfrom(s, buf, MAX_MESSAGE_LENGTH, 0, server, &len))==-1)
															{
															printf("Read error!\n");
															return -1;
															}

															//proper null-termination of string so it can be used further if needed
															buf[readBytes] = '\0';    

															//print to output in master server
															printf("Answer from microserver received by master server: %s\n", buf);							

													
												}//end for
												
												
												/* create the message that goes from
												master server to TCP client (as an ASCII string) */
												sprintf(messageout, "%s\n", buf);
												printf("Child about to send message to TCP client: %s\n", messageout);
											
												/* send the result message back to the client */
												send(childsockfd, messageout, strlen(messageout), 0);

												/* clear out message strings again to be safe */
												bzero(transformin, MAX_MESSAGE_LENGTH);
												bzero(buf, MAX_MESSAGE_LENGTH);
												bzero(messageout, MAX_MESSAGE_LENGTH);


												//continue looping for user to try further transformations on original string
												strncpy(buf, messagein, MAX_MESSAGE_LENGTH);
												continue;

										
								}//end option 2 if
			} //end while


			/* when client is no longer sending information to us, */
			/* the socket can be closed and the child process terminated */
			close(childsockfd);
			exit(0);
		} /* end of then part for child */






		else
		{
			/* the parent process is the one doing the "else" part */



			fprintf(stderr, "Master Server Created child process %d to handle new client\n", pid);
			fprintf(stderr, "Parent going back to job of listening...\n\n");

			/* parent doesn't need the childsockfd */
			close(childsockfd);
		}
	}//end infinite for
}
