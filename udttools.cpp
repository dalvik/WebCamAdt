#include "jni.h"
//#include <utils/Log.h>
#include<android/log.h>
#include <udttools.h>

#define LOGI(fmt, args...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, fmt, ##args)
#define LOGD(fmt, args...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, fmt, ##args)
#define LOGE(fmt, args...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, fmt, ##args) 
/*
#define LOG_LEVEL 10
#define LOGI(level, ...) if (level <= LOG_LEVEL) {__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__);}
#define LOGE(level, ...) if (level <= LOG_LEVEL) {__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__);}
*/
#define LOG_TAG "webcam"

char* jstringToChar(JNIEnv* env, jstring jstr )
{
	const char *nativeString = env->GetStringUTFChars(jstr, 0); 
	int length = env->GetStringUTFLength(jstr); 
	char * tmp = (char*)malloc(length+1);
	memset(tmp,0,length+1);
	memcpy(tmp, nativeString, length);	
	env->ReleaseStringUTFChars(jstr, nativeString);
   return tmp;
} 

//video thread interrupt
void freeSocket() {
	LOGI("### UdtTools start free!");
	if(socket1) {
		socket_fusion_unref(socket1);
		socket1 = NULL;
   }
	LOGI("### UdtTools end free!");
}

/****************** new udt socket ********************/

extern "C" int JNICALL Java_com_iped_ipcam_gui_UdtTools_startSearch(JNIEnv *env, jobject thiz)
{	
	fetchCamIndex = 0;
	LOGI("### UdtTools startSearch");
	monitor_search_lan(NULL, NULL, NULL, NULL, NULL);
	LOGI("### UdtTools search over!");
    return 0;
}

extern "C" jstring JNICALL Java_com_iped_ipcam_gui_UdtTools_fetchCamId(JNIEnv *env, jobject thiz)
{
	jstring camIdTmp;
	stun_lock_enter(&cam_search_mutex);
  	if (cam_lan_id[fetchCamIndex][0] != '\0' && fetchCamIndex <64)
    {
		camIdTmp = env->NewStringUTF(cam_lan_id[fetchCamIndex]); 
		fetchCamIndex++;
		LOGI("## UdtTools_fetch index = %d",fetchCamIndex);
		stun_lock_leave(&cam_search_mutex);
		return camIdTmp;  
	}
	stun_lock_leave(&cam_search_mutex);
	return NULL;
}

extern "C" void JNICALL Java_com_iped_ipcam_gui_UdtTools_stopSearch(JNIEnv *env, jobject thiz)
{	
	LOGI("### UdtTools stopSearch");	
	fetchCamIndex = 0;
	monitor_search_stop();
	LOGI("### UdtTools stopSearch over!");
}

extern "C" jstring JNICALL Java_com_iped_ipcam_gui_UdtTools_monitorSocket(JNIEnv *env, jobject thiz, jstring camId)
{
	static int first = 1;

	if (first)
	{
		first = 0;
		monitor_init();
	}
	freeSocket();
	LOGI("### start monitorSocke");
	char* idTmp = jstringToChar(env,camId);
	id = idTmp;
	socket1 = monitor_socket_fusion(idTmp);
	while (1)
    {
		LOGI("### wait to connection.");
        stun_sleep_ms(100);
        if (socket_fusion_is_usable(socket1) || socket_fusion_is_done(socket1))
        {
            break;
        }
    }
	if (socket_fusion_is_usable(socket1))
    {
		LOGI("### connection success. %p", socket1);
		return getOK(env);
	}
	LOGI("### connection faliled.");
	return getError(env,socket1);
}

extern "C" jstring JNICALL Java_com_iped_ipcam_gui_UdtTools_monitorCmdSocket(JNIEnv *env, jobject thiz, jstring camId,jstring rand)
{
	LOGI("### start config monitorCmdSocket");
	char* idTmp = jstringToChar(env,camId);
	if(id == NULL || !strcmp( idTmp, id )==0) {
		socket2 = monitor_socket_fusion(idTmp);
		while (1)
		{
			LOGI("### wait config to connection.");
			stun_sleep_ms(100);
			if (socket_fusion_is_usable(socket2) || socket_fusion_is_done(socket2))
			{
				break;
			}
		}
		if (socket_fusion_is_usable(socket2))
		{
			LOGI("### configf connection success.%p", socket2);
			return getOK(env);
		}
	} else {
		if (socket_fusion_is_usable(socket1))
		{
			LOGI("### configf connection success.%p", socket2);
			return getOK(env);
		} else {
			Java_com_iped_ipcam_gui_UdtTools_monitorSocket(env, thiz,camId);
		}
	}
	LOGI("### config connection faliled.");
	return getError(env, socket2);
}

extern "C" jint JNICALL Java_com_iped_ipcam_gui_UdtTools_freeCmdSocket(JNIEnv *env, jobject thiz)
{
	LOGI("### start freeCmdSocket");
	if(socket2) {
		socket_fusion_unref(socket2);
        socket2 = NULL;
	}
	LOGI("### end freeCmdSocket");
	return 0;
}

extern "C" jint JNICALL Java_com_iped_ipcam_gui_UdtTools_sendCmdMsg(JNIEnv *env, jobject thiz, jstring cmdName, jint cmdNameLength)
{	
	LOGI("### UdtTools send cmd msg !");
	char * cmd = jstringToChar(env,cmdName);
	
	int res = 0;
	if (socket_fusion_is_usable(socket1)){
		res = socket_fusion_sendmsg(socket1, 0, cmd, cmdNameLength);
	}
	LOGI("### UdtTools send cmd name = %s send msg length = %d", cmd,res);
	free(cmd);
	return res;
}

extern "C" jint JNICALL Java_com_iped_ipcam_gui_UdtTools_sendCmdMsgById(JNIEnv *env, jobject thiz, jstring camId, jstring cmdName, jint cmdNameLength)
{	
	char* tmp = jstringToChar(env,camId);
	LOGI("### UdtTools send cmd by id");
	char * cmd = jstringToChar(env,cmdName);
	int res = 0;
	if(id != NULL && strcmp( tmp, id )==0) {
		if (socket_fusion_is_usable(socket1)) {
			res = socket_fusion_sendmsg(socket1, 0, cmd, cmdNameLength);
		}else {
			LOGI("### UdtTools send cmd socket1 is invalid! %p", socket1);
		}
	} else {
		if (socket_fusion_is_usable(socket2)) {
			res = socket_fusion_sendmsg(socket2, 0, cmd, cmdNameLength);
		}else {
			LOGI("### UdtTools send cmd socket2 is invalid! %p", socket2);
		}
	}
	LOGI("### UdtTools send cmd name = %s send msg length = %d", cmd,res);
	free(cmd);
	free(tmp);
	return res;
}


extern "C" jint JNICALL Java_com_iped_ipcam_gui_UdtTools_recvCmdMsg(JNIEnv *env, jobject thiz, jbyteArray buffer, jint bufferLength)
{
	char * tmp = (char*)malloc(bufferLength); 
	LOGI("### UdtTools wait for recv cmd ...");
	int dataLength = 0;
	if (socket_fusion_is_usable(socket1)) {
		dataLength = socket_fusion_recvmsg(socket1, 0, tmp, bufferLength);
	}
	
	if(dataLength < 0) {
	     free(tmp);
	     LOGI("### UdtTools recv error!");
	     return -1;
	}
	LOGI("### UdtTools recv data length = %d,%s", dataLength,tmp);
	env->SetByteArrayRegion(buffer, 0, dataLength,(jbyte*) tmp);  
	free(tmp);
	return dataLength; 
}

extern "C" jint JNICALL Java_com_iped_ipcam_gui_UdtTools_recvCmdMsgById(JNIEnv *env, jobject thiz, jstring camId, jbyteArray buffer, jint bufferLength)
{
	LOGI("### UdtTools start recv cmd msg by id!");
	char* idTmp = jstringToChar(env,camId);
	char * tmp = (char*)malloc(bufferLength); 
	LOGI("### UdtTools wait for recv cmd ...");
	int dataLength = 0;
	if(id != NULL && strcmp( idTmp, id )==0) {
		if (socket_fusion_is_usable(socket1)) {
			dataLength = socket_fusion_recvmsg(socket1, 0, tmp, bufferLength);
		}
	}else{
		if (socket_fusion_is_usable(socket2)) {
			dataLength = socket_fusion_recvmsg(socket2, 0, tmp, bufferLength);
		}
	}
	if(dataLength < 0) {
	     free(tmp);
	     LOGI("### UdtTools recv error!");
	     return -1;
	}
	LOGI("### UdtTools recv data length = %d", dataLength);
	env->SetByteArrayRegion(buffer, 0, dataLength,(jbyte*) tmp);  
	free(tmp);
	return dataLength;	
}

char recvAudioBuf[1600];

extern "C" jint JNICALL Java_com_iped_ipcam_gui_UdtTools_recvAudioMsg(JNIEnv *env, jobject thiz,jint smallAudioBufferLength, jbyteArray buffer, jint bigAudioBufferLength)
{
	int pos=0;
	int dataLength;
	//char recvAudioBuf[smallAudioBufferLength];
	if(socket_fusion_is_usable(socket1)) 
	{
		dataLength = socket_fusion_recv(socket1, 2, recvAudioBuf, smallAudioBufferLength);
	} 
	if(dataLength < 0) { 
	    LOGI("UdtTools recvAudioMsg over");
	    return pos = -1;
	}
    env->SetByteArrayRegion(buffer, 0, dataLength,(jbyte*) recvAudioBuf);  
    return dataLength;	
}

extern "C" jint JNICALL Java_com_iped_ipcam_gui_UdtTools_sendAudioMsg(JNIEnv *env, jobject thiz,jbyteArray amrBuffer, jint amrBufferLength)
{
        char* data = (char*)malloc(amrBufferLength);
        env->GetByteArrayRegion (amrBuffer, (jint)0, (jint)amrBufferLength, (jbyte*)data);
        int dataLength;
        //char recvAudioBuf[smallAudioBufferLength];
        if(socket_fusion_is_usable(socket1)) 
        {
                dataLength = socket_fusion_send(socket1, 2, data, amrBufferLength);
        }
        //LOGI("UdtTools send audio Msg length = %d", dataLength);
        free(data);
        return dataLength;
}

extern "C" jint JNICALL Java_com_iped_ipcam_gui_UdtTools_recvVideoMsg(JNIEnv *env, jobject thiz,jbyteArray buffer, int bufferLength)
{
    int dataLength;
    if(socket_fusion_is_usable(socket1)) 
    {
		dataLength = socket_fusion_recv(socket1,1,buf, 4096);
    } else {
		dataLength = -1;
	}
    //LOGI("### UdtTools recvVideoMsg result %d", dataLength);
    if(dataLength < 0) {
	LOGI("UdtTools recvVideoMsg over");
     	return -1;
    }
    env->SetByteArrayRegion(buffer, 0, dataLength,(jbyte*) buf);  
    //LOGI("### UdtTools recvVideoMsg result aaaaaaa %d", dataLength);
    return dataLength;
}
extern "C" jint JNICALL Java_com_iped_ipcam_gui_UdtTools_sendPTZMsg(JNIEnv *env, jobject thiz,jbyteArray buffer)
{

	jint len = env->GetArrayLength(buffer);
	char* data = (char*)malloc(len);
	env->GetByteArrayRegion (buffer, (jint)0, (jint)len, (jbyte*)data);
	int res = 0;
	if(socket_fusion_is_usable(socket1)) {
		res =  socket_fusion_sendmsg(socket1,0, data, len);
	}
	free(data);
	LOGI("### UdtTools send PTZ msg over!");
	return res;
}

extern "C" void JNICALL Java_com_iped_ipcam_gui_UdtTools_close(JNIEnv *env, jobject thiz)
{
	if(socket1) {
		socket_fusion_unref(socket1);
		socket1 = NULL;
	}
    if(socket2) {
		socket_fusion_unref(socket2);
		socket2 = NULL;
    }   
	if(id) {
		free(id);
		id = NULL;
	}
	LOGI("### app exit and release all socket!");
}
//video thread interrupt free socket1
extern "C" void JNICALL Java_com_iped_ipcam_gui_UdtTools_exit(JNIEnv *env, jobject thiz)
{
	freeSocket();
}
/*
extern "C" void JNICALL Java_com_iped_ipcam_gui_UdtTools_freeConnectionById(JNIEnv *env, jobject thiz,jstring camId)
{
	LOGI("### UdtTools start free!");
	char* idTmp = jstringToChar(env,camId);
	if((id == NULL) || (id != NULL && !strcmp( idTmp, id )==0)) {
		if(socket2) {
			socket_fusion_unref(socket2);
			socket2 = NULL;
		}
	}
	LOGI("### UdtTools end free!");
}*/


extern "C" void JNICALL Java_com_iped_ipcam_gui_UdtTools_deleteRef(JNIEnv *env) {
	deleteRefer(env);
}

extern "C" void JNICALL Java_com_iped_ipcam_gui_UdtTools_startUp(JNIEnv *env, jobject thiz)
{
	UDT::startup();
	LOGI("### UdtTools start up!");
}

extern "C" void JNICALL Java_com_iped_ipcam_gui_UdtTools_cleanUp(JNIEnv *env, jobject thiz)
{
	 UDT::cleanup();
	 LOGI("### UdtTools clean up!");
}

jstring getOK(JNIEnv *env){
	result = env->NewStringUTF("OK");
	LOGI("### getOK -- %s", result);
	deleteRefer(env);
	return result;
}

jstring getError(JNIEnv *env, SocketFusion *socket) {
	char tmp[20]; 
	memset(tmp,0,20); 
	int index;
	index = sprintf(tmp,"%s","[");
	index += sprintf(tmp+index,"%d",socket->lan_status);
	index += sprintf(tmp+index,"%s",",");
	index += sprintf(tmp+index,"%d",socket->wan_status);
	index += sprintf(tmp+index,"%s",",");
	index += sprintf(tmp+index,"%d",socket->relay_status);
	index += sprintf(tmp+index,"%s","]");
	result = env->NewStringUTF(tmp);
	
	LOGI("### get error info %s", tmp);
	deleteRefer(env);
	return result;
}

void deleteRefer(JNIEnv *env) {
	env->DeleteLocalRef(result);
}