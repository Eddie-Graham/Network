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
#include <sys/stat.h>
#include <fcntl.h>
#include <netdb.h>


#define BUFLEN 1500
#define PORT 8081
#define ACCEPTCONNECTIONS 1
#define RESPONSEBUF 10000

char *getFileName(char buf[]);
char *getFileBuf(char *filename);
char *getFileType(char *filename);
char *getContent(char *fileType);
int getSizeOfFile(char *ptr);
void writeSuccessfulResponse(char *fileType, char *filename, char *fileBuf, int connfd);
void writeNotFoundResponse(int connfd);
void writeBadRequestResponse(int connfd);
void writeInternalServiceErrorResponse(int connfd);
char *getHostIp();
int checkHostHeader(char buf[]);

int main(){

	int fd = socket(AF_INET, SOCK_STREAM, 0);

	if (fd == -1) {
		// an error occurred
	}	

	struct sockaddr_in addr;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	
	if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
		printf("%s\n", "Bind failed");
	}

	int backlog = 10;	
	
	if (listen(fd, backlog) == -1) {
		// an error occurred
	}

	int connfd;

	struct sockaddr_in cliaddr;
	socklen_t cliaddrlen = sizeof(cliaddr);

	while(ACCEPTCONNECTIONS){	

		int serviceError = 0;
	
		printf("Server: Waiting to accept\n");
		connfd = accept(fd, (struct sockaddr *) &cliaddr, &cliaddrlen);
		
		if (connfd == -1) {
			// an error occurred
			serviceError = 1;
		} 

		printf("Server: Accepted!\n");

//		while(1){

		
			char buf[BUFLEN+1];
			ssize_t rcount;
			char *fileBuf = NULL;
			char *filename = NULL;		
			char *fileType = NULL;	
			int hostOk = 0;

			printf("%s\n", "before read");

			rcount = read(connfd, buf, BUFLEN);
			if (rcount == -1) {
				// An error has occurred
				serviceError = 1;
			}
			printf("%d\n", rcount);			
				
			if (rcount == 0)
				break;		

			buf[rcount]='\0';

			printf("%s\n", "after read");

//			printf("%d\n", rcount);

			
		
			if(!serviceError){	
	
				filename = getFileName(buf);
				hostOk = checkHostHeader(buf);
		
				if(filename && hostOk){

					printf("File: %s\n", filename);	
					fileType = getFileType(filename);	
					fileBuf = getFileBuf(filename);			

					if(fileBuf){	
												
						writeSuccessfulResponse(fileType, filename, fileBuf, connfd);

					}
					else{ 
						writeNotFoundResponse(connfd);
					}
				}	
				else{
					writeBadRequestResponse(connfd);
				}
			}
			else{
				writeInternalServiceErrorResponse(connfd);
			}

//		}
		printf("Closing\n");	
		close(connfd);
		}
	
	
	close(fd);
	return 0;

}

char *getFileName(char buf[]){
	
	char *pt;
	char *filename;
	char *tempbuf = calloc(strlen(buf)+1, sizeof(char));;
	
	strcpy(tempbuf, buf);	

	pt = strtok(tempbuf, " ");

	if(strcmp(pt, "GET") == 0){

		filename = strtok(NULL, " ");
		filename++;

		pt = strtok(NULL, " \r\n");
		
		if((strcmp(pt, "HTTP/1.1") == 0) || (strcmp(pt, "http/1.1") == 0)){
			return filename;			
		}
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

char *getFileType(char *filename){

	char *pt;
	char *currentPt;	
	char *tempFilename = calloc(strlen(filename)+1, sizeof(char));;
	
	strcpy(tempFilename, filename);	

	pt = strtok(tempFilename, ".");

	while(pt != NULL){
		currentPt = pt;
		pt = strtok(NULL, ".");
	}

	return currentPt;
}

char *getContent(char *fileType){
	
	char *contentType;	

	if((strcmp(fileType, "html") == 0) || (strcmp(fileType, "htm") == 0) )
		contentType = "text/html";

	else if(strcmp(fileType, "txt") == 0)
		contentType = "text/plain";

	else if((strcmp(fileType, "jpg") == 0) || (strcmp(fileType, "jpeg") == 0) )
		contentType = "image/jpeg";
		
	else if(strcmp(fileType, "gif") == 0)
		contentType = "image/gif";	
	
	else contentType = "application/octet-stream";

	return contentType;

}

int getSizeOfFile(char *filename){  
	
   	struct stat fs;

        int fd = open(filename, O_RDONLY);
	
	if(fstat(fd, &fs) == -1){
	
	}
   	
   	return fs.st_size;

}

void writeSuccessfulResponse(char *fileType, char *filename, char *fileBuf, int connfd){

	long fileSize = (long) NULL;
	fileSize = getSizeOfFile(filename);

	char *contentTypeString = (char *)malloc(1000);  
	char *contentType;

	char *contentLengthString = (char *)malloc(1000); 

	char header1[] = "HTTP/1.1 200 OK\r\n";

	char header2[] = "Content-Type: ";	

	contentType = getContent(fileType);	

	strcat(contentTypeString, header2);
	strcat(contentTypeString, contentType);
	strcat(contentTypeString, "\r\n");

	char header3[] = "Content-Length: ";

	char length[11];   	
   	sprintf(length, "%ld", fileSize);	

	strcat(contentLengthString, header3);
	strcat(contentLengthString, length);
	strcat(contentLengthString, "\r\n");

	char header4[] = "Connection: close\r\n\r\n";

	write(connfd, header1, strlen(header1)); 
	write(connfd, contentTypeString, strlen(contentTypeString)); 
	write(connfd, contentLengthString, strlen(contentLengthString)); 
	write(connfd, header4, strlen(header4));	
	write(connfd, fileBuf,fileSize);

}

void writeNotFoundResponse(int connfd){

	char header1[] = "HTTP/1.1 404 Not Found\r\n";

	char header2[] = "Content-Type: text/html\r\n";	

	char header3[] = "Connection: close\r\n\r\n";   // keep- alive strange behaviour here

	char html[] = "<html><head><title>404 Not Found</title></head><body><p>The requested file cannot be found.</p></body></html>";

	write(connfd, header1, strlen(header1)); 
	write(connfd, header2, strlen(header2)); 
	write(connfd, header3, strlen(header3));	
	write(connfd, html, strlen(html));

}

void writeBadRequestResponse(int connfd){	

	char header1[] = "HTTP/1.1 404 Bad Request\r\n";	

	char header2[] = "Connection: close\r\n\r\n";

	char html[] = "<html><head><title>404 Bad Request</title></head><body><p>Bad Request.</p></body></html>";

	write(connfd, header1, strlen(header1)); 
	write(connfd, header2, strlen(header2));	
	write(connfd, html, strlen(html));

}

void writeInternalServiceErrorResponse(int connfd){	

	char header1[] = "HTTP/1.1 500 Internal Service Error\r\n";	

	char header2[] = "Connection: close\r\n\r\n";

	char html[] = "<html><head><title>500 Internal Service Error</title></head><body><p>Internal Service Error.</p></body></html>";

	write(connfd, header1, strlen(header1)); 
	write(connfd, header2, strlen(header2));	
	write(connfd, html, strlen(html));

}

char *getHostIp(){
	
	char *ip;
	struct hostent *he;	

	char hostname[1024];
	hostname[1023] = '\0';
	gethostname(hostname, 1023);

	he = gethostbyname(hostname);
	if (he == NULL) { // do some error checking
    		herror("gethostbyname"); // herror(), NOT perror()
    		exit(1);
	}

	// print information about this host:
//	printf("Official name is: %s\n", he->h_name);
//	printf("IP address: %s\n", inet_ntoa(*(struct in_addr*)he->h_addr));

	ip = inet_ntoa(*(struct in_addr*)he->h_addr);

	return ip;
	
}

int checkHostHeader(char buf[]){
	
	int port = PORT;
	char portStr[50];

	sprintf(portStr, "%d", port);

	char *ip;
	ip = getHostIp();
	
	char *hostAndPort = (char *)malloc(2000);
	
	strcat(hostAndPort, ip);
	strcat(hostAndPort, ":");
	strcat(hostAndPort, portStr);

	char *tempbuf = calloc(strlen(buf)+1, sizeof(char));
	char *pt;
	
	strcpy(tempbuf, buf);	

	pt = strtok(tempbuf, " \r\n");

	while(pt != NULL){

		if(strcmp(pt, "Host:") == 0){
	
			pt = strtok(NULL, " \r\n");

			if(strcmp(pt, hostAndPort) == 0){

				return 1;
			}
		}
		pt = strtok(NULL, " \r\n");
	}


	return 0;

}





