/*
Author Sajid Choudhry	Feb 2020
Based on Dr.Carey Williamson's code

Microserver:
  Receives message from master server through UDP;
  Manipulates the text, and sends it back.

*/

/* Include files */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


/* Manifest constants */
#define MAX_BUFFER_SIZE 100  /*max sentence size*/
#define PORT 8081           /*client sends to this port and receives from this port*/
                            /*client has its own dynamic port number*/
#define DEBUG 1             /* Verbose debugging */


/*reverses passed in text*/
void reverse(char array[])
{
        int len = strlen(array);
        char z;

        // (len-1) because we dont want to reverse if i=j (middle element)
        for(int i=0, j=(len-1); i < j; i++,j--)
        {
                z = array[i];           //store left side element
                array[i] = array[j];    //swap last element into first place
                array[j] = z;            //swap temp into last element
        }

}


int main()
{
    struct sockaddr_in si_server, si_client;      //struct objects of type sockaddr_in called si_server, and si_client
                                                    //server will store client IP and port in struct si_client when it receives a message
    struct sockaddr *server, *client;             //pointers for ease of use in methods
    int s;                                        //listening socket id
    int len=sizeof(si_server);              
    char messagein[MAX_BUFFER_SIZE];              //store messages received by client
    char messageout[MAX_BUFFER_SIZE];             //store messages that will be sent to client
    int readBytes;

    //1a- set up listening socket
    //AF_INET: IPv4 protocol, SOCK_DGRAM: socket type UDP, IPPROTO_UDP: use UDP protocol
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
    {
        printf("Could not setup a socket for UDP!\n");
        return 1;
    }
   //1b- Initialize attributes of si_server struct
    memset((char *) &si_server, 0, sizeof(si_server));    //fill in the memory area the si_server struct holds, with 0's
    si_server.sin_family = AF_INET;                 //server attribute set as IPV4
    si_server.sin_port = htons(PORT);               //PORT 
    si_server.sin_addr.s_addr = htonl(INADDR_ANY);
    server = (struct sockaddr *) &si_server;        //set pointers to point to struct si_server, and si_client
    client = (struct sockaddr *) &si_client;          //client storing client IP and port in this struct when connection is made


    //2 bind listening socket (s) port # and IP # (from struct server)
    if (bind(s, server, sizeof(si_server))==-1)
      {
            printf("Could not bind to port %d!\n", PORT);
            return 1;
      }

    fprintf(stderr, "Reverse microserver online!\n");
    printf("Microserver now listening on UDP port %d...\n", PORT);

    /* big loop, looking for incoming messages from clients */      //reloop back here after sending client answer; port remains the same


                /* clear out message buffers to be safe */
                bzero(messagein, MAX_BUFFER_SIZE);
                bzero(messageout, MAX_BUFFER_SIZE);

                /* see what comes in from a client, if anything */
                if ((readBytes=recvfrom(s, messagein, MAX_BUFFER_SIZE, 0, client, &len)) < 0)
                  {
                    printf("Read error!\n");
                    return -1;
                  }
              #ifdef DEBUG
                else printf("Microserver received %d bytes from master server.\n", readBytes);
              #endif

                printf("  server received \"%s\" from IP %s port %d.\n",
                      messagein, inet_ntoa(si_client.sin_addr), ntohs(si_client.sin_port));    ////get client IP and port from client struct

                /*manipulate the message*/
                reverse(messagein);

                /* create the outgoing message (as an ASCII string) */
                sprintf(messageout, "%s", messagein);

              #ifdef DEBUG
                printf("Microserver sending back the message to master: \"%s\"\n\n", messageout);
              #endif

                /* send the result message back to the client */
                sendto(s, messageout, strlen(messageout), 0, client, len);		


                close(s);
                return 0;
  }