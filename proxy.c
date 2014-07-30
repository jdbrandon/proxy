#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "sbuf.h"
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define NTHREADS 4
#define SBUFSIZE 16


struct req_t {
    char* domain;
    char* path;
    char* hdrs;
};
typedef struct req_t req_t;

typedef struct sockaddr_in sockaddr_in;
typedef struct hostent hostent;

int process_request(int, req_t *);
void parse_req(char*, req_t*);
char *handle_hdr(char*);
void forward_request(int, req_t);
void *thread(void* vargp);

/* You won't lose style points for including these long lines in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static const char *accept_hdr = "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n";
static const char *accept_encoding_hdr = "Accept-Encoding: gzip, deflate\r\n";
static const char *conn_hdr = "Connection: close\r\n";
static const char *prox_hdr = "Proxy-Connection: close\r\n";

sbuf_t sbuf;
sem_t mutex, w;
int num_entries;
cache_obj * cache;


int main(int argc, char** argv)
{
    int i, listenport, listenfd, connfd;
    unsigned clientlen;
    sockaddr_in clientaddr;
    pthread_t tid;
		
    Sem_init(&mutex, 0, 1);
    Sem_init(&w, 0, 1);
    num_entries = 0;
    cache = NULL;


    if(argc < 2){
        printf("usage: %s <port number to bind and listen>\n", argv[0]);
        exit(1);
    }
    
    listenport = atoi(argv[1]);
    sbuf_init(&sbuf, SBUFSIZE);
    listenfd = Open_listenfd(listenport);
    clientlen = sizeof(clientaddr);
  
    for(i = 0; i < NTHREADS; i++) /* prethreading, creating worker threads */
	Pthread_create(&tid, NULL, thread, &clientaddr);
 
    while(1){
        connfd = Accept(listenfd, (SA *) &clientaddr, &clientlen); 
    	sbuf_insert(&sbuf, connfd);	// put in buffer    
	}

    return 0;
}


void *thread(void *vargp){
	// avoid memory leak
	Pthread_detach(pthread_self());
	sockaddr_in clientaddr = *((sockaddr_in *) vargp);
	hostent* clientinfo;
    req_t request;
    char* haddrp;
	
	while(1){
		int connfd = sbuf_remove(&sbuf);
		// switch to Open_clientfd_r?
		printf("fgfdg\n");
		clientinfo = gethostbyaddr((const char*)&clientaddr.sin_addr.s_addr,
                                   sizeof(clientaddr.sin_addr.s_addr), AF_INET);
        printf("fdgdfgdf\n");
		haddrp = inet_ntoa(clientaddr.sin_addr);
        process_request(connfd, &request);
        forward_request(connfd, request);
        Close(connfd);
	
	}	
}


/* Read a request from a connection socket and parse its information into a 
 * req_t struct.
 * Initally based on csapp echo() p.911
 */
int process_request(int fd, req_t* req){
    size_t n;
    char buf[MAXLINE];
    rio_t rio;

    req->domain = NULL;
    req->path = NULL;
    req->hdrs = NULL;

    //Parse domain and path information    
    Rio_readinitb(&rio, fd);
    if((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0){
        parse_req(buf, req);
    }
    else return -1;
    //parse header information
    while((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0){
        if(strcmp(buf, "\r\n") == 0)
            break;
        if(req->hdrs == NULL){
            req->hdrs = Malloc(n+1);
            strcpy(req->hdrs, handle_hdr(buf));
        } else {
            n = strlen(req->hdrs) + strlen(buf) + 1;
            req->hdrs = Realloc(req->hdrs,strlen(req->hdrs)+ n);
            strcat(req->hdrs, handle_hdr(buf));
        }
    }
    return 0;
}

/* Use string library functions to extract domain and path from
 * an HTTP request.
 * TODO: handle malformed HTTP requests.
 */
void parse_req(char* buf, req_t* req){
    char* save, *p;
    strtok_r(buf, " ", &save);			//GET
    strtok_r(NULL, "//", &save); 		//http:
    p = strtok_r(NULL, "/", &save);	 	//domain
    req->domain = Malloc(strlen(p)+1);
    strcpy(req->domain, p);
    p = strtok_r(NULL, " ", &save);		//path
    req->path = Malloc(strlen(p)+1);
    strcpy(req->path, p);
}

/* Parse a header, if it is a header that our proxy defines
 * statically replace the header with our predefined version.
 * Otherwise just add the the header to the existing header list
 */
char *handle_hdr(char* buf){
    char* cp, * head;
    size_t size;
    size = strlen(buf) > strlen(user_agent_hdr) ? strlen(buf) : strlen(user_agent_hdr);
    cp = (char*) Malloc(size+1);
    head = (char*) Malloc(51);
    strcpy(cp, buf);

    strcpy(head, strtok(buf, ":"));
    if(strcmp(head, "User-Agent") == 0)
        strcpy(cp, user_agent_hdr); 
    if(strcmp(head, "Accept") == 0) 
        strcpy(cp, accept_hdr);
    if(strcmp(head, "Accept-Encoding") == 0) 
        strcpy(cp, accept_encoding_hdr);
    if(strcmp(head, "Connection") == 0) 
        strcpy(cp, conn_hdr);
    if(strcmp(head, "Proxy-Connection") == 0) 
        strcpy(cp, prox_hdr);

    strcpy(buf, cp);
    Free(head);
    Free(cp);
    return buf;
}

/* Takes a request and forwards it to its destination server by opening a client
 * connection. Returns the response of the destination server.
 */
void forward_request(int fd, req_t request){
    int server;
    size_t n, total_read;
    char *name, *portstr, http[1024], buf[MAXLINE];
    rio_t rio;

    name = strtok(request.domain, ":");
    portstr = strtok(NULL, ":");
    if(name == NULL) return;
    if(name == NULL) return;
    if(portstr == NULL) portstr = "80";

    // checking the cache is still updating it (age)
    /*P(&w);
    if((entry = in_cache(name, num_entries, cache)) != NULL){
        // is that it?
        V(&w);
        strcpy(entry->buf, buf);
        n = entry->obj_size;
        Rio_writen(fd, buf, n);
    }
    else {
        V(&w);*/
        server = Open_clientfd_r(name, atoi(portstr));
        sprintf(http, "GET /%s HTTP/1.0\r\n", request.path);
        strcat(http, request.hdrs);
        Rio_writen(server, http, strlen(http));
        Rio_writen(server, "\r\n", 2);
        Rio_readinitb(&rio, server);

        total_read = 0;
        while((n = Rio_readlineb(&rio, buf, MAXLINE)) > 0){

            //if(total_read+n > MAX_OBJECT_SIZE){

                // discard buf?
            //  break;  
            //} 

            total_read += n;
            Rio_writen(fd, buf, n);
        }


        // cache update, critical section
        if(total_read <= MAX_OBJECT_SIZE){
            P(&w);
            cache = cache_write(name, buf, num_entries, cache);
            num_entries++;
            //printf("num entries is now %d\n", num_entries);
            V(&w);
        }
    //}

    Free(request.domain);
    Free(request.path);
    Free(request.hdrs);

}
