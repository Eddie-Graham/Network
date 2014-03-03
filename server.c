/* Eddie Graham 1101301g, simple C server*/

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
#include <ctype.h>
#include <pthread.h>

#define BUFLEN 1500
#define PORT 8081
#define ACCEPTCONNECTIONS 1
#define MAINTAINCONNECTION 1
#define NO_IN_THREAD_POOL 10

typedef struct threadnode THRNODE;
typedef struct threadpool THRPOOL;
typedef struct params PARAMS;

/*threadnode stores node and its state (0 for availiable, 1 for working)*/
struct threadnode {

	pthread_t thr;
	int state;	

};

/*threadpool stores list of threadnodes*/
struct threadpool {

	THRNODE *list[NO_IN_THREAD_POOL];

};

struct params {

	int connfd;
	THRNODE *thrnode;
};

char *getFileName(char buf[]);
char *getFileBuf(char *filename);
char *getFileType(char *filename);
char *getContent(char *fileType);
int getSizeOfFile(char *ptr);
void writeSuccessfulResponse(char *fileType, char *filename, char *fileBuf, int connfd);
void writeNotFoundResponse(int connfd);
void writeBadRequestResponse(int connfd);
void writeInternalServiceErrorResponse(int connfd);
char *getHostName();
int checkHostHeader(char buf[]);
void *processRequest(void *connfdPtr);
THRPOOL *create_thrpool();
THRNODE *get_thr_for_work(THRPOOL *pool);

pthread_mutex_t my_mutex = PTHREAD_MUTEX_INITIALIZER;

int main(){

	THRPOOL *pool;
	pool = create_thrpool();

	int fd = socket(AF_INET, SOCK_STREAM, 0);

	if (fd == -1) {
		// an error occurred
		perror("Error: File Descriptor creation failed");
	}	

	struct sockaddr_in addr;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	
	if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
		// an error occurred
		perror("Error: Bind failure");
	}

	int backlog = 10;	
	
	if (listen(fd, backlog) == -1) {
		// an error occurred
		perror("Error: Listen on File Descriptor failure");
	}

	int connfd;

	struct sockaddr_in cliaddr;
	socklen_t cliaddrlen = sizeof(cliaddr);

	while(ACCEPTCONNECTIONS){	//handles multiple connections

		int i;
	
		printf("Server: Waiting to accept\n");

		connfd = accept(fd, (struct sockaddr *) &cliaddr, &cliaddrlen);
		
		if (connfd == -1) {
			// an error occurred
			perror("Error: Accept connection failure");
		} 

		printf("Server: Accepted!\n");

		THRNODE *thrnode;		

		pthread_mutex_lock(&my_mutex);
		thrnode = get_thr_for_work(pool);
		pthread_mutex_unlock(&my_mutex);

		pthread_t *thr;
		thr = &(thrnode->thr);

		for(i=0; i<NO_IN_THREAD_POOL;i++){

			int w =pool->list[i]->state;
			printf("1st %d\t", w);
		}	

		PARAMS *param;
		param = (PARAMS *)malloc(sizeof(PARAMS));
		param->connfd = connfd;
		param->thrnode = thrnode;	

		pthread_create(thr, NULL, processRequest, param);

		printf("OUT %lu\n", (unsigned long) pthread_self());

		for(i=0; i<NO_IN_THREAD_POOL;i++){

			int w =pool->list[i]->state;
			printf("2nd %d\t", w);
		}

		printf("\nThread finished\n");
		
		}
	
	
	close(fd);
	return 0;

}

/*process HTTP request via thread*/
void *processRequest(void *param){


	PARAMS *ptr = param;
	int connfd = ptr->connfd;

	printf("IN %lu\n", (unsigned long) pthread_self());

	while(MAINTAINCONNECTION){	//handles multiple requests per connection
		
		char buf[BUFLEN+1];
		ssize_t rcount;
		char *fileBuf = NULL;
		char *filename = NULL;		
		char *fileType = NULL;	
		int hostOk = 0;
		int serviceError = 0;

		rcount = read(connfd, buf, BUFLEN);
		if (rcount == -1) {
			// An error has occurred
			perror("Error: Unable to read");
			serviceError = 1;
		}		
			
		if (rcount == 0){
			printf("Closing\n");	
			close(connfd);
			break;	
		}	

		buf[rcount]='\0';			
		
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

	}

	pthread_mutex_lock(&my_mutex);
	ptr->thrnode->state = 0;
	pthread_mutex_unlock(&my_mutex);
	
	return NULL;
}

/*return filename from buf[]*/
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

/*returns file contents from filename, if file does not exist NULL is returned*/
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

/*returns file type (html, txt, gif etc)*/
char *getFileType(char *filename){

	char *pt;
	char *currentPt;	
	char *tempFilename = calloc(strlen(filename)+1, sizeof(char));
	
	strcpy(tempFilename, filename);	

	pt = strtok(tempFilename, ".");

	while(pt != NULL){
		currentPt = pt;
		pt = strtok(NULL, ".");
	}

	return currentPt;
}

/*returns content type*/
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

/*returns size of file*/
int getSizeOfFile(char *filename){  
	
   	struct stat fs;

	int fd = open(filename, O_RDONLY);
	
	if(fstat(fd, &fs) == -1){
	
	}
   	
   	return fs.st_size;

}

/*writes a successful response to the browser*/
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

	char header4[] = "Connection: keep-alive\r\n\r\n";	//keep alive

	write(connfd, header1, strlen(header1)); 
	write(connfd, contentTypeString, strlen(contentTypeString)); 
	write(connfd, contentLengthString, strlen(contentLengthString)); 
	write(connfd, header4, strlen(header4));	
	write(connfd, fileBuf,fileSize);

}

/*writes a not found response to the browser*/
void writeNotFoundResponse(int connfd){

	char header1[] = "HTTP/1.1 404 Not Found\r\n";

	char header2[] = "Content-Type: text/html\r\n";	

	char header3[] = "Content-Length: 109\r\n";

	char header4[] = "Connection: keep-alive\r\n\r\n";   

	char html[] = "<html><head><title>404 Not Found</title></head><body><p>The requested file cannot be found.</p></body></html>";

	write(connfd, header1, strlen(header1)); 
	write(connfd, header2, strlen(header2)); 
	write(connfd, header3, strlen(header3));	
	write(connfd, header4, strlen(header4));	
	write(connfd, html, strlen(html));

}

/*writes a bad request response to the browser*/
void writeBadRequestResponse(int connfd){	

	char header1[] = "HTTP/1.1 404 Bad Request\r\n";

	char header2[] = "Content-Type: text/html\r\n";	

	char header3[] = "Content-Length: 88\r\n";	

	char header4[] = "Connection: keep-alive\r\n\r\n";

	char html[] = "<html><head><title>404 Bad Request</title></head><body><p>Bad Request.</p></body></html>";

	write(connfd, header1, strlen(header1)); 
	write(connfd, header2, strlen(header2));	
	write(connfd, header3, strlen(header3)); 
	write(connfd, header4, strlen(header4));
	write(connfd, html, strlen(html));

}

/*writes an internal service error response to the browser*/
void writeInternalServiceErrorResponse(int connfd){	

	char header1[] = "HTTP/1.1 500 Internal Service Error\r\n";	

	char header2[] = "Content-Type: text/html\r\n";	

	char header3[] = "Content-Length: 110\r\n";	

	char header4[] = "Connection: keep-alive\r\n\r\n";

	char html[] = "<html><head><title>500 Internal Service Error</title></head><body><p>Internal Service Error.</p></body></html>";

	write(connfd, header1, strlen(header1)); 
	write(connfd, header2, strlen(header2));	
	write(connfd, header3, strlen(header3)); 
	write(connfd, header4, strlen(header4));
	write(connfd, html, strlen(html));

}

/*returns hostname*/
char *getHostName(){	
	
	char *pt;
	char hostname[1024];
	hostname[1023] = '\0';
	gethostname(hostname, 1023);

	int i;

	for(i = 0; hostname[i]; i++){
 		 hostname[i] = tolower(hostname[i]);
	}
		
	pt = hostname;

	return pt;	
}

/*checks hostname of server matches Host: header, returns 1 if true, else 0*/
int checkHostHeader(char buf[]){	

	char *dcsString = (char *)malloc(1000); 

	char *hostName;
	hostName = getHostName();

	strcat(dcsString, hostName);
	strcat(dcsString, ".dcs.gla.ac.uk");

	char *tempbuf = calloc(strlen(buf)+1, sizeof(char));
	char *pt;	
	strcpy(tempbuf, buf);		

	pt = strtok(tempbuf, " \r\n");

	while(pt != NULL){

		if(strcmp(pt, "Host:") == 0){
	
			pt = strtok(NULL, " \r\n");
			pt = strtok(pt, ":");

			if((strcmp(pt, hostName) == 0)|| (strcmp(pt, dcsString) == 0) ){

				return 1;
			}
		}
		pt = strtok(NULL, " \r\n");
	}


	return 0;
}

/*creates threadpool and returns pointed to it*/
THRPOOL *create_thrpool(){

	THRPOOL *pool;
	pool = (THRPOOL *)malloc(sizeof(THRPOOL));
	int i;

	for(i=0; i<NO_IN_THREAD_POOL; i++){

		pool->list[i] = (THRNODE *)malloc(sizeof(THRNODE));
		pool->list[i]->state = 0;
	}
	
	return pool;

}

/*returns pointer to thrnode for work and changes its state to 1(busy)*/
THRNODE *get_thr_for_work(THRPOOL *pool){

	int i;

	while(1){
	for(i=0; i<NO_IN_THREAD_POOL;i++){

		if(pool->list[i]->state == 0){
			pool->list[i]->state = 1;
			return pool->list[i];
		}
	}
}
	
	return NULL;

}
