#include "pipe_networking.h"

static void validate(char *entity, char *data, const char* const expected) {
	if (!strcmp(data, expected)) {
		printf("[%s] received expected value\n", entity);
	}
	else {
		fprintf(stderr, "[%s] did not receive expected value\n", entity);
		exit(1);
	}
}

/*=========================
  server_setup
  args:

  creates the WKP (upstream) and opens it, waiting for a
  connection.

  removes the WKP once a connection has been made

  returns the file descriptor for the upstream pipe.
  =========================*/
int server_setup() {
	//The well known pipe the client will connect to
	printf("[SERVER] creating well known pipe\n");
	int wkp = mkfifo(WKP, 0644);
	
	//Whenever client connects, it will read the data that it sends thru
	char data[HANDSHAKE_BUFFER_SIZE];
	printf("[SERVER] awaiting connection\n");
	wkp = open(WKP, O_RDONLY);
	
	//Remove the upstream WKP
	printf("[SERVER] removing well known pipe\n");
	remove(WKP);
	
	return wkp;
}


/*=========================
  server_connect
  args: int from_client

  handles the subserver portion of the 3 way handshake

  returns the file descriptor for the downstream pipe.
  =========================*/
int server_connect(int from_client) {
	char data[HANDSHAKE_BUFFER_SIZE];
	read(from_client, data, HANDSHAKE_BUFFER_SIZE);
	printf("[SUBSERVER %d] got message from client: %s\n", getpid(), data);
	int to_client = open(data, O_WRONLY);

	//Connect to the private FIFO and send an acknowledgement
	//strcpy(data, ACK);
	printf("[SUBSERVER %d] sending acknowledgement to client\n", getpid());
	write(to_client, ACK, strlen(ACK));
	
	//Now, wait for client to send back the acknowledgement
	printf("[SUBSERVER %d] awaiting acknowledgement from client\n", getpid());
	strncpy(data, "", sizeof(data));
	read(from_client, data, sizeof(data));
	
	//Check if ACK and data match
	validate("SUBSERVER", data, ACK);
	printf("[SUBSERVER] connection established\n");
	
	return to_client;
}

/*=========================
  server_handshake
  args: int * to_client

  Performs the server side pipe 3 way handshake.
  Sets *to_client to the file descriptor to the downstream pipe.

  returns the file descriptor for the upstream pipe.
  =========================*/
int server_handshake(int *to_client) {
  int from_client;

  char buffer[HANDSHAKE_BUFFER_SIZE];

  mkfifo(WKP, 0600);

  //block on open, recieve mesage
  printf("[server] handshake: making wkp\n");
  from_client = open( WKP, O_RDONLY, 0);
  read(from_client, buffer, sizeof(buffer));
  printf("[server] handshake: received [%s]\n", buffer);

  remove(WKP);
  printf("[server] handshake: removed wkp\n");

  //connect to client, send message
  *to_client = open(buffer, O_WRONLY, 0);
  write(*to_client, buffer, sizeof(buffer));

  //read for client
  read(from_client, buffer, sizeof(buffer));
  printf("[server] handshake received: %s\n", buffer);

  return from_client;
}

/*=========================
  client_handshake
  args: int * to_server

  Performs the client side pipe 3 way handshake.
  Sets *to_server to the file descriptor for the upstream pipe.

  returns the file descriptor for the downstream pipe.
  =========================*/
int client_handshake(int *to_server) {

  int from_server;
  char buffer[HANDSHAKE_BUFFER_SIZE];

  //send pp name to server
  printf("[client] handshake: connecting to wkp\n");
  *to_server = open( WKP, O_WRONLY, 0);
  if ( *to_server == -1 )
    exit(1);

  //make private pipe
  sprintf(buffer, "%d", getpid() );
  mkfifo(buffer, 0600);

  write(*to_server, buffer, sizeof(buffer));

  //open and wait for connection
  from_server = open(buffer, O_RDONLY, 0);
  read(from_server, buffer, sizeof(buffer));
  /*validate buffer code goes here */
  printf("[client] handshake: received [%s]\n", buffer);

  //remove pp
  sprintf(buffer, "%d", getpid() );
  remove(buffer);
  printf("[client] handshake: removed pp\n");

  //send ACK to server
  write(*to_server, ACK, sizeof(buffer));

  return from_server;
}
