#include "AgentLogger.h"

AgentLogger::AgentLogger(){}

AgentLogger &AgentLogger::getInstance(){
	static AgentLogger logger;
	return logger;
}

void AgentLogger::setFileName(string name){
	this->fileName = "./" + name;
}

void AgentLogger::log(string msg){
	this->file.open(this->fileName);
	this->file << msg;
	this->file.flush();
	this->file.close();
}

void AgentLogger::print(const char *msg){
	printf(msg);
	fflush(stdout);
}

void AgentLogger::print(const char *msg, int data){
	int len = strlen(msg);
	char m[len+2];
	strcpy(m, msg);
	strcat(m, "%d");
	
	printf(m, data);
	fflush(stdout);
}