/*
Author Sajid Choudhry	Feb 2020
Based on Dr.Carey Williamson's code

Main client.
Main Client that interacts through TCP with Master Server only,
Receives input from user.
It sends the string and transform option to the server,
It receives the final string response back.

Usage:
	Run after you run the mainserver.c master server

  	compile with: gcc mainclient.c -o mainclient
  	run with:	./mainserver
*/





/* Include files for C socket programming and stuff */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>    //networking
#include <string.h>

/* Some generic error handling stuff */
extern int errno;
void perror(const char *s);

/* Manifest constants used by client program */
#define MAX_HOSTNAME_LENGTH 64
#define MAX_WORD_LENGTH 100   /*max length of messages sent over the network*/
#define BYNAME 1
#define MYPORTNUM 8080        /* must match the server's port! */  /*for server struct, so you know where to connect to*/

/* Menu selections */
#define ALLDONE 0
#define ENTER 1

/* Prompt the user to enter a word */
void printmenu()
  {
    printf("\n");
    printf("Please choose from the following selections:\n");
    printf("  1 - Enter a Sentence\n");
    printf("  2 - Perform a Transformation\n");
    printf("  0 - Exit program\n");
    printf("Your desired menu selection? ");
  }


/* Main program of client */
int main()
  {
    char c;

    int sockfd;
    struct sockaddr_in server;
    struct hostent *hp;

    char hostname[MAX_HOSTNAME_LENGTH];
    char message[MAX_WORD_LENGTH];        //storesmessage send to TCP master server
    char transformkey[MAX_WORD_LENGTH];   //stores transform key sent to master server
    char messageback[MAX_WORD_LENGTH];    //stores final messages received from master server
    int choice, len, bytes;


/////////////////////
////Initialization///
////////////////////
    /* Initialization of server sockaddr data structure for use by this client */
    memset(&server, 0, sizeof(server));          //clear/0 the memory area
    server.sin_family = AF_INET;                 //IPv4
    server.sin_port = htons(MYPORTNUM);          //set port 8080
    server.sin_addr.s_addr = htonl(INADDR_ANY);  //set to accept any available IP adddress

#ifdef BYNAME
    /* use a resolver to get the IP address for a domain name */
    /* I did my testing using csx1 (136.159.5.25)    -Carey */
    /*strcpy(hostname, "csx1.cpsc.ucalgary.ca");*/
    strcpy(hostname, "localhost");      /*I chose localhost instead of the above line -Sajid*/
    hp = gethostbyname(hostname);       /*convert name to IP address*/
    if (hp == NULL)
      {
          fprintf(stderr, "%s: unknown host\n", hostname);
          exit(1);
      }

    /* copy the IP address into the sockaddr structure */
    bcopy(hp->h_addr, &server.sin_addr, hp->h_length);
#else
    /* hard code the IP address so you don't need hostname resolver */
    server.sin_addr.s_addr = inet_addr("136.159.5.25");
#endif

    /* create the client socket for its transport-level end point 
    sockfd port # is dynamically determined here for the
    client(see man socket() ), it is NOT the same as the 
    CONSTANT PORT number above, that was for use by the server structure
    */
    if( (sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1 )
      {
          fprintf(stderr, "main client: socket() call failed!\n");
          exit(1);
      }


    /* connect the socket to the server's address using TCP 
    sockfd is client socket; 'server' has IP and port of server
    */
    if( connect(sockfd, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) == -1 )
      {
          fprintf(stderr, "main client: connect() call failed!\n");
          perror(NULL);
          exit(1);
      }

    /* Print welcome banner */
    printf("Client Connected!\n\n");
 //////////////////////////////////End Initialization/////////////////////////////////////   



/////////////////////
////Main Loop////////
////////////////////
    /* main loop: read a word, send to server, and print answer received */
    while(1)
    {
        printmenu();

        /*scan terminal input from TCP client for option*/
        scanf("%d", &choice);

        if( choice == 0)    //user chose to exit
        {
            break;
        }

        if( choice == 1 )   //user chose to enter a message
        {
                    /* get rid of newline after the (integer) menu choice given */
                    /*Without this, errors in reading input*/
                    c = getchar();

                    //send server the choice selection
                    char sel[MAX_WORD_LENGTH];
                    sel[0] = '1';
                    send(sockfd,sel,MAX_WORD_LENGTH,0);

                    /* prompt TCP client for the input */
                    printf("Enter your sentence: ");
                    

                    /*terminal message -> message[]*/
                    len = 0;
                    while( (c = getchar()) != '\n' )
                    {
                        message[len] = c;
                        len++;
                    }
                    /* make sure the message is null-terminated in C */
                    message[len] = '\0';  
                    /* send it to the server via the socket */
                    send(sockfd, message, len, 0);
                    //sleep(1);
                    continue;
        }
        if( choice == 2)    //user chose to transform the text
        {
                    /* get rid of newline after the (integer) menu choice given */
                    c = getchar();

                    //send server the choice selection
                    char sel[MAX_WORD_LENGTH];
                    sel[0] = '2';
                    send(sockfd,sel,MAX_WORD_LENGTH,0);

                    /* prompt TCP client for the input */
                    printf("Enter Transformations: ");
                    

                    /*terminal message -> transformkey[]*/
                    len = 0;
                    while( (c = getchar()) != '\n' )
                    {
                        transformkey[len] = c;
                        len++;
                    }
                    /* make sure the message is null-terminated in C */
                    transformkey[len] = '\0';

                    /* send it to the server via the socket */
                    send(sockfd, transformkey, len, 0);

                    /* see what the server sends back; MAX_WORD_LENGTH is max # of bytes/chars that can be received */
                    if( (bytes = recv(sockfd, messageback, MAX_WORD_LENGTH, 0)) > 0 )
                    {

                            /* make sure the message is null-terminated in C */
                            messageback[bytes] = '\0';
                            
                            /*Print output to client terminal*/
                            printf("~~~~~\nAnswer received from server: ");
                            printf("%s~~~~~\n", messageback);   //messageback contains newline before null
                    }
                    else
                    {
                        /* an error condition if the server dies unexpectedly */
                        printf("Sorry, dude. Server failed!\n");
                        close(sockfd);
                        exit(1);
                    }

                    /*Clear buffers for reuse*/
                    bzero(messageback, bytes);
                    bzero(transformkey, len);
        }
        else printf("Invalid menu selection. Please try again.\n");


    }

    /* Program all done, so clean up and exit the client */
    close(sockfd);
    exit(0);
  }
