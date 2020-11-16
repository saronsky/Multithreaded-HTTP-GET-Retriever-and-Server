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
const string OK_RESPONSE = "HTTP/1.1 200 OK\n";

const int BUFSIZE = 1500;

/**
 * Sets up a socket connection based on command line arguments
 * @param argValues - values from the command line
 * @return -1 for failure. else return socket number
 * */
int connectSocket(char *server_name) {
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
    if (connect(clientSd, (sockaddr *) &sendSockAddr, sizeof(sendSockAddr)) == -1) {
        cerr << "Connect Failure: " << errno << endl;
        return -1;
    } else
        return clientSd;
}

int getStatusCode(char* buffer, string &statusCode){
    statusCode=buffer;
    std::size_t pos=statusCode.find("\n");
    statusCode=statusCode.substr(0,pos+1);
    return pos+1;
}

string getBody(char* buffer){
    int contentLength;
    string buffer2=buffer;
    string find="Content-Length: ";
    std::size_t pos=buffer2.find(find);
    size_t pos2=buffer2.find("\n",pos);
    contentLength=stoi(buffer2.substr(pos+find.length(), pos2-(pos+find.length())));
    string output=buffer;
    output=output.substr(pos2+2, contentLength+1);
    return output;
}

int collectFile(int socketD) {
    char buffer[BUFSIZE];
    int pos=0;
    while(true) {
        cerr<<"Pos: "<<pos<<endl;
        int length = read(socketD, buffer, BUFSIZE - pos);
        if (length == -1) {
            cerr << "Unable to Read from Socket" << endl;
            return -1;
        }
        else if (length==0)
            break;
        pos+=length;
    }
    string statusCode;
    *(buffer)+=getStatusCode(buffer, statusCode);
    if (statusCode!=OK_RESPONSE){
        cout<<"Status Code: "<<statusCode<<endl;
        return 0;
    }
    string body=getBody(buffer);
    cout<<"Body: "<<body;
    fstream myFile(OUTPUT_FILE_DESTINATION);
    myFile<<body;
    myFile.close();
}

/**
 * Gets requested file
 * @param socket: the socket to retrieve the file from
 * @return -1 if failed. else outputs file to OUTPUT_FILE_DESTINATION
 * */
int requestFile(int socketD, char *path) {
    string getRequest="GET "+string(path)+" HTTP/1.1\r\n\r\n";
    return write(socketD, getRequest.c_str(), getRequest.length());
}


/**
 * Receives a server name and file name from the command line and "downloads" it to OUTPUT_FILE_DESTINATION
 * @param argc: the number of arguments fed to main
 * @param argv: the string arguments fed to main
 * @return -1 if not enough arguments, else outputs the file to OUTPUT_FILE_DESTINATION
 * */
int main(int argc, char *argv[]) {
    if (argc != 3) { //if the input is not just a server and a file
        //do something; print to console or something
        cerr << "Not enough arguments" << endl;
        return -1;
    }
    //set up the socket
    int httpSocket = connectSocket(argv[1]);
    if (requestFile(httpSocket, argv[2])!=1)
        collectFile(httpSocket);
    else
        cerr<<"Send ERROR"<<endl;
    close(httpSocket);
}
