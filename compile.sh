#!/bin/sh
g++ -o server -pthread Server.cpp
g++ -o retriever Retriever.cpp