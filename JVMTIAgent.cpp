#include <windows.h>
#include "JVMTIAgent.h"

static GlobalAgentData *gdata;

BOOL APIENTRY DllMain(HANDLE hModule, DWORD reason,  LPVOID lpReserved)
{
	switch (reason) {
	case DLL_PROCESS_ATTACH:
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
	static GlobalAgentData data;
	jint rc;
	
	rc = vm->GetEnv((void **)&jvmti, JVMTI_VERSION_1_2);
	
	if (rc != JNI_OK) {
		fprintf(stderr, "ERROR: Unable to create jvmtiEnv, GetEnv failed, error=%d\n", rc);
		return -1;
	}
	
	(void)memset((void*)&data, 0, sizeof(data));
	gdata = &data;
	gdata->jvmti = jvmti;
	
	jvmti->CreateRawMonitor("agent data", &(gdata->lock));
	
	if(initCapabilities(jvmti, jvmtiCaps) != JVMTI_ERROR_NONE){
		printf("Unable to get necessary JVMTI capabilities.");
		return -1;
	}
	
	if(initNotifications(jvmti) != JVMTI_ERROR_NONE){
		printf("Cannot set event notification");
		return -1;
	}
	
	if(initCallbacks(jvmti, callbacks) != JVMTI_ERROR_NONE){
		printf("Cannot set jvmti callbacks");
		return -1;
	}
	
	return JNI_OK;
}

JNIEXPORT jint JNICALL Agent_OnAttach(JavaVM* vm, char *options, void *reserved){}

//Optional
JNIEXPORT void JNICALL Agent_OnUnload(JavaVM *vm){}

jvmtiError initCapabilities(jvmtiEnv *jvmti, jvmtiCapabilities caps){
	(void)memset(&caps, 0, sizeof(jvmtiCapabilities));
	
	/*
		Set capabilities. Ex.:
		jvmtiCaps.can_signal_thread = 1;
		capa.can_get_owned_monitor_info = 1;
	*/
	
	caps.can_suspend = 1;
	caps.can_generate_breakpoint_events = 1;
	
	return jvmti->AddCapabilities(&caps);
}

jvmtiError initNotifications(jvmtiEnv *jvmti){
	jvmtiError error;
	
	/*
		Set notifications. Ex.:
		error = (*jvmti)->SetEventNotificationMode(jvmti, JVMTI_ENABLE, JVMTI_EVENT_VM_INIT, (jthread)NULL);
 		error = (*jvmti)->SetEventNotificationMode(jvmti, JVMTI_ENABLE, JVMTI_EVENT_VM_DEATH, (jthread)NULL);
	*/
	
	error = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_VM_INIT, (jthread)NULL);
	error = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_CLASS_PREPARE, (jthread)NULL);
	error = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_BREAKPOINT, (jthread)NULL);
	
	return error;
}

jvmtiError initCallbacks(jvmtiEnv *jvmti, jvmtiEventCallbacks callbacks){
	(void)memset(&callbacks, 0, sizeof(callbacks));
	
	/*
		Register callbacks. Ex.:
		callbacks.VMInit = &callbackVMInit;
		callbacks.VMDeath = &callbackVMDeath;
	*/
	
	callbacks.VMInit = &callbackVMInit;
	callbacks.ClassPrepare = &callbackClassPrepare;
	callbacks.Breakpoint = &callbackBreakpoint;
	
	return jvmti->SetEventCallbacks(&callbacks,(jint)sizeof(callbacks));
}

void JNICALL callbackVMInit(jvmtiEnv *jvmti, JNIEnv *jni_env, jthread t){}

void JNICALL callbackClassPrepare(jvmtiEnv *jvmti, JNIEnv *jni_env, jthread t, jclass c){
	jint methodCount = 0;
	jmethodID *methods;
	
	char *methodName;
	char *methodSignature;
	char *methodGenericSignature;
	
	jvmtiError error;
	
	error = jvmti->GetClassMethods(c, &methodCount, &methods);
	//printf("GetClassMethods Error: %d\n", error);
	//printf("Method Count: %d\n", methodCount);
	
	for(int j = 0; j < methodCount; j++){
		//printf("Getting method name from method with index %d\n", j);
		//printf("MethodID: %d\n", methods[j]);
		error = jvmti->GetMethodName(methods[j], &methodName, &methodSignature, &methodGenericSignature);
		//printf("MethodName: %s\n", methodName);
		//printf("GetMethodName Error: %d\n", error);
		
		if(strcmp(methodName, "main") == 0){
			jvmti->SetBreakpoint(methods[j], 0);
			printf("Breakpoint set on main.\n");
		}
		
		jvmti->Deallocate(reinterpret_cast<unsigned char*>(methodName));
		jvmti->Deallocate(reinterpret_cast<unsigned char*>(methodSignature));
		jvmti->Deallocate(reinterpret_cast<unsigned char*>(methodGenericSignature));
	}
	
	jvmti->Deallocate(reinterpret_cast<unsigned char*>(methods));
}

void JNICALL callbackBreakpoint(jvmtiEnv *jvmti, JNIEnv* jni_env, jthread t, jmethodID method, jlocation location){
	jvmti->SuspendThread(t);
}