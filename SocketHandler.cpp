#include "SocketHandler.h"

using namespace std;

static SOCKET ClientSocket;

void SocketHandler::sendResponse(const char *message, int byteCount){
	logger.print("Byte count: ", byteCount);
	logger.print("\n");
	logger.print("Bytes sent: ", send(ClientSocket, message, byteCount, 0));
	logger.print("\n");
}

void SocketHandler::start(){
	WSADATA wsaData;
	
	int bufferLen = 4096;
	char const *address = "127.0.0.1";
	char const *port = "5554";
	
    int iResult;
    
    SOCKET ListenSocket = INVALID_SOCKET;
    
    struct addrinfo *result = NULL;
    
    int iSendResult;
    char recvbuf[bufferLen];
    int recvbuflen = bufferLen;
    
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        logger.print("WSAStartup failed\n");
        //return 1;
    }
    
    sockaddr_in service;
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = inet_addr("127.0.0.1");
    service.sin_port = htons(5554);
    
    // Create a SOCKET for connecting to server
    ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ListenSocket == INVALID_SOCKET) {
        logger.print("socket failed\n");
        freeaddrinfo(result);
        WSACleanup();
        //return 1;
    }
    
    // Setup the TCP listening socket
    iResult = bind(ListenSocket, (SOCKADDR*) &service, sizeof(service));
    if (iResult == SOCKET_ERROR) {
        logger.print("bind failed\n");
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        //return 1;
    }
    
    freeaddrinfo(result);
    
    // Listen for incoming connections requests
    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        logger.print("listen failed\n");
        closesocket(ListenSocket);
        WSACleanup();
        //return 1;
    }
    
    // Accept a client socket
    
    ClientSocket = accept(ListenSocket, NULL, NULL);
    
    if (ClientSocket == INVALID_SOCKET) {
        logger.print("accept failed\n");
        closesocket(ListenSocket);
        WSACleanup();
        //return 1;
    }
    
    //No longer need server socket
    closesocket(ListenSocket);
    
    threadHandle = CreateThread(NULL, 0, listenClients, NULL, 0, NULL);
	
	//return 0;
}

DWORD WINAPI listenClients(LPVOID lpParam){
	AgentLogger &logger = AgentLogger::getInstance();
	
    CommandHandler commandHandler;
    
    int recvbuflen = 4096;
    char recvbuf[recvbuflen];
    int iResult;
    
    do {
        logger.print("Waiting for client message\n");
	    iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
	    if (iResult > 0) {
	        logger.print("Bytes received\n");
	
	    	commandHandler.handleCommand(recvbuf);
	    }
	    else if (iResult == 0)
	        logger.print("Connection closing...\n");
	    else  {
	        logger.print("recv failed\n");
	        closesocket(ClientSocket);
	        WSACleanup();
	        return 1;
	    }
    } while (iResult > 0);
    
    // shutdown the connection since we're done
    iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        logger.print("shutdown failed\n");
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }
    
    // cleanup
    closesocket(ClientSocket);
    WSACleanup();
	
	return 0;
}