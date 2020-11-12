#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <fstream>
#include <cstring>

using std::ofstream;
using std::string;
string OUTPUT_FILE_DESTINATION = "outputFile.txt";

/**
 * Sets up a socket connection based on command line arguments
 * @param argValues - values from the command line
 * @return -1 for failure. else return socket number
 * */
int connectSocket(char *argValues[]) {
    int socketSetup;
    struct addrinfo hints;
    struct addrinfo *serverInfo;
    memset(&hints, 0, sizeof(hints));
    //setting up address information
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    int socketAddrInfo = getaddrinfo(argValues[1], "80", &hints, &serverInfo); //using the magic number 80 for HTML/HTTP request

    socketSetup = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
    //setting up socket using the info from above, listening for HTTP stuffs
    int httpSocket = connect(socketSetup, serverInfo->ai_addr, serverInfo->ai_addrlen);
    freeaddrinfo(serverInfo);
    return httpSocket;
}

string parseHeader(int socketD) {
    bool endFlag = false;
    char readByte = 0;
    char prevReadByte = 0;
    string headerMessage = "";
    while (!endFlag) {
        recv(socketD, &readByte, 1, 0);
        headerMessage += readByte;
        if (readByte == '\r' && prevReadByte == '\n') {
            endFlag = true;
        }
        prevReadByte = readByte;
    }
    return headerMessage;

}

/**
 * Gets requested file
 * @param socket: the socket to retrieve the file from
 * @return -1 if failed. else outputs file to OUTPUT_FILE_DESTINATION
 * */
int getRequestFile(int socketD, char* argValues[]) {
    string getRequest = string("GET " + string(argValues[2]) + "HTTP/1.1\r\nHost: " + string(argValues[1]) + "\r\n\r\n");
    send(socketD, getRequest.c_str(), sizeof(getRequest.c_str()), 0);
    //read/parse header
    parseHeader(socketD);
    ofstream outFile;
    outFile.open(OUTPUT_FILE_DESTINATION);
    //write the html to the file


    close(socketD);
    outFile.close();
    return 0;
}


/**
 * Receives a server name and file name from the command line and "downloads" it to OUTPUT_FILE_DESTINATION
 * @param argc: the number of arguments fed to main
 * @param argv: the string arguments fed to main
 * @return -1 if not enough arguments, else outputs the file to OUTPUT_FILE_DESTINATION
 * */
int main(int argc, char* argv[]) {
    int httpSocket;

    if (argc != 3) { //if the input is not just a server and a file
        //do something; print to console or something
        return -1;

    } else {

        //set up the socket
        httpSocket = connectSocket(argv);

    }
    return getRequestFile(httpSocket, argv);
}
