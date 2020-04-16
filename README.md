# Data-Transformation-Microservers

This is a C-language client-server program that implements several text-data transformation services. The master server operates on sentence-like messages entered by the user, and uses TCP as its transport-layer protocol, for reliable data transfer with the client.


The client operates by connecting to the master server, entering a sentence of one or more words to be used as the source data, and then entering a loop for interaction with the server. Within the loop, the client can specify what data transformations are desired on the original sentence source data, and in what order. These requests may involve one or more data transformations, to be performed in the order specified, as described below. The master server then communicates with the micro-services via UDP to perform the composed data transformations on each word, prior to returning the final result data back to the client via TCP. Additional client commands can be sent to apply new transformations to the same original sentence source data.


### The microservices are described as follows:
1. Identity: The identity transformation does nothing to the data, but merely returns exactly what was received. It is also known as an echo server.

2. Reverse: This transformation reverses the order of the bytes in a message, and returns the result back. For example, the message "dog" would become "god".

3. Upper: This transformation changes all lower-case alphabetic symbols (i.e., a-z) in a message into upper case (i.e., A-Z). Anything that is already upper case remains unchanged, and anything that is not a letter of the alphabet remains unchanged. For example, the message "Canada 4 Russia 3" would become "CANADA 4 RUSSIA 3".

4. Lower: This transformation changes all upper-case alphabetic symbols (i.e., A-Z) in a message into lower case (i.e., a-z). Anything that is already lower case remains unchanged, and anything that is not a letter of the alphabet remains unchanged. For example, the message "Canada 4 Russia 3" would become "canada 4 russia 3".

5. Caesar: This transformation applies a simple Caesar cipher to all alphabetic symbols (i.e., a-zA-Z) in a message. Recall that a Caesar cipher adds a fixed offset to each letter (with wraparound). Please use a fixed offset of 13, and preserve the case of each letter. Anything that is not a letter of the alphabet remains unchanged. For example, the message "I love cats!" would become "V ybir pngf!".


## Usage
* Server: localhost (127.0.0.1) port 8080
* Microserver port: 8081
* OS: Linux Mint

### Option 1
1. Execute the ‘run’ bash script located in the present working directory. This should compile all current files, as well as run the client and master server.

2. In the mainclient terminal:
* input 1 on the keyboard and hit return
    * Now enter a sentence in the command line
* Input 2 -> choose a transformation service (or a concatenation of them) -> hit return
    * You can continuously press 2 to transform text, 1 to enter a new sentence, or press 0 to exit the program

### Option 2
1. Compile as follows
$ gcc mainserver.c -o mainserver.out
$ gcc mainclient.c -o mainclient.out
$ gcc caesar.c -o caesar.out
$ gcc lower.c -o lower.out
$ gcc upper.c -o upper.out
$ gcc yours.c -o yours.out
$ gcc identity.c -o identity.out
$ gcc reverse.c -o reverse.out

2. Run each of the following commands in order, and in different terminal sessions:
$ ./caesar
$ ./lower
$ ./upper
$ ./yours
$ ./identity
$ ./reverse
$ ./mainserver
$ ./mainclient

3. In the mainclient terminal:
* input 1 on the keyboard and hit return
    * Now enter a sentence in the command line
* Input 2 -> choose a transformation service (or a concatenation of them) -> hit return
    * You can continuously press 2 to transform text, 1 to enter a new sentence, or press 0 to exit the program

