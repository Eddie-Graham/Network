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
#define PORT 8081
#define WAIT 1
#define RESPONSEBUF 10000

char *getFileName(char buf[]);
char *getFileBuf(char *filename);
long getFileSize(char *filename);
char *getFileType(char *filename);
char *generateSuccessfulResponse(char *fileType);
char *getContent(char *fileType);
int getSizeOfString(char *ptr);

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
		// an error occurred
	}

	int backlog = 10;	
	
	if (listen(fd, backlog) == -1) {
		// an error occurred
	}

	int connfd;

	struct sockaddr_in cliaddr;
	socklen_t cliaddrlen = sizeof(cliaddr);

	while(WAIT){	

//		char successfulResponse[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n";	
		char notFoundResponse[] = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n<html><head><title>404 Not Found</title></head><body><p>The requested file cannot be found.</p></body></html>";
		char badRequestResponse[] = "HTTP/1.1 404 Bad Request\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n<html><head><title>404 Bad Request</title></head><body><p>Bad Request.</p></body></html>";
		char internalServiceError[] = "HTTP/1.1 500 Internal Service Error\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n<html><head><title>500 Internal Service Error</title></head><body><p>Internal Service Error.</p></body></html>";

		char *response = NULL;
		long size;
		int serviceError = 0;
	
		printf("Server: Waiting to accept\n");
		connfd = accept(fd, (struct sockaddr *) &cliaddr, &cliaddrlen);
		
		if (connfd == -1) {
			// an error occurred
			response = internalServiceError;
			size = sizeof(internalServiceError);
			serviceError = 1;
		} 

		printf("Server: Accepted!\n");

		ssize_t rcount;
		char buf[BUFLEN+1];

		char *fileBuf = NULL;
		char *filename = NULL;
		long fileSize = (long) NULL;
		char *fileType = NULL;

		rcount = read(connfd, buf, BUFLEN);
		if (rcount == -1) {
			// An error has occurred
			response = internalServiceError;
			size = sizeof(internalServiceError);
			serviceError = 1;
		}
		buf[rcount]='\0';
		
		if(!serviceError){		
		
			filename = getFileName(buf);			
		
			if(filename){

				printf("File: %s\n", filename);		
				fileBuf = getFileBuf(filename);			

				if(fileBuf){					
					
					fileType = getFileType(filename);					
					fileSize = getFileSize(filename);				
					response = generateSuccessfulResponse(fileType);
					int strSize = 0;
					strSize = getSizeOfString(response);
					strcat(response, fileBuf);
					size = strSize+fileSize;
					printf("FileType: %s\n", fileType);
//					printf("%s\n", response);

				}
				else{ 

					response = notFoundResponse;
					size = sizeof(notFoundResponse);
				}
			}	
			else{
				response = badRequestResponse;
				size = sizeof(badRequestResponse);
			}

		}

		
			
		if ((write(connfd, response, size ) == -1)) {
			// Error has occurred
			printf("Cant write\n");
		}
			
		close(connfd);
		}
	
	printf("Closing\n");
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

char *getFileType(char *filename){

	char *pt;	
	char *type;
	char *tempFilename = calloc(strlen(filename)+1, sizeof(char));;
	
	strcpy(tempFilename, filename);	

	pt = strtok(tempFilename, ".");
	pt = strtok(NULL, ".");
	type = pt;

	return type;
}

char *generateSuccessfulResponse(char *fileType){
	

	char *response;
	char *contentType;
	char string[RESPONSEBUF] = "";  // KEY
	char header1[] = "HTTP/1.1 200 OK\r\nContent-Type: ";

	contentType = getContent(fileType);
	char header2[] = "\r\nConnection: close\r\n\r\n";

	strcat(string, header1);
	strcat(string, contentType);
	strcat(string, header2);
	response = string;


	printf("%s\n", response);

	return response;


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

int getSizeOfString(char *ptr){

int size = 0;

while(*ptr != '\0'){
	size ++;
	ptr++;

}

size++;

return size;

}



