//
// Created by Simon on 11/14/2020.
//

//#include "Retriever.cpp"
//#include "Server.cpp"
#include <stdio.h>          //printf
#include <iostream>         //cerr
#include <unistd.h>
using namespace std;

int main()
{
    char* path="/mnt/c/Users/Simon/Desktop/School/CSS432 Networking/HW2/temp.txt";
    FILE *file;
    file= fopen(path, "r");
    string output = "";
    while (true) {
        cout<<"search"<<endl;
        char c = fgetc(file);
        if (c == EOF)
            break;
        output.push_back(c);
        cout<<output<<endl;
    }
    cout<<output;
}
