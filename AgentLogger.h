#ifndef _JVMTIAgent_AgentLogger_h_
#define _JVMTIAgent_AgentLogger_h_

#include <string>
#include <fstream>
#include <string.h>

using namespace std;

class AgentLogger{
	private:
		ofstream file;
		string fileName = "./log.txt";
		
		AgentLogger();
		AgentLogger(const AgentLogger &);
		void operator=(const AgentLogger &);
	public:
		static AgentLogger &getInstance();
		void log(string msg);
		void setFileName(string name);
		void print(const char *msg);
		void print(const char *msg, int data);
};

#endif