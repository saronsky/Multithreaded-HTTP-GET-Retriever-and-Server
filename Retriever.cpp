#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <fstream>
#include <cstring>

using std::ofstream;
using std::string;
using std::cout;
string OUTPUT_FILE_DESTINATION = "outputFile.txt";

int connectSocket(char *argValues[]);
string parseHeaderLine(int socketD);
int getRequestFile(int socketD, char* argValues[]);

/**
 * Receives a server name and file name from the command line and "downloads" it to OUTPUT_FILE_DESTINATION
 * @param argc: the number of arguments fed to main
 * @param argv: the string arguments fed to main
 * @return -1 if not enough arguments, else outputs the file to OUTPUT_FILE_DESTINATION
 * */
int main(int argc, char* argv[]) {
    int httpSocket;

    if (argc != 4) { //if the input is not just a server and a file
        //do something; print to console or something
        return -1;

    } else {

        //set up the socket
        httpSocket = connectSocket(argv);

    }
    return getRequestFile(httpSocket, argv);
}

/**
 * Sets up a socket connection based on command line arguments
 * @param argValues - values from the command line
 * @return -1 for failure. else return socket number
 * */
int connectSocket(char *argValues[]) {
    struct addrinfo hints;
    struct addrinfo *serverInfo;
    memset(&hints, 0, sizeof(hints));
    //setting up address information
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    int httpSocket;
    if (getaddrinfo(argValues[1], argValues[3], &hints, &serverInfo) != 0) {  //using the magic number 80 for HTML/HTTP request
        return -1;
    }
    struct addrinfo *connectionIterator;
    for (connectionIterator = serverInfo; connectionIterator != nullptr; connectionIterator = connectionIterator->ai_next) {
        if ((httpSocket = socket(connectionIterator->ai_family, connectionIterator->ai_socktype, connectionIterator->ai_protocol)) == -1) {
            cout << "Invalid socket descriptor...\n";
        } else {
            if (connect(httpSocket, serverInfo->ai_addr, serverInfo->ai_addrlen) == -1) {
                cout << "Invalid socket connection\n";
            } else {
                cout << "CONNECTION FOUND!\n";
                break;
            }
        }
    }
    if (connectionIterator == nullptr) {
        cout << "Unable to connect\n";
        return -1;
    }
    //setting up socket using the info from above, listening for HTTP stuffs
    freeaddrinfo(serverInfo);
    return httpSocket;
}

/**
 * Parses 1 line of the HTTP return header
 * @param socketD: socket descriptor
 * @return a string that is the next line of the header
 * */
string parseHeaderLine(int socketD) {
    char prevReadByte = 0;
    string headerMessage = "";
    while (1) {
        char readByte = 0;
        read(socketD, &readByte, sizeof(readByte));
        if (readByte == '\n' || readByte == '\r') {
            if (readByte == '\n' && prevReadByte == '\r') {
                break;
            }
        } else {
            headerMessage += readByte;
            prevReadByte = readByte;
        }
    }
    return headerMessage;
}

/**
 * Gets requested file
 * @param socket: the socket to retrieve the file from
 * @return -1 if failed. else outputs file to OUTPUT_FILE_DESTINATION
 * */
int getRequestFile(int socketD, char* argValues[]) {

    //send GET request using non-persistent connection
    string getRequest = string("GET /" + string(argValues[2]) + " HTTP/1.1\r\n"
            + "Host: " + string(argValues[1]) + "\r\n" + "\r\n");
    if ((write(socketD, getRequest.c_str(), sizeof(getRequest.c_str()))) == sizeof(getRequest.c_str())) {
        cout << "Request: " << getRequest << "Sent\n";
    }
    //read/parse header
    int contentLength = 0;
    while (1) {
        string curHeaderLine = parseHeaderLine(socketD);
        cout << curHeaderLine << "";
        if (curHeaderLine.substr(0, 15) == "Content-Length:") {
            contentLength = atoi(curHeaderLine.substr(16,curHeaderLine.length()).c_str());
        }
        if (curHeaderLine.empty()) {
            break;
        }
    }

    char fileBuf[contentLength];

    //output contents to OUTPUT_FILE_DESTINATION
    ofstream outFile;
    outFile.open(OUTPUT_FILE_DESTINATION);

    outFile << fileBuf;

    //always close the files/streams!
    close(socketD);
    outFile.close();

    return 0;
}


