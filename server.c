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
char *getFileBuf(char *filename);
long getFileSize(char *filename);

char response[] = "HTTP/1.1 200 OK\r\n"
"Content-Type: text/html; charset=UTF-8\r\n\r\n"
"<doctype !html><html><head><title>My Webpage</title>"
"<style>body { background-color: #111 }"
"h1 { font-size:4cm; text-align: center; color: black;"
" text-shadow: 0 0 2mm white}</style></head>"
"<body><h1>Welcome!</h1></body></html>\r\n";


int main(){
#000000
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

		char *fileBuf = NULL;
		char *filename = NULL;
		long fileSize = (long) NULL;

		rcount = read(connfd, buf, BUFLEN);
		if (rcount == -1) {
			// An error has occurred...
		}
		buf[rcount]='\0';
		
		filename = getFileName(buf);
		fileSize = getFileSize(filename);
		fileBuf = getFileBuf(filename);

		printf("File: %s\n", filename);
		printf("Size: %lu\n", fileSize);
			
		if ((write(connfd, fileBuf, fileSize) == -1)) {
			// Error has occurred
			printf("Cant write\n");
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

	printf("%s\n", buf);

	pt = strtok(buf, " ");

	while(pt != NULL){

//		printf("pt %s\n", pt);

		if(strcmp(pt, "GET")==0){

			filename = strtok(NULL, " ");
			filename++;

			pt = strtok(NULL, " \r\n");
		
			if(strcmp(pt, "HTTP/1.1")==0){
				return filename;			
			}
		}
			
		pt = strtok(NULL, " ");
		
	}
	return NULL;
}

char *getFileBuf(char *filename){

	FILE * pFile;
  	long lSize;
	char * buffer;
  	size_t result;

  	pFile = fopen (filename , "rb" );
 	if (pFile==NULL) { 
		return NULL;
	}

  	// obtain file size:
  	fseek (pFile , 0 , SEEK_END);
  	lSize = ftell (pFile);
  	rewind (pFile);

  	// allocate memory to contain the whole file:
 	buffer = (char*) malloc (sizeof(char)*lSize);
 	if (buffer == NULL) {
		return NULL;
	}

  	// copy the file into the buffer:
 	result = fread (buffer,1,lSize,pFile);
  	if ((unsigned) result != lSize) {}

  	/* the whole file is now loaded in the memory buffer. */

  	// terminate
  	fclose (pFile);
//	free (buffer);
 	return buffer;


}

long getFileSize(char *filename){

	FILE * pFile;
  	long lSize;	

  	pFile = fopen (filename , "rb" );
 	if (pFile==NULL) { 
		return (long) NULL;
	}

  	// obtain file size:
  	fseek (pFile , 0 , SEEK_END);
  	lSize = ftell (pFile);
	fclose (pFile);

	return lSize;
}
