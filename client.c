#include "pipe_networking.h"

int main() {

  int to_server;
  int from_server;
  char buffer[BUFFER_SIZE];

  from_server = client_handshake( &to_server );

  while (1) {
	
    printf("enter data: ");
    fgets(buffer, sizeof(buffer), stdin);
    *strchr(buffer, '\n') = 0;
	if (strlen(buffer) < 1) continue;
    write(to_server, buffer, sizeof(buffer));
    read(from_server, buffer, sizeof(buffer));
    printf("received: [%s]\n", buffer);
	
	/*
	fgets(data, sizeof(data), stdin);
	if (strlen(data) < 1) continue;
	printf("Sending message to server\n");
	write(to_server, data, sizeof(data));
	
	printf("Reading response from server\n");
	read(from_server, data, strlen(data));
	printf("Response:\n%s\n", data);
	*/
  }
}
