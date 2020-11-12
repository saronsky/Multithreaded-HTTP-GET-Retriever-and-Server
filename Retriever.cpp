//
// Created by Simon on 11/11/2020.
//

#include <sys/types.h>      //socket, bind
#include <sys/socket.h>     //socket, bind, listen, inet_ntoa
#include <sys/time.h>       //gettimeofday
#include <netinet/in.h>     //htonl, htons, inet_ntoa
#include <arpa/inet.h>      //inet_ntoa
#include <netdb.h>          //gethostbyname
#include <unistd.h>         //read, write, close
#include <strings.h>        //bzero
#include <netinet/tcp.h>    //SO_REUSEADDR
#include <sys/uio.h>        //writev
#include <pthread.h>        //pthread_t
#include <chrono>           //usec
#include <stdio.h>          //printf
#include <iostream>         //cerr
#include <map>

using namespace std;



struct Socket {
    int clientSd; //server socket File Descriptor
    sockaddr_in sendSockAddr; //address used for accepting socket
};

int makeConnection(char *serverName, int serverPort, Socket initial);
int writeRequest(char* fileName, Socket initial);

int main(int argc, char *argv[]){
    if (argc!=3)
        return -1;

    char *serverName = argv[1];
    char *fileName=argv[2];
    int serverPort; //WHAT PORT NUM
    struct Socket initial;

    if(makeConnection(serverName, serverPort, initial)==-1){
        cerr<<"Unable to Connect"<<endl;
        return -1;
    }
    if (writeRequest(fileName, initial)==-1){
        cerr<<"Unable to Send Request"<<endl;
        return -1;
    }
}

/*
 * This function takes the server name and server port and returns a socket to that server
 *
 * PreConditions: server_name - the Name of the server being connected to
 *                              may be IP address or name
 *                server_port - the port at the server being connected to
 *                              Should be last 4 digits of student ID
 *
 * PostConditions: returns socket
 */

int makeConnection(char *serverName, int serverPort, Socket initial) {
    struct hostent *host = gethostbyname(serverName);
    bzero((char *) &initial.sendSockAddr, sizeof(initial.sendSockAddr));
    initial.sendSockAddr.sin_family = AF_INET; // Address Family Internet
    initial.sendSockAddr.sin_addr.s_addr =
            inet_addr(inet_ntoa(*(struct in_addr *) *host->h_addr_list));
    initial.sendSockAddr.sin_port = htons(serverPort);

    initial.clientSd = socket(AF_INET, SOCK_STREAM, 0);  //socket number

    return(connect(initial.clientSd, (sockaddr *) &initial.sendSockAddr, sizeof(initial.sendSockAddr)));

}

int writeRequest(char* fileName, Socket initial){
    string request="";
    request+="GET ";
    request+=fileName;
    request+=" HTTP/1.0";
    return write(initial.clientSd, request.c_str(), sizeof(request.c_str()));
}