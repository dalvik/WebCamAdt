#include <arpa/inet.h>
#include <netdb.h>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <udt.h> 

using namespace std;

SocketFusion *socket1;

SocketFusion *socket2;

char* id = NULL;

char buf[4096];

int timeoutCount = 0;

int fetchCamIndex;

jstring result = NULL;

jstring getOK(JNIEnv *env);

jstring getError(JNIEnv *env,SocketFusion *socket);

void deleteRefer(JNIEnv *env);
