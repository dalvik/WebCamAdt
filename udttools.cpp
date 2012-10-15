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

StunSocket *threeSocketArrTmp[3];

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
   for(int i=0;i<length;i++) {
	StunSocket *cmd = socketArr[i].cmdSocket;
	char *tmp = socketArr[i].id;
	if(tmp>0) {
	   free(tmp);
	}
	if(cmd>0 && cmd->sock >0) {
	   if(cmd->type == STUN_SOCK_TCP)
	   {
	  	close(cmd->sock);
	   } else {
		UDT::close(cmd->sock);
	   }
	   cmd->sock = 0;
	   cmd = 0;
	}
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
	StunSocket *cmdSocketTmp;
	for(int i=0;i<length;i++) {
		cmdSocketTmp = socketArr[i].cmdSocket;
		char *tmp = socketArr[i].id;
		if(tmp != 0 && strcmp( tmp, id )==0 && 
			cmdSocketTmp != NULL && cmdSocketTmp>0 && cmdSocketTmp->sock >0){
		  	return cmdSocketTmp; 
		}
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

extern "C" jint JNICALL Java_com_iped_ipcam_gui_UdtTools_monitorSocket(JNIEnv *env, jobject thiz, jstring camId)
{
	LOGI("### start monitorSocke");
	char* idTmp = jstringToChar(env,camId);
	int ret = monitor_socket_avc(idTmp, threeSocketArr);
	if(ret < 0) {
	    threeSocketArr[0] = NULL; threeSocketArr[1] = NULL; threeSocketArr[2] = NULL;
	}else {
	    cmdSocket = threeSocketArr[0];
	    videoSocket = threeSocketArr[1];
	    audioSocket = threeSocketArr[2];
	    socketArr[0].id = idTmp;
	    socketArr[0].cmdSocket = cmdSocket;
	}
	LOGI("### end monitorSocke, result %d", ret);
	return ret;
}

extern "C" jint JNICALL Java_com_iped_ipcam_gui_UdtTools_monitorCmdSocket(JNIEnv *env, jobject thiz, jstring camId,jstring rand)
{
	LOGI("### start monitorCmdSocket");
	char* idTmp = jstringToChar(env,camId);
	int ret = monitor_socket_avc(idTmp, threeSocketArrTmp);
	if(ret < 0) {
	    threeSocketArrTmp[0] = NULL; threeSocketArrTmp[1] = NULL; threeSocketArrTmp[2] = NULL;
	}else{
	    socketArr[1].id = idTmp;
	    socketArr[1].cmdSocket = threeSocketArrTmp[0];
	    LOGI("### end monitorCmdSocket, result %d", ret);
	}
	return ret;
}

extern "C" jint JNICALL Java_com_iped_ipcam_gui_UdtTools_sendCmdMsgById(JNIEnv *env, jobject thiz, jstring camId, jstring cmdName, jint cmdNameLength)
{	
	char* id = jstringToChar(env,camId);
	StunSocket *cmdSendSocket;
	cmdSendSocket = checkCmdSocketValid(id);
	if(cmdSendSocket->sock <= 0) {
		//free(id);
		LOGI("### UdtTools send cmd socket is invalid!");
		return cmdSendSocket->sock;
	}
	char * cmd = jstringToChar(env,cmdName);
	int res = 0;
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
	char* id = jstringToChar(env,camId);
	StunSocket *cmdSendSocket;
	cmdSendSocket = checkCmdSocketValid(id);
	if(cmdSendSocket->sock <= 0) {
		//free(id);
		LOGI("### UdtTools recv cmd socket is invalid!  error info = %i", cmdSendSocket->sock);
		return cmdSendSocket->sock;
	}
	char * tmp = (char*)malloc(bufferLength); 
	LOGI("### UdtTools wait for recv cmd ...");
	int dataLength = 0;
	if(cmdSendSocket->type == STUN_SOCK_TCP) {
		dataLength = stun_tcp_recvmsg(cmdSendSocket->sock,tmp,bufferLength,NULL, NULL);
	} else {
		dataLength = stun_recvmsg(cmdSendSocket->sock,tmp,bufferLength,NULL, NULL);
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

extern "C" jint JNICALL Java_com_iped_ipcam_gui_UdtTools_sendAudioMsg(JNIEnv *env, jobject thiz,jshortArray amrBuffer, int amrBufferLength)
{
	short* data = (short*)malloc(amrBufferLength);
LOGI("UdtTools  = %p", data);
LOGI("UdtTools  = %p", amrBuffer);
LOGI("UdtTools  = %d", amrBuffer[0]);
	env->GetShortArrayRegion(amrBuffer, 0, amrBufferLength, data);
	//unsigned char* sendAudio = (unsigned char*)malloc(amrBufferLength*2);
	//short tmp;
	//int index = 0;
	//for(int i=0;i<amrBufferLength;i++)
	//{
	   //tmp = data[i];
//LOGI("UdtTools send audio amrBufferLength = %d,%d", tmp,(tmp & 0xFF));
//LOGI("UdtTools  = %d,%d,%p", amrBufferLength,index,sendAudio);
//index+=2;
	   //sendAudio[index++] = tmp & 0x00FF;	   
	  // sendAudio[index++] = tmp & 0xFF00;	   
	//}

	int dataLength;
	//char recvAudioBuf[smallAudioBufferLength];
	//if(audioSocket->type == STUN_SOCK_TCP) 
	//{
		//dataLength = send(audioSocket->sock, sendAudio, index, 0);
	//} else {
		//dataLength = UDT::send(audioSocket->sock, sendAudio, index, 0);
	//}
	//LOGI("UdtTools send audio Msg length = %d", dataLength);
	free(data);  
	//free(sendAudio); 
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


