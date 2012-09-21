#include "jni.h"
#include <cerrno>
#include <cstddef>

#include <arpa/inet.h>
#include <netdb.h>
//#include <cstdlib>
#include <cstring>
#include <android/log.h> 


#include <fstream>


#define LOG_TAG "debug"
#define LOGI(fmt, args...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, fmt, ##args)
#define LOGD(fmt, args...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, fmt, ##args)
#define LOGE(fmt, args...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, fmt, ##args) 

extern "C" JNIEXPORT jint JNICALL Java_com_iped_ipcam_utils_UdtTools_initialize(JNIEnv *env, jobject thiz) {
	LOGI("--->webcam jni init success");
	//UDT::startup();
//	 printf("Hello World!\n");
	 
    return 0;
}

