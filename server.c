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

#define BUFLEN 1500
#define PORT 8080

char *getFileName(char buf[]);

char response[] = "HTTP/1.1 200 OK\r\n"
"Content-Type: text/html; charset=UTF-8\r\n\r\n"
"<doctype !html><html><head><title>My Webpage</title>"
"<style>body { background-color: #111 }"
"h1 { font-size:4cm; text-align: center; color: black;"
" text-shadow: 0 0 2mm white}</style></head>"
"<body><h1>Welcome!</h1></body></html>\r\n";

int main(){

	int wait = 1;
	int fd = socket(AF_INET, SOCK_STREAM, 0);

	if (fd == -1) {
		// an error occurred
	}	

	struct sockaddr_in addr;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	
	if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
		// an error occurred
	}

	int backlog = 10;	
	
	if (listen(fd, backlog) == -1) {
		// an error occurred
	}

	int connfd;

	struct sockaddr_in cliaddr;
	socklen_t cliaddrlen = sizeof(cliaddr);

	while(wait){		
	
		printf("Server: Waiting to accept\n");
		connfd = accept(fd, (struct sockaddr *) &cliaddr, &cliaddrlen);
		
		if (connfd == -1) {
			// an error occurred
		} 

		printf("Server: Accepted!\n");

		ssize_t rcount;
		char buf[BUFLEN+1];
		char *filename = NULL;

		rcount = read(connfd, buf, BUFLEN);
		if (rcount == -1) {
			// An error has occurred...
		}
		buf[rcount]='\0';
		
		filename = getFileName(buf);

		printf("GET file: %s\n", filename);
	
		if ((write(connfd, response, sizeof(response)-1) == -1)) {
			// Error has occurred
			printf("Cant write");
		}
			
		close(connfd);
//		printf("wrote\n");
		}
	
	printf("Closing\n");
	close(fd);
	return 0;

}

char *getFileName(char buf[]){
	
	char *pt;
	char *filename;

	pt = strtok(buf, " ");

	while(pt != NULL){
		
		if(*pt == '/'){
			filename = strtok(pt, " ");			
			return filename;
		}
		
		pt = strtok(NULL, " ");	
	}

	return NULL;

}
