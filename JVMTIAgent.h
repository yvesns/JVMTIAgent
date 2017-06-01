#include <jni.h>
#include <jvmti.h>

#ifndef _JVMTIAgent_JVMTIAgent_h
#define _JVMTIAgent_JVMTIAgent_h

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

typedef struct {
	jvmtiEnv *jvmti;
	jboolean vm_is_started;
	jrawMonitorID lock;
} GlobalAgentData;

#endif