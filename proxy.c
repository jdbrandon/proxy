#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

struct http_request {
    char* header;
    char* domain;
    char* path;
};
typedef struct http_request req_t;

/* You won't lose style points for including these long lines in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static const char *accept_hdr = "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n";
static const char *accept_encoding_hdr = "Accept-Encoding: gzip, deflate\r\n";

int main(int argc, char** argv)
{
    int listenport;
    req_t request;
    printf("%s%s%s", user_agent_hdr, accept_hdr, accept_encoding_hdr);

    //psuedocode for getting started
    
    if(argc < 2){
        printf("usage: %s <port number to bind and listen>\n", argv[0]);
        exit(1);
    }
    listenport = get_port(argv);
    
    //create socket
    //bind
    //listen
    while(1){
        //accept
        request = process_request();
        response = forward_request(request);
        forward_response(response);
    }

    return 0;
}
