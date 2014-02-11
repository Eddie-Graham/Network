#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h> 
#include <netdb.h>

#define BUFLEN 1500




int main(int argc, char *argv[]){

	struct sockaddr_in addr;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(8080);


	int fd = socket(AF_INET, SOCK_STREAM, 0);

	if (fd == -1) {
		// an error occurred
	}

	struct addrinfo hints;
	struct addrinfo	*ai0;
	int i;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_UNSPEC; // Can use either IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; // Want a TCP socket
	if ((i = getaddrinfo(argv[1], "5000", &hints, &ai0)) != 0) {
		printf("Error: unable to lookup IP address: %s", gai_strerror(i));
	}
	// ai0 is a pointer to the head of a linked list of struct addrinfo
	// values containing the possible addresses of the server; interate
	// through the list, trying to connect to each turn, stopping when
	// a connection succeeds:
	struct addrinfo *ai;
	

	for (ai = ai0; ai != NULL; ai = ai->ai_next) {
		fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
			if (fd == -1) {
				// Unable to create socket, try next address in list
				continue;
			}
			if (connect(fd, ai->ai_addr, ai->ai_addrlen) == -1) {
				// couldn’t connect to the address, try next in list
				close(fd);
				continue;
			}
		
		break; // successfully connected
	}

	if (ai == NULL) {
		// Couldn’t connect to any of the addresses, handle failure...
	} else {
		printf("Client: connection successful\n");
	  
        }

	char *data = argv[2];
	int datalen = sizeof(char)*strlen(data);

	if (write(fd, data, datalen) == -1) {
		// Error has occurred
		} 

	
	ssize_t rcount;
	char buf[BUFLEN+1];

	rcount = read(fd, buf, BUFLEN);
	if (rcount == -1) {
		// An error has occurred...
	}
	buf[rcount]='\0';
	printf("From server: %s\n", buf);
	
	
	

	close(fd);
	return 0;


}
