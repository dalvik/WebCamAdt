# A simple test for the minimal standard C++ library
#

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_LDLIBS := libudt.so

LOCAL_LDLIBS += -L$(SYSROOT)/usr/lib -llog

LOCAL_CPPFLAGS += -fexceptions

LOCAL_MODULE := RecvFile 

LOCAL_MODULE_TAGS := libRecvFile
  
# Also need the JNI headers.
LOCAL_C_INCLUDES += \
	$(JNI_H_INCLUDE)

LOCAL_STATIC_LIBRARIES := libm

LOCAL_SHARED_LIBRARIES := \
	libutils 

LOCAL_SRC_FILES :=  \
		udttools.cpp\
		udt4/app/stun.cpp \
		udt4/app/stun-monitor.cpp\
		miniupnpc-1.7.20120830/miniwget.c \
		miniupnpc-1.7.20120830/minixml.c \
		miniupnpc-1.7.20120830/igd_desc_parse.c \
		miniupnpc-1.7.20120830/minisoap.c \
		miniupnpc-1.7.20120830/miniupnpc.c \
		miniupnpc-1.7.20120830/upnpreplyparse.c \
		miniupnpc-1.7.20120830/upnpcommands.c \
		miniupnpc-1.7.20120830/upnperrors.c \
		miniupnpc-1.7.20120830/connecthostport.c \
		miniupnpc-1.7.20120830/portlistingparse.c \
		miniupnpc-1.7.20120830/receivedata.c \
		miniupnpc-1.7.20120830/minissdpc.c \
		miniupnpc-1.7.20120830/upnpc.c


LOCAL_CFLAGS := -DMINIUPNPC_SET_SOCKET_TIMEOUT -DMINIUPNPC_GET_SRC_ADDR -D_BSD_SOURCE -D_POSIX_C_SOURCE=1 
 
LOCAL_PRELINK_MODULE := false

include $(BUILD_SHARED_LIBRARY)
