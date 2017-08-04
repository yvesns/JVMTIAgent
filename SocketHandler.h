#ifndef _JVMTIAgent_SocketHandler_h_
#define _JVMTIAgent_SocketHandler_h_

#include <iostream>
#include "CommandHandler.h"
#include "AgentLogger.h"

DWORD WINAPI listenClients(LPVOID lpParam);
DWORD WINAPI listenClients2(LPVOID lpParam);

class SocketHandler{
	public:
		void start();
		void sendResponse(const char *message, int byteCount);
	private:
		struct GlobalAgentData *gdata;
		AgentLogger &logger = AgentLogger::getInstance();
		HANDLE threadHandle;
};

#endif
