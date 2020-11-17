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

const string THREAD_MESSAGE = "Creating new thread with count: ";
const string OK_RESPONSE = "HTTP/1.1 200 OK\n";
const string DOES_NOT_EXIST_RESPONSE = "HTTP/1.1 404 Not Found\n";
const string UNAUTHORIZED_RESPONSE = "HTTP/1.1 401 Unauthorized\n";
const string FORBIDDEN_RESPONSE = "HTTP/1.1 403 Forbidden\n";
const string BAD_REQUEST_RESPONSE = "HTTP/1.1 400 Bad Request\n";
const string SECRET_FILE = "SecretFile.html";
const int CONNECTION_REQUEST_MAX = 10;
const int BUFSIZE = 1500;

//All-in-one Socket struct
struct Socket {
    int serverSd; //server socket File Descriptor
    sockaddr_storage serverStorage; //used for storing a bunch of socket addresses WHAT IS THIS USED FOR DELETE
    sockaddr_in sockAddrIn; //address used for accepting socket
    socklen_t addressSize; //size of the address
};

int establishConnection(int portNum, Socket &initial);
static void *completeRequest(void *threadData);
int collectInput(int newSocket, string &input);
void createResponse(string input, string &output);

/**
 * Multi-threaded server that handles HTTP requests
 * @param argc - number of arguments fed to main
 * @param argv - string arguments fed to main
 * @return -1 if failed, 0 on server exit
 * */
int main(int argc, char *argv[]) {
    /*
     * argc check
     */
    if (argc != 2) {
        cerr << "Incorrect number of arguments " << endl;
        return -1;
    }

    /*
     * convert char* to int
     */
    int portNum = stoi(argv[1]);

    struct Socket initial;
    if (establishConnection(portNum, initial) == -1)
        return -1;
    else
        cout << "Successfully established Connection" << endl;


    //accept loop creates a new socket for an incoming connection
    initial.addressSize = sizeof(initial.serverStorage);

    /*
     * 3. Loop back to the accept command and wait for a new connection
     */
    int newSd = 0;
    while ((newSd = accept(initial.serverSd, (sockaddr *) &initial.serverStorage, &initial.addressSize)) >= 0) {
        pthread_t newTID;
        if (newSd == -1) {
            cerr << "Problem with Client connecting" << endl;
            return -1;
        } else if (pthread_create(&newTID, NULL, &completeRequest, &newSd) != 0) {
            cerr << "Thread not created" << endl;
            return -1;
        }
        cerr << "Connection Accepted" << endl;
    }
}

/**
 * Creates a connection to be listened to
 * @param initial - Socket struct describing a socket
 * @param portNum - port to be listened to
 * @return -1 if failed, 0 if bind success
 * */
int establishConnection(int portNum, Socket &initial) {
    /*
     * 1. Accept a new Connection
     */

    //set all bits of the address to 0
    bzero((char *) &initial.sockAddrIn, sizeof(initial.sockAddrIn));

    //configure the settings of the server address
    //address family: internet (AF_INET)
    initial.sockAddrIn.sin_family = AF_INET;
    //set addresses to any incoming
    initial.sockAddrIn.sin_addr.s_addr = htonl(INADDR_ANY);
    //set port num to last 4 digits of id
    initial.sockAddrIn.sin_port = htons(portNum);

    initial.serverSd = socket(AF_INET, SOCK_STREAM, 0);
    if (initial.serverSd == -1) {
        cerr << "Unable to create socket" << endl;
        return -1;
    }

    //setup socket settings: reuse the address to make shutdown easier
    const int on = 1;
    if (setsockopt(initial.serverSd, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(int)) == -1) {
        cerr << "Unable to set Socket Options" << endl;
        return -1;
    }

    if (bind(initial.serverSd, (sockaddr *) &initial.sockAddrIn, sizeof(initial.sockAddrIn)) == -1) {
        cerr << "Unable to bind server socket to the address" << endl;
        cerr << errno;
        return -1;
    }
    sockaddr_in newSockAddr;

    if (listen(initial.serverSd, CONNECTION_REQUEST_MAX == -1)) {
        cerr << "Unable to listen to socket" << endl;
        return -1;
    }
    return 0;
}

/**
 * This function communicates through the socket to respond to HTTP requests
 * @param threadData - thread to be run on
 */
static void *completeRequest(void *threadData) {
    int newSocket = *((int *) threadData);
    string input, output;
    collectInput(newSocket, input);
    createResponse(input, output);
    if (write(newSocket, output.c_str(), output.size()) == -1)
        cout << "Failed to write";
    else
        cout << "Wrote Successfully!" << endl;
    close(newSocket);
    pthread_exit(0);
}

/**
 * Collects the HTTP message from a socket
 * @param input - string buffer to be read into
 * @param newSocket - socket descriptor to be read from
 * @return -1 if failed, 0 if collected
 * */
int collectInput(int newSocket, string &input) {
    input.resize(BUFSIZE);
    int length = read(newSocket, &input[0], BUFSIZE - 1);
    if (length == -1) {
        cerr << "Unable to Read from Socket" << endl;
        return -1;
    }
    input.resize(length);
    return 0;
}

/**
 * Creates a HTTP response based on the parameters of it
 * @param input - input message to be responded to
 * @param output - response output
 * */
void createResponse(string input, string &output) {
    cout<<"Input: "<<input<<endl;
    if (input.substr(0, 3) != "GET" || input.find(" HTTP/1.1\r\n") == string::npos) {
        cout << "Last 13: " << input.substr(input.length() - 13);
        output = BAD_REQUEST_RESPONSE;
        return;
    }
    input = input.substr(0, input.find(" HTTP/1.1\r\n") + 13);
    string path = input.substr(4).substr(0, input.length() - 17);
    cout << "Path: " << path << endl;
    if (path.find("../")) {
        output = FORBIDDEN_RESPONSE;
        return;
    }
    if (path.length()>=15 && path.substr(path.length() - 15, path.length()) == SECRET_FILE) {
        output = UNAUTHORIZED_RESPONSE;
        return;
    }
    FILE *file = fopen(path.c_str(), "r");
    if (file == NULL) {
        if (errno == EACCES) {
            output = UNAUTHORIZED_RESPONSE;
            return;
        }
        output = DOES_NOT_EXIST_RESPONSE;
        return;
    }
    output = OK_RESPONSE;
    output += "Content-Type: text/plain\nContent-Length: ";
    string body = "\r\n\r\n";
    while (true) {
        char c = fgetc(file);
        if (c == EOF) {
            break;
        }
        body.push_back(c);
    }
    output += to_string(body.length() - 4);
    output += body;
}