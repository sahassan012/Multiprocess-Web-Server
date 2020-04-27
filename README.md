# Multi-process Web Server in C
## About
This program serves files(html, txt, pdf, png, jpeg, jpg, and gif) using HTTP protocol and displays them. It also uses multi-threading to handle multiple client connections.

## How it works
Two arguments are taken from the command line:
 - Port number to listen for incomming connections
 - Directory name or path from which to serve files(document root) 

For each client, the program looks for the path that was requested and the following possibilities are considered:
 - If the path is valid and a file, send a response back to client with that file
 - If the path is valid and a directory, check if it contains an index.html file
   - If it contains index.html, send a response back to client with that file
   - If it does not contain index.html, respond with a directory listing
 - If the path is invalid, send a response with a 404 code back to client 

Finally, the connection is closed.

## How to run(from command line) 
To create an executable file, use Makefile by running:
```console
make
```
This should create an executable "server" file in the program directory

You may then run the server by entering:
```console
./server {port} {directory}
```


