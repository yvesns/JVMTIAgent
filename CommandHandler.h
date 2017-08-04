#ifndef _JVMTIAgent_CommandHandler_h_
#define _JVMTIAgent_CommandHandler_h_

#include "JVMTIAgent.h"
#include "Agentlogger.h"
#include "SocketHandler.h"

enum Command{
	resume = 0,
	getCurrentMethodBytes,
	getLoadedClasses,
    getClassMethods,
    getMethodBytes,
    getPhase
};

class SocketHandler;
class CommandHandler{
	public:
		int handleCommand(char *message);
		CommandHandler();
	private:
		SocketHandler *socketHandler;
		AgentLogger &logger = AgentLogger::getInstance();
		template<typename T> void objectToCharArray(T object, char *byteArray);
		void resume();
		void sendCurrentMethodBytes();
		void sendLoadedClasses();
		void sendPhaseCode();
};

#endif
