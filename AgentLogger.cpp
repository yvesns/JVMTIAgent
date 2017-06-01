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