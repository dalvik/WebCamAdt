#include "jni.h"
//#include <utils/Log.h>

#define LOGI(fmt, args...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, fmt, ##args)
#define LOGD(fmt, args...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, fmt, ##args)
#define LOGE(fmt, args...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, fmt, ##args) 

#include<android/log.h>

#define LOG_TAG "webcam"

#include <arpa/inet.h>
#include <netdb.h>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <udt.h> 

using namespace std;
 
UDTSOCKET UDTSocket;

UDTSOCKET UDTAudioSocket;

StunSocket *cmdSocket;

StunSocket *audioSocket;

StunSocket *videoSocket;

const int length = 2;

CmdSocket socketArr[length];

StunSocket *threeSocketArr[3];

char* deviceId;

char buf[1024];

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

void clearConnection() {
   //LOGI("### UdtTools start exit!");
   if(cmdSocket>0) {
	if(cmdSocket->type == STUN_SOCK_TCP)
	{
		close(cmdSocket->sock);
	} else {
		UDT::close(cmdSocket->sock);
	}
	cmdSocket->sock = 0;
	cmdSocket = 0;
   }
   if(videoSocket>0) {
	if(videoSocket->type == STUN_SOCK_TCP)
	{
		close(videoSocket->sock);
	} else {
		UDT::close(videoSocket->sock);
	}
	videoSocket->sock = 0;
	videoSocket = 0;
      
   }
   if(audioSocket>0) {
	if(audioSocket->type == STUN_SOCK_TCP)
	{
		close(audioSocket->sock);
	} else {
		UDT::close(audioSocket->sock);
	}
	audioSocket->sock = 0;
	audioSocket = 0;
      
   }  
   LOGI("### UdtTools clearConnection over!");
}

// release connected socket
extern "C" void JNICALL Java_com_iped_ipcam_gui_UdtTools_release(JNIEnv *env, jobject thiz)
{
   if(UDTSocket>0) {
      UDT::close(UDTSocket);
   }
}

/****************** new udt socket ********************/

int fetchCamIndex;

StunSocket * checkCmdSocketValid(char *id) {
	StunSocket *cmdSocketTmp = threeSocketArr[0];
	if(deviceId != 0 && strcmp(deviceId, id )==0 && cmdSocketTmp != NULL && cmdSocketTmp->sock >0){	
	  	return cmdSocketTmp; 
	}
	return 0;
}

extern "C" int JNICALL Java_com_iped_ipcam_gui_UdtTools_startSearch(JNIEnv *env, jobject thiz)
{	
	LOGI("### UdtTools startSearch");
	fetchCamIndex = 0;
	monitor_search_lan(NULL, NULL, NULL, NULL);
	LOGI("### UdtTools search over!");
        return 0;
}

extern "C" jstring JNICALL Java_com_iped_ipcam_gui_UdtTools_fetchCamId(JNIEnv *env, jobject thiz)
{
	jstring camIdTmp;
  	if (cam_lan_id[fetchCamIndex][0] != '\0' && fetchCamIndex <64)
        {
		camIdTmp = env->NewStringUTF(cam_lan_id[fetchCamIndex]); 
		fetchCamIndex++;
		LOGI("## UdtTools_fetch index = %d",fetchCamIndex);
		return camIdTmp;  
	}
	return NULL;
}

extern "C" void JNICALL Java_com_iped_ipcam_gui_UdtTools_stopSearch(JNIEnv *env, jobject thiz)
{	
	LOGI("### UdtTools stopSearch");	
	fetchCamIndex = 0;
	monitor_search_stop();
	LOGI("### UdtTools stopSearch over!");

}

int monitorSocket(char *id) {
	LOGI("### start monitorSocke");
	int ret = monitor_socket_avc(id, threeSocketArr);
	if(ret < 0) {
	    threeSocketArr[0] = NULL; threeSocketArr[1] = NULL; threeSocketArr[2] = NULL;
	}
	LOGI("### end monitorSocke, result %d", ret);
LOGI("### UdtTools init ---- %d,%d,%d", threeSocketArr[0]->sock,threeSocketArr[1]->sock,threeSocketArr[2]->sock);
	return ret;
}


extern "C" jint JNICALL Java_com_iped_ipcam_gui_UdtTools_monitorSocket(JNIEnv *env, jobject thiz, jstring camId)
{
	deviceId = jstringToChar(env,camId);
	return monitorSocket(deviceId);
}

extern "C" jint JNICALL Java_com_iped_ipcam_gui_UdtTools_monitorCmdSocket(JNIEnv *env, jobject thiz, jstring camId,jstring rand)
{
	char* id = jstringToChar(env,camId);
	return monitorSocket(id);
}

extern "C" jint JNICALL Java_com_iped_ipcam_gui_UdtTools_sendCmdMsgById(JNIEnv *env, jobject thiz, jstring camId, jstring cmdName, jint cmdNameLength)
{	
	//char* id = jstringToChar(env,camId);
	StunSocket *cmdSendSocket = threeSocketArr[0];
	//cmdSendSocket = checkCmdSocketValid(id);
	if(cmdSendSocket->sock <= 0) {
		//free(id);
		LOGI("### UdtTools send cmd socket is invalid!");
		return cmdSendSocket->sock;
	}
	char * cmd = jstringToChar(env,cmdName);
	int res = 0;
	LOGI("### UdtTools send ---- %d,%d,%d", threeSocketArr[0]->sock,threeSocketArr[1]->sock,threeSocketArr[2]->sock);
	if(cmdSendSocket->type == STUN_SOCK_TCP) {
		res =  stun_tcp_sendmsg(cmdSendSocket->sock, cmd, cmdNameLength);
	} else {
		res = stun_sendmsg(cmdSendSocket->sock, cmd, cmdNameLength);
	}
	LOGI("### UdtTools send cmd name = %s send msg length = %d", cmd,res);
	//free(id);
	free(cmd);
	return res;
}


extern "C" jint JNICALL Java_com_iped_ipcam_gui_UdtTools_recvCmdMsgById(JNIEnv *env, jobject thiz, jstring camId, jbyteArray buffer, jint bufferLength)
{
	LOGI("### UdtTools start recv cmd msg!");
	//char* id = jstringToChar(env,camId);
	StunSocket *cmdSendSocket = threeSocketArr[0];
	//cmdSendSocket = checkCmdSocketValid(id);
	if(cmdSendSocket->sock <= 0) {
		//free(id);
		LOGI("### UdtTools recv cmd socket is invalid!  error info = %i", cmdSendSocket->sock);
		return cmdSendSocket->sock;
	}
	char * tmp = (char*)malloc(bufferLength); 
	LOGI("### UdtTools wait for recv cmd ...");
	int dataLength = 0;
LOGI("### UdtTools recv ----%d,%d,%d", threeSocketArr[0]->sock,threeSocketArr[1]->sock,threeSocketArr[2]->sock);
	if(cmdSendSocket->type == STUN_SOCK_TCP) {
		dataLength = stun_tcp_recvmsg(cmdSendSocket->sock,tmp,bufferLength,NULL, NULL);
LOGI("### UdtTools ---==== %d,",dataLength);
	} else {
		dataLength = stun_recvmsg(cmdSendSocket->sock,tmp,bufferLength,NULL, NULL);
LOGI("### UdtTools --- %d,",dataLength);
	}
	
	if(dataLength <= 0) {
	     //free(id);
	     free(tmp);
	     LOGI("### UdtTools recv error!");
	     return cmdSendSocket->sock;
	}
	LOGI("### UdtTools recv cmd success!");
	env->SetByteArrayRegion(buffer, 0, dataLength,(jbyte*) tmp);  
	//free(id);
	free(tmp);
	return dataLength;	
}

char recvAudioBuf[1024];

extern "C" jint JNICALL Java_com_iped_ipcam_gui_UdtTools_recvAudioMsg(JNIEnv *env, jobject thiz,jint smallAudioBufferLength, jbyteArray buffer, jint bigAudioBufferLength)
{
	int pos=0;
	int dataLength;
	//char recvAudioBuf[smallAudioBufferLength];
	if(audioSocket->type == STUN_SOCK_TCP) 
	{
		dataLength = recv(audioSocket->sock, recvAudioBuf, smallAudioBufferLength,0);
	} else {
		dataLength = UDT::recv(audioSocket->sock, recvAudioBuf, smallAudioBufferLength,0);
	}
	if(dataLength <= 0) { 
	    LOGI("UdtTools recvAudioMsg over");
	    return pos = -1;
	}
	//LOGI("UdtTools recv Audio Msg length = %d,%d,%d", dataLength,recvAudioBuf[1],recvAudioBuf[2]);
	//memcpy(audioBuf+pos,recvAudioBuf,dataLength);
	//pos+=dataLength;

        env->SetByteArrayRegion(buffer, 0, dataLength,(jbyte*) recvAudioBuf);  
       return dataLength;	
}

extern "C" jint JNICALL Java_com_iped_ipcam_gui_UdtTools_sendAudioMsg(JNIEnv *env, jobject thiz,jbyteArray amrBuffer, jint amrBufferLength)
{
	char* data = (char*)malloc(amrBufferLength);
	env->GetByteArrayRegion (amrBuffer, (jint)0, (jint)amrBufferLength, (jbyte*)data);
	int dataLength;
	//char recvAudioBuf[smallAudioBufferLength];
	if(audioSocket->type == STUN_SOCK_TCP) 
	{
		dataLength = send(audioSocket->sock, data, amrBufferLength, 0);
	} else {
		dataLength = UDT::send(audioSocket->sock, data, amrBufferLength, 0);
	}
	//LOGI("UdtTools send audio Msg length = %d", dataLength);
	free(data);  
        return dataLength;	
}

extern "C" jint JNICALL Java_com_iped_ipcam_gui_UdtTools_recvVideoMsg(JNIEnv *env, jobject thiz,jbyteArray buffer, int bufferLength)
{
    int dataLength;
    if(videoSocket->type == STUN_SOCK_TCP) 
    {
	dataLength = recv(videoSocket->sock,buf, 1024, 0);
    } else {
	dataLength = UDT::recv(videoSocket->sock,buf,1024,0);
    }
    
    //LOGI("### UdtTools recvVideoMsg result %d", dataLength);
    if(dataLength <= 0) {
	LOGI("UdtTools recvVideoMsg over");
     	return videoSocket->sock;
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
	if(cmdSocket->type == STUN_SOCK_TCP) {
		res =  stun_tcp_sendmsg(cmdSocket->sock, data, len);
	} else {
		res = stun_sendmsg(cmdSocket->sock, data, len);
	}
	free(data);
	LOGI("### UdtTools send PTZ msg over!");
	return res;
}

extern "C" void JNICALL Java_com_iped_ipcam_gui_UdtTools_close(JNIEnv *env, jobject thiz)
{
	if(socketArr[1].cmdSocket > 0){
		if(socketArr[1].cmdSocket->lan == STUN_SOCK_TCP){
			close(socketArr[1].cmdSocket->sock);
		} else {
			UDT::close(socketArr[1].cmdSocket->sock);
		}
		socketArr[1].id = 0;
		socketArr[1].cmdSocket = 0;
		LOGI("### UdtTools release cmd socket!");
	}
}

extern "C" void JNICALL Java_com_iped_ipcam_gui_UdtTools_exit(JNIEnv *env, jobject thiz)
{
   clearConnection();
}

extern "C" void JNICALL Java_com_iped_ipcam_gui_UdtTools_startUp(JNIEnv *env, jobject thiz)
{
	UDT::startup();
	monitor_init();
	LOGI("### UdtTools start up!");
}

extern "C" void JNICALL Java_com_iped_ipcam_gui_UdtTools_cleanUp(JNIEnv *env, jobject thiz)
{
	 UDT::cleanup();
	 LOGI("### UdtTools clean up!");
}


extern "C" jint JNICALL Java_com_iped_ipcam_gui_UdtTools_checkCmdSocketEnable(JNIEnv *env, jobject thiz, jstring camId)
{
	char* id = jstringToChar(env,camId);//length
	StunSocket *cmd = checkCmdSocketValid(id);
	if(cmd == NULL || cmd->sock<0) {
		free(id);
		return -1;
	}
	free(id);
	return 1;
}


