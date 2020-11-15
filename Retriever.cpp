#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <fstream>
#include <cstring>
#include <arpa/inet.h>    // inet_ntoa


using namespace std;
string OUTPUT_FILE_DESTINATION = "outputFile.txt";
int server_port = 1025; ///MIGHT NEED TO IMPLEMENT DELETE

const int BUFSIZE = 1500;

/**
 * Sets up a socket connection based on command line arguments
 * @param argValues - values from the command line
 * @return -1 for failure. else return socket number
 * */
int connectSocket(char *server_name) {
    /*int socketSetup;
    struct addrinfo hints;
    struct addrinfo *serverInfo;
    memset(&hints, 0, sizeof(hints));
    //setting up address information
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    int socketAddrInfo = getaddrinfo(argValues[1], "80", &hints, &serverInfo); //using the magic number 80 for HTML/HTTP request

    socketSetup = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
    if (socketSetup==-1)
        cerr<<"Socket Failure: "<<errno<<endl;
    else
        cerr<<"Socket Success: "<<socketSetup<<endl;
    //setting up socket using the info from above, listening for HTTP stuffs
    int httpSocket = connect(socketSetup, serverInfo->ai_addr, serverInfo->ai_addrlen);
    if (httpSocket==-1)
        cerr<<"Connect Failure: "<<errno<<endl;
    else
        cerr<<"Connect Success"<<endl;
    freeaddrinfo(serverInfo);
    return httpSocket;*/
    struct hostent *host = gethostbyname(server_name);
    sockaddr_in sendSockAddr;
    bzero((char *) &sendSockAddr, sizeof(sendSockAddr));
    sendSockAddr.sin_family = AF_INET; // Address Family Internet
    sendSockAddr.sin_addr.s_addr =
            inet_addr(inet_ntoa(*(struct in_addr *) *host->h_addr_list));
    sendSockAddr.sin_port = htons(server_port);
    int clientSd = socket(AF_INET, SOCK_STREAM, 0);  //socket number
    if (clientSd == -1)
        cerr << "Socket Failure: " << errno << endl;
    else
        cerr << "Socket Success: " << clientSd << endl;

    if (connect(clientSd, (sockaddr *) &sendSockAddr, sizeof(sendSockAddr)) == -1) {
        cerr << "Connect Failure: " << errno << endl;
        return -1;
    } else
        return clientSd;
}

string parseHeaderLine(int socketD) {
    /*bool endFlag = false;
    char readByte = 0;
    char prevReadByte = 0;
    string headerMessage = "";
    while (!endFlag) {
        read(socketD, &readByte, 1);
        headerMessage += readByte;
        if (readByte == '\r' && prevReadByte == '\n') {
            endFlag = true;
        }
        prevReadByte = readByte;
    }
    return headerMessage;*/

    string headerMessage;
    headerMessage.resize(BUFSIZE);
    int length = read(socketD, &headerMessage[0], BUFSIZE - 1);
    if (length == -1) {
        cerr << "Unable to Read from Socket" << endl;
    }
    headerMessage.resize(length);
    return headerMessage;
}

/**
 * Gets requested file
 * @param socket: the socket to retrieve the file from
 * @return -1 if failed. else outputs file to OUTPUT_FILE_DESTINATION
 * */
int getRequestFile(int socketD, char *path) {
    string getRequest="GET "+string(path)+" HTTP/1.0\r\n\r\n";
    cout<<"Request: "<<getRequest.c_str()<<endl<<"Size: "<<getRequest.length()<<endl;
    int count=write(socketD, getRequest.c_str(), getRequest.length());
    if (count == -1)
        cerr << "Send ERROR" << endl;
    else
        cerr << "Send Success" << endl;
    //read/parse header
    cout<<parseHeaderLine(socketD);
    close(socketD);
    return 0;
}


/**
 * Receives a server name and file name from the command line and "downloads" it to OUTPUT_FILE_DESTINATION
 * @param argc: the number of arguments fed to main
 * @param argv: the string arguments fed to main
 * @return -1 if not enough arguments, else outputs the file to OUTPUT_FILE_DESTINATION
 * */
int main(int argc, char *argv[]) {
    int httpSocket;
    cout << "Argc: " << argc << endl;
    if (argc != 3) { //if the input is not just a server and a file
        //do something; print to console or something
        cerr << "Not enough arguments" << endl;
        return -1;
    }
    cerr << "Enought Arguments" << endl;
    //set up the socket
    httpSocket = connectSocket(argv[1]);
    return getRequestFile(httpSocket, argv[2]);
}
