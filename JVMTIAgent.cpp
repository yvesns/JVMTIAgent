#include "JVMTIAgent.h"

using namespace std;

GlobalAgentData *gdata;
SocketHandler socketHandler;
AgentLogger &logger = AgentLogger::getInstance();
bool isPaused = false;

BOOL APIENTRY DllMain(HANDLE hModule, DWORD reason,  LPVOID lpReserved)
{
	switch (reason) {
	case DLL_PROCESS_ATTACH:
		socketHandler.start();
		break;
	case DLL_PROCESS_DETACH:
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	}
	return TRUE;
}

JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM *vm, char *options, void *reserved){
	static jvmtiEnv *jvmti = NULL;
	static jvmtiCapabilities jvmtiCaps;
	static jvmtiEventCallbacks callbacks;
	static GlobalAgentData data = {};
	jint rc;
	
	rc = vm->GetEnv((void **)&jvmti, JVMTI_VERSION_1_2);
	
	if (rc != JNI_OK) {
		logger.print("ERROR: Unable to create jvmtiEnv, GetEnv failed.\n");
		return -1;
	}
	
	gdata = &data;
	gdata->jvmti = jvmti;
	
	jvmti->CreateRawMonitor("agent data", &(gdata->lock));
	
	if(initCapabilities(jvmti, jvmtiCaps) != JVMTI_ERROR_NONE){
		logger.print("Unable to get necessary JVMTI capabilities.\n");
		return -1;
	}
	
	if(initNotifications(jvmti) != JVMTI_ERROR_NONE){
		logger.print("Cannot set event notification\n");
		return -1;
	}
	
	if(initCallbacks(jvmti, callbacks) != JVMTI_ERROR_NONE){
		logger.print("Cannot set jvmti callbacks\n");
		return -1;
	}
	
	return JNI_OK;
}

JNIEXPORT jint JNICALL Agent_OnAttach(JavaVM* vm, char *options, void *reserved){}

//Optional
JNIEXPORT void JNICALL Agent_OnUnload(JavaVM *vm){}

jvmtiError initCapabilities(jvmtiEnv *jvmti, jvmtiCapabilities caps){
	(void)memset(&caps, 0, sizeof(jvmtiCapabilities));
	
	caps.can_suspend = 1;
	caps.can_generate_breakpoint_events = 1;
	caps.can_get_bytecodes = 1;
	
	return jvmti->AddCapabilities(&caps);
}

jvmtiError initNotifications(jvmtiEnv *jvmti){
	jvmtiError error;
	
	error = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_VM_INIT, (jthread)NULL);
	error = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_CLASS_PREPARE, (jthread)NULL);
	error = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_BREAKPOINT, (jthread)NULL);
	
	return error;
}

jvmtiError initCallbacks(jvmtiEnv *jvmti, jvmtiEventCallbacks callbacks){
	(void)memset(&callbacks, 0, sizeof(callbacks));
	
	callbacks.VMInit = &callbackVMInit;
	callbacks.ClassPrepare = &callbackClassPrepare;
	callbacks.Breakpoint = &callbackBreakpoint;
	
	return jvmti->SetEventCallbacks(&callbacks,(jint)sizeof(callbacks));
}

void JNICALL callbackVMInit(jvmtiEnv *jvmti, JNIEnv *jni_env, jthread t){}

void JNICALL callbackClassPrepare(jvmtiEnv *jvmti, JNIEnv *jni_env, jthread t, jclass c){
	jint methodCount = 0;
	jmethodID *methods;
	std::vector<jmethodID>::iterator it;
	
	char *methodName;
	char *methodSignature;
	char *methodGenericSignature;
	
	jvmtiError error;
	
	error = jvmti->GetClassMethods(c, &methodCount, &methods);
	
	for(int j = 0; j < methodCount; j++){
		error = jvmti->GetMethodName(methods[j], &methodName, &methodSignature, &methodGenericSignature);
		
		if(strcmp(methodName, "main") == 0){
			it = find(gdata->suspendedMethods.begin(), gdata->suspendedMethods.end(), methods[j]);
			
			jvmti->SetBreakpoint(methods[j], 0);
			logger.print("Breakpoint set on main.\n");
		}
		
		jvmti->Deallocate(reinterpret_cast<unsigned char*>(methodName));
		jvmti->Deallocate(reinterpret_cast<unsigned char*>(methodSignature));
		jvmti->Deallocate(reinterpret_cast<unsigned char*>(methodGenericSignature));
	}
	
	jvmti->Deallocate(reinterpret_cast<unsigned char*>(methods));
}

void JNICALL callbackBreakpoint(jvmtiEnv *jvmti, JNIEnv* jni_env, jthread t, jmethodID method, jlocation location){
	isPaused = true;
	jvmtiPhase phase;
	
	gdata->jvmti->GetPhase(&phase);
	
	logger.print("phase in breakpoint: ", (int)phase);
	
	if(phase == JVMTI_PHASE_LIVE){
		while(isPaused){Sleep(10);}
	}
}