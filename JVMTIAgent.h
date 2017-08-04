//#pragma comment (lib, "Ws2_32.lib")

#ifndef _JVMTIAgent_JVMTIAgent_h
#define _JVMTIAgent_JVMTIAgent_h

#include <jni.h>
#include <jvmti.h>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include <algorithm> 
#include "SocketHandler.h"
#include "AgentLogger.h"

#define DllImport extern "C" __declspec(dllimport)
#define DllExport extern "C" __declspec(dllexport)

#ifdef flagDLL
	#define DLLIMPORT __declspec(dllexport)
#else
	#define DLLIMPORT __declspec(dllimport)
#endif

jvmtiError initCapabilities(jvmtiEnv *jvmti, jvmtiCapabilities caps);
jvmtiError initNotifications(jvmtiEnv *jvmti);
jvmtiError initCallbacks(jvmtiEnv *jvmti, jvmtiEventCallbacks callbacks);
void JNICALL callbackVMInit(jvmtiEnv *jvmti ,JNIEnv *jni_env, jthread t);
void JNICALL callbackClassPrepare(jvmtiEnv *jvmti, JNIEnv *jni_env, jthread t, jclass c);
void JNICALL callbackBreakpoint(jvmtiEnv *jvmti_env, JNIEnv* jni_env, jthread t, jmethodID method, jlocation location);

typedef struct GlobalAgentData{
	jvmtiEnv *jvmti;
	jvmtiPhase phase;
	jboolean vm_is_started;
	jrawMonitorID lock;
	std::map<jmethodID, jthread> methodThreadBreakpointMap;
	std::vector<jthread> suspendedThreads;
	std::vector<jmethodID> suspendedMethods;
} GlobalAgentData;

extern GlobalAgentData *gdata;
extern bool isPaused;

#endif