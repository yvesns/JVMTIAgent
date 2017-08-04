#include "CommandHandler.h"

CommandHandler::CommandHandler(){
	socketHandler = new SocketHandler();
}

int CommandHandler::handleCommand(char *message){
	int c = message[0];
	
	if(c == Command::resume){
		logger.print("Resuming\n");
		resume();
	} else if(c == Command::getCurrentMethodBytes){
		logger.print("Sending method bytes\n");
		sendCurrentMethodBytes();
	} else if(c == Command::getLoadedClasses){
		logger.print("Sending loaded classes\n");
		sendLoadedClasses();
	} else if(c == Command::getPhase){
		logger.print("Sending phase code\n");
		sendPhaseCode();
	}
}

template<typename T>
void CommandHandler::objectToCharArray(T object, char *byteArray){
	int size = sizeof(T);
	
	for(int i = 0; i < size; i++){
		byteArray[i] = object >> i;
	}
}

void CommandHandler::resume(){
	isPaused = false;
	logger.print("resumed\n");
}

void CommandHandler::sendCurrentMethodBytes(){
	jthread currentThread;
	jmethodID method;
	jlocation location;
	jint bytecodeCount;
	unsigned char *bytecodes;
	jvmtiError err;
	jvmtiPhase phase;
	
	gdata->jvmti->GetCurrentThread(&currentThread);
	gdata->jvmti->GetFrameLocation(currentThread, 1, &method, &location);
	err = gdata->jvmti->GetBytecodes(method, &bytecodeCount, &bytecodes);
	
	logger.print("Bytecode count: ", bytecodeCount);
	logger.print("GetBytecodes error: ", err);
	
	socketHandler->sendResponse(reinterpret_cast<const char*>(bytecodes), bytecodeCount);
	//TODO: properly call jvmti->deallocate here.
}

void CommandHandler::sendLoadedClasses(){
	jint classCount;
	jclass *classArray;
	char *response;
	
	gdata->jvmti->GetLoadedClasses(&classCount, &classArray);
	
	response = new char[classCount*8];
	
	for(int i = 0; i < (classCount*8); i += 8){
		for(int j = 0; j < 8; j++){
			response[i+j] = (long long)classArray[i+j] >> (j * 8);
		}
	}
	
	logger.print("Response array built\n");
	
	socketHandler->sendResponse(response, classCount*8);
	
	delete [] response;
	
	logger.print("Response array sent\n");
	
	//TODO: properly call jvmti->deallocate here.
}

void CommandHandler::sendPhaseCode(){
	jvmtiPhase phase;
	char* response = new char[sizeof(jvmtiPhase)];
	
	gdata->jvmti->GetPhase(&phase);
	logger.print("phase in cmdh: ", (int)phase);
	objectToCharArray(phase, response);
	socketHandler->sendResponse(response, 4);
	
	delete [] response;
}