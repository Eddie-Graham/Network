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

#define BUFLEN 1500
#define PORT 8081
#define ACCEPTCONNECTIONS 1
#define RESPONSEBUF 10000

char *getFileName(char buf[]);
char *getFileBuf(char *filename);

char *getFileType(char *filename);
char *generateSuccessfulResponse(char *fileType, int size);
char *getContent(char *fileType);
int getSizeOfFile(char *ptr);
void writeSuccessful(char *fileType, int size, char *fileBuf, int connfd);

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
		printf("%s\n", "bind failed");
	}

	int backlog = 10;	
	
	if (listen(fd, backlog) == -1) {
		// an error occurred
	}

	int connfd;

	struct sockaddr_in cliaddr;
	socklen_t cliaddrlen = sizeof(cliaddr);

	while(ACCEPTCONNECTIONS){	

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

//		while(1){

		
			char buf[BUFLEN+1];
			ssize_t rcount;
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
			if (rcount == 0)
				break;

		

			buf[rcount]='\0';
		
			if(!serviceError){		
		
				filename = getFileName(buf);			
			
				if(filename){

					printf("File: %s\n", filename);		
					fileBuf = getFileBuf(filename);			

					if(fileBuf){					
						
						fileType = getFileType(filename);					
						fileSize = getSizeOfFile(filename);			
						writeSuccessful(fileType, fileSize, fileBuf, connfd);
//						strcat(response, fileBuf);
						size = fileSize;
//						size = sizeof(response);
						printf("size %d\n", fileSize);

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

		
			if ((write(connfd, response, size) == -1)) {
				// Error has occurred
				printf("Cant write\n");
			}
//			if ((write(connfd, fileBuf, size ) == -1)) {
				// Error has occurred
//				printf("Cant write\n");
//			}

//			write(connfd, "HTTP/1.1 200 OK\n", 16); 
//			write(connfd, "Content-length: 68428\n", 21); // for eddie.jpg
//			write(connfd, "Content-Type: image/jpeg\n\n", 26); 
//			write(connfd, fileBuf,size);

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
	char *type;
	char *tempFilename = calloc(strlen(filename)+1, sizeof(char));;
	
	strcpy(tempFilename, filename);	

	pt = strtok(tempFilename, ".");
	pt = strtok(NULL, ".");
	type = pt;

	return type;
}

char *generateSuccessfulResponse(char *fileType, int size){
	
	
	char *response;
	char *contentType;
	char string[RESPONSEBUF] = "";  // KEY
	char header1[] = "HTTP/1.1 200 OK\r\nContent-Type: ";

	contentType = getContent(fileType);

	char header2[] = "\r\nConnection: close";

	char header3[] = "\r\nContent-Length: ";

	char text[11];
   	
   	sprintf(text, "%d", size);

	char end[] = "\r\n\r\n";


	strcat(string, header1);
	strcat(string, contentType);
	strcat(string, header3);
	strcat(string, text);
	strcat(string, header2);
	strcat(string, end);
	response = string;


	printf("\n%s\n", response);

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

int getSizeOfFile(char *filename){  
	
   	struct stat fs;

        int fd = open(filename, O_RDONLY);
	
	if(fstat(fd, &fs) == -1){
	
	}
   	
   	return fs.st_size;

}

/*void writeSuccessful(char *fileType, int size, char *fileBuf, int connfd){*/

/*	*/
/*	char *contentType;*/

/*	char header1[] = "HTTP/1.1 200 OK\r\n";*/

/*	char header2[] = "Content-Type: ";*/

/*	contentType = getContent(fileType);	*/

/*	char header3[] = "\r\nContent-Length: ";*/

/*	char text[11];*/
/*   	*/
/*   	sprintf(text, "%d", size);*/

/*	char header4[] = "\r\nConnection: close";*/

/*	char end[] = "\r\n\r\n";*/

/*		write(connfd, header1, sizeof(header1)); */
/*		write(connfd, "Content-length: 68428\r\n", 23); // for eddie.jpg*/
/*		write(connfd, "Content-Type: image/jpeg\r\n\r\n", 28); */
/*		write(connfd, fileBuf,size);*/





/*}*/

void writeSuccessful(char *fileType, int fileSize, char *fileBuf, int connfd){

	
	char *contentString = (char *)malloc(300);  
	char *contentType;

	char *contentLength = (char *)malloc(300); 

	char header1[] = "HTTP/1.1 200 OK\r\n";

	char header2[] = "Content-Type: ";	

	contentType = getContent(fileType);	

	strcat(contentString, header2);
	strcat(contentString, contentType);
	strcat(contentString, "\r\n");

	char header3[] = "Content-Length: ";

	char text[11];
   	
   	sprintf(text, "%d", fileSize);	

	strcat(contentLength, header3);
	strcat(contentLength, text);
	strcat(contentLength, "\r\n\r\n");

	char header4[] = "Connection: close\r\n\r\n";

	write(connfd, header1, sizeof(header1)); 
	write(connfd, contentString, strlen(contentString)+1); 
	write(connfd, contentLength, strlen(contentLength)); // for eddie.jpg
//	write(connfd, header4, sizeof(header4) + 1);	
	write(connfd, fileBuf,fileSize);

}





