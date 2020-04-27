#include <fnmatch.h>
#include <fcntl.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#define BACKLOG (10)

void serve_request(int,char*);

//various HTTP requests
char * request_str = "HTTP/1.0 200 OK\r\n"
        "Content-type: text/html; charset=UTF-8\r\n\r\n";

char * request_text = "HTTP/1.0 200 OK\r\n"
        "Content-type: text/plain; charset=UTF-8\r\n\r\n";

char * request_png = "HTTP/1.0 200 OK\r\n"
        "Content-type: image/png; charset=UTF-8\r\n\r\n";

char * request_jpeg = "HTTP/1.0 200 OK\r\n"
        "Content-type: image/jpeg; charset=UTF-8\r\n\r\n";

char * request_jpg = "HTTP/1.0 200 OK\r\n"
        "Content-type: image/jpg; charset=UTF-8\r\n\r\n";

char * request_gif = "HTTP/1.0 200 OK\r\n"
        "Content-type: image/gif; charset=UTF-8\r\n\r\n";

char * request_pdf = "HTTP/1.0 200 OK\r\n"
        "Content-type: application/pdf; charset=UTF-8\r\n\r\n";

char * request_404 = "HTTP/1.0 404 NOT FOUND\r\n"
        "Content-type: text/html; charset=UTF-8\r\n\r\n";


char * index_hdr = "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 3.2 Final//EN\"><html>"
        "<title>Directory listing for %s</title>"
"<body>"
"<h2>Directory listing for %s</h2><hr><ul>";

//404 message
char * message_404 = "<!doctype html>"
"<html lang=\"en\">"
"<head>"
"    <meta charset=\"utf-8\">"
"    <title>Page Not Found</title>"
"    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
"</head>"
"<body>"
"    <h1>Page Not Found</h1>"
"    <p>Sorry, but the page you were trying to view does not exist.</p>"
"</body>"
"</html>";

// Characters for header and footer of directory content
char * index_body = "<li><a href=\"%s\">%s</a>";
char * index_ftr = "</ul><hr></body></html>";

//
// parseRequest:
//  takes HTTP request
//  and returns 0 if request is invalid
//    Example: "GET /path/to/resource HTTP/1.X" 
//
char* parseRequest(char* request) {
  //assume file paths are no more than 256 bytes + 1 for null.
  char *buffer = malloc(sizeof(char)*257);
  memset(buffer, 0, 257);

  if(fnmatch("GET * HTTP/1.*",  request, 0)) return 0;

  sscanf(request, "GET %s HTTP/1.", buffer);
  return buffer;
}

//
// serve_request:
//  takes the directory that the client requested and displays the file asked for
//
void serve_request(int client_fd, char* directory){
  int read_fd;
  int bytes_read;
  int file_offset = 0;
  char client_buf[4096];
  char send_buf[4096];
  char filename[4096];
  char * requested_file;
  memset(client_buf,0,4096);
  memset(filename,0,4096);
  
  //Store client's path that was requested 
  while(1){
    file_offset += recv(client_fd,&client_buf[file_offset],4096,0);
    if(strstr(client_buf,"\r\n\r\n"))
      break;
  }
  
  //Add WWW to the path 
  char* path = malloc(sizeof(char)*1043);
  strcat(path, "/WWW");
  
  //Add client requested file to path
  requested_file = parseRequest(client_buf);  
  strcat(path, requested_file);
  free(requested_file);
  requested_file = malloc(sizeof(char)*1043);
  strcat(requested_file, path);
  
  //Send to client
  send(client_fd,request_str,strlen(request_str),0);
  
  //Take requested_file, add a . to beginning, open that file
  filename[0] = '.';
  strncpy(&filename[1],requested_file,4095);
  read_fd = open(filename,0,0);
  while(1){
    bytes_read = read(read_fd,send_buf,4096);
    if(bytes_read <= 0)
      break;
    send(client_fd,send_buf,bytes_read,0);
  }
  close(read_fd);

  //Clean up the resources associated with the client
  close(client_fd);
  return;
}

//
// main:
//   Args : 1) The port number to listen for connections
//          2) The directory to serve files from.
int main(int argc, char** argv) {
    /* For checking return values. */
    int retval;

    /* Read the port number from the first command line argument. */
    int port = atoi(argv[1]);

    /* Create a socket to which clients will connect. */
    int server_sock = socket(AF_INET6, SOCK_STREAM, 0);
    if(server_sock < 0) {
        perror("Creating socket failed");
        exit(1);
    }

    /* Read the name of directory from the second line argument. */
    char* directory = argv[2];

    //A server socket binding to a port
    int reuse_true = 1;
    retval = setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &reuse_true,
                        sizeof(reuse_true));
    if (retval < 0) {
        perror("Setting socket option failed");
        exit(1);
    }

    //Create an address structure
    struct sockaddr_in6 addr;   // internet socket address data structure
    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(port); // byte order is significant
    addr.sin6_addr = in6addr_any; // listen to all interfaces

    //Bind the socket to address and specified port
    retval = bind(server_sock, (struct sockaddr*)&addr, sizeof(addr));
    if(retval < 0) {
        perror("Error binding to port");
        exit(1);
    }

    //Start listening
    retval = listen(server_sock, BACKLOG);
    if(retval < 0) {
        perror("Error listening for connections");
        exit(1);
    }

    while(1) {
        //Declare a socket for the client connection
        int sock;

        //Another address structure to fill in when a new connection is accepted
        struct sockaddr_in remote_addr;
        unsigned int socklen = sizeof(remote_addr);

        //Accept the first waiting connection from the server socket
        sock = accept(server_sock, (struct sockaddr*) &remote_addr, &socklen);
        if(sock < 0) {
            perror("Error accepting connection");
            exit(1);
        }

	//Serve
        serve_request(sock, directory);
    }
    close(server_sock);
}
