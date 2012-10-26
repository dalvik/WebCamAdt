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
   LOGI("### UdtTools start exit!");
   if(cmdSocket != NULL) {
   LOGI("### UdtTools 1111111111");
	if(cmdSocket->type == STUN_SOCK_TCP)
	{
		close(cmdSocket->sock);
	} else {
		UDT::close(cmdSocket->sock);
	}
   LOGI("### UdtTools 22222222");
	cmdSocket->sock = 0;
	free(cmdSocket);
   }
   if(videoSocket != NULL) {
	if(videoSocket->type == STUN_SOCK_TCP)
	{
		close(videoSocket->sock);
	} else {
		UDT::close(videoSocket->sock);
	}
	videoSocket->sock = 0;
	free(videoSocket);
      
   }
   if(audioSocket != NULL) {
	if(audioSocket->type == STUN_SOCK_TCP)
	{
		close(audioSocket->sock);
	} else {
		UDT::close(audioSocket->sock);
	}
	audioSocket->sock = 0;
	free(audioSocket);
   }  
   for(int i=0;i<length;i++) {
	StunSocket *cmd = socketArr[i].cmdSocket;
	char *tmp = socketArr[i].id;
   LOGI("### UdtTools aaaa");
	if(cmd != NULL && cmd->sock >0) {
   LOGI("### UdtTools bbbb");
	   if(cmd->type == STUN_SOCK_TCP)
	   {
	  	close(cmd->sock);
	   } else {
		UDT::close(cmd->sock);
	   }
	   cmd->sock = 0;
	   free(cmd);
	}
   LOGI("### UdtTools cccc");
	   if(cmd->type == STUN_SOCK_TCP)
	free(tmp);
    }
   LOGI("### UdtTools clearConnection over!");
}

void freeSocket() {
	LOGI("### UdtTools start free!");
   	StunSocket *socket;
	for(int i=0;i<3;i++) {
		socket = threeSocketArr[i];
		if(socket != NULL && socket->sock >0){
			if(socket->type == STUN_SOCK_TCP)
			{
				close(socket->sock);
			} else {
				UDT::close(socket->sock);
			}
		}
		free(socket);
		threeSocketArr[i] = NULL;
	}
	LOGI("### UdtTools end free!");
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
		if(tmp != NULL  && strcmp( tmp, id )==0 && 
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
	static int first = 1;

	if (first)
	{
		first = 0;
		monitor_init();
	}
	freeSocket();
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
	LOGI("### end monitorSocke, result %d,cmdsock value=%d,video socket value=%d, audio socket value=%d", ret,cmdSocket->sock,videoSocket->sock,audioSocket->sock);
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
	    LOGI("### end monitorCmdSocket res = %d, cmdSocket=%d", ret,threeSocketArrTmp[0]->sock);	
	}
        LOGI("### end monitorCmdSocket, result %d", ret);
	return ret;
}

extern "C" jint JNICALL Java_com_iped_ipcam_gui_UdtTools_freeCmdSocket(JNIEnv *env, jobject thiz)
{
	LOGI("### start freeCmdSocket");
    	StunSocket *cmd = socketArr[1].cmdSocket;
	if(cmd != NULL)
	{
		if(cmd != NULL && cmd->sock >0){
			if(cmd->type == STUN_SOCK_TCP)
			{
				close(cmd->sock);
			} else {
				UDT::close(cmd->sock);
			}
		}
		free(cmd);
		socketArr[1].cmdSocket = NULL;
		socketArr[1].id = NULL;
	}
        LOGI("### end freeCmdSocket");
	return 0;
}

extern "C" jint JNICALL Java_com_iped_ipcam_gui_UdtTools_sendCmdMsgById(JNIEnv *env, jobject thiz, jstring camId, jstring cmdName, jint cmdNameLength)
{	
	char* id = jstringToChar(env,camId);
	StunSocket *cmdSendSocket;
	cmdSendSocket = checkCmdSocketValid(id);
	if(cmdSendSocket->sock <= 0) {
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
	LOGI("### UdtTools send cmd name = %s send msg length = %d, Sock type =%d, Sock value=%d", cmd,res,cmdSendSocket->type,cmdSendSocket->sock);
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
	     free(tmp);
	     LOGI("### UdtTools recv error!");
	     return cmdSendSocket->sock;
	}
	LOGI("### UdtTools recv data length = %d, Socket Type = %d, socket value=%d", dataLength,cmdSendSocket->type,cmdSendSocket->sock);
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
//LOGI("UdtTools  1111 = %p", data);
//LOGI("UdtTools  2222 = %p", amrBuffer);
//LOGI("UdtTools  3333 = %d", amrBuffer[0]);
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
     	return -1;
    }
    env->SetByteArrayRegion(buffer, 0, dataLength,(jbyte*) buf);  
    //LOGI("### UdtTools recvVideoMsg result aaaaaaa %d", dataLength);
    return dataLength;
}
extern "C" jint JNICALL Java_com_iped_ipcam_gui_UdtTools_sendPTZMsg(JNIEnv *env, jobject thiz,jbyteArray buffer)
{

	jint len = env->GetArrayLength(buffer);
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

extern "C" void JNICALL Java_com_iped_ipcam_gui_UdtTools_free(JNIEnv *env, jobject thiz)
{
	LOGI("### UdtTools start free!");
   	StunSocket *socket;
	for(int i=0;i<3;i++) {
		socket = threeSocketArr[i];
		if(socket != NULL && socket->sock >0){
			if(socket->type == STUN_SOCK_TCP)
			{
				close(socket->sock);
			} else {
				UDT::close(socket->sock);
			}
			socket->sock = 0;
			socket = 0;
		}
	}
	LOGI("### UdtTools end free!");
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


