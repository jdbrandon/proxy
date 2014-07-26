#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

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
char *handle_buf(char*);

/* You won't lose style points for including these long lines in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static const char *accept_hdr = "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n";
static const char *accept_encoding_hdr = "Accept-Encoding: gzip, deflate\r\n";
static const char *conn_hdr = "Connection: close\r\n";
static const char *prox_hdr = "Proxy-Connection: close\r\n";

int main(int argc, char** argv)
{
    int listenport, listenfd, connfd;
    unsigned clientlen;
    req_t request;
    sockaddr_in clientaddr;
    hostent* clientinfo;
    char* haddrp;

    if(argc < 2){
        printf("usage: %s <port number to bind and listen>\n", argv[0]);
        exit(1);
    }
    listenport = atoi(argv[1]);
    listenfd = open_listenfd(listenport);
    if(listenfd == -1){
        fprintf(stderr,"error opening listening socket\n");
        exit(1);
    }
    
    while(1){
        clientlen = sizeof(clientaddr);
        connfd = accept(listenfd, (SA *) &clientaddr, &clientlen); 
        clientinfo = gethostbyaddr((const char*)&clientaddr.sin_addr.s_addr,
                                   sizeof(clientaddr.sin_addr.s_addr), AF_INET);
        haddrp = inet_ntoa(clientaddr.sin_addr);
        process_request(connfd, &request);
        //response = forward_request(request);
        //forward_response(response);
    }

    return 0;
}

int process_request(int fd, req_t* req){
    size_t n;
    char buf[MAXLINE];
    rio_t rio;
    req = req;
    
    Rio_readinitb(&rio, fd);
    if((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0){
        parse_req(buf, req);
        printf("domain: %s\tpath: %s\n", req->domain, req->path);
    }
    else return -1;
    req->hdrs = NULL;
    while((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0){
        if(strcmp(buf, "\r\n") == 0)
            break;
        if(req->hdrs == NULL){
            req->hdrs = (char*)malloc(strlen(buf)+1);
            if(req->hdrs == NULL){
                fprintf(stderr,"malloc failed\n");
                exit(1); //TODO:may not want to fail here
            }
            strcpy(req->hdrs, handle_buf(buf));
        } else {
            req->hdrs = (char*) realloc(req->hdrs, strlen(req->hdrs)+strlen(buf)+1);
            if(req->hdrs == NULL){
                fprintf(stderr,"realloc failed\n");
                exit(1); //TODO:may not want to fail here
            }
            strcat(req->hdrs, handle_buf(buf));
        }
    }
    printf("hdrs: %s\n", req->hdrs);
    free(req->hdrs);//TODO: move this to after request has been forwarded
    return 0;
}

void parse_req(char* buf, req_t* req){
    char* save;
    strtok_r(buf, " ", &save);			//GET
    strtok_r(NULL, "//", &save); 		//http:
    req->domain = strtok_r(NULL, "/", &save); 	//domain
    req->path = strtok_r(NULL, " ", &save);	//path
}

char *handle_buf(char* buf){
    char* cp, * head;
    size_t size;
    size = strlen(buf) > strlen(user_agent_hdr) ? strlen(buf) : strlen(user_agent_hdr);
    cp = (char*) malloc(size+1);
    if(cp == NULL){
        fprintf(stderr, "malloc failed\n");
    }
    head = (char*) malloc(51);
    if(head == NULL){
        fprintf(stderr, "malloc failed\n");
    }
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
    free(head);
    free(cp);
    return buf;
}
