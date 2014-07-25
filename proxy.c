#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

struct req_t {
    char* header;
    char* domain;
    char* path;
};
typedef struct req_t req_t;

typedef struct sockaddr_in sockaddr_in;
typedef struct hostent hostent;

int process_request(int, req_t *);

/* You won't lose style points for including these long lines in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static const char *accept_hdr = "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n";
static const char *accept_encoding_hdr = "Accept-Encoding: gzip, deflate\r\n";

int main(int argc, char** argv)
{
    int listenport, listenfd, connfd;
    unsigned clientlen;
    req_t request;
    sockaddr_in clientaddr;
    hostent* clientinfo;
    char* haddrp;

    user_agent_hdr = user_agent_hdr;
    accept_hdr = accept_hdr;
    accept_encoding_hdr = accept_encoding_hdr;

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
    while((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0){
        printf("received %zd bytes\n", n);
        printf("message: (%s)\n", buf);
    }
    return 0;
}
