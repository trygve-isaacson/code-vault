/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.0
http://www.bombaydigital.com/
*/

#ifndef vsocket_platform_h
#define vsocket_platform_h

/** @file */

// This file is intended to be include *by* the generic vsocket.h to get
// any platform-specific declarations or includes.

#ifndef PP_Target_Carbon
    // Unix platform includes needed for platform socket implementation.
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <netinet/tcp.h>
#endif

#include <netdb.h>
#include <arpa/inet.h>

// On Mac OS X, we disable SIGPIPE in VSocket::setDefaultSockOpt(), so these flags are 0.
// On Winsock, it is irrelevant so these flags are 0.
// For other Unix platforms, we specify it in the flags of each send()/recv() call via this parameter.
#ifdef VPLATFORM_UNIX
    #define VSOCKET_DEFAULT_SEND_FLAGS MSG_NOSIGNAL
    #define VSOCKET_DEFAULT_RECV_FLAGS MSG_NOSIGNAL
#else
    #define VSOCKET_DEFAULT_SEND_FLAGS 0
    #define VSOCKET_DEFAULT_RECV_FLAGS 0
#endif

/*
There are a couple of Unix APIs we call that take a socklen_t parameter.
Well, on HP-UX the parameter is defined as an int. The cleanest way of dealing
with this is to have our own conditionally-defined type here and use it there.
*/
#ifdef VPLATFORM_UNIX_HPUX
    typedef int VSocklenT;
#else
    typedef socklen_t VSocklenT;
#endif

typedef int VSocketID;    ///< The platform-dependent definition of a socket identifier.
#define V_NO_SOCKET_ID_CONSTANT -1 ///< Used internally to initialize kNoSocketID

// Winsock uses slightly incompatible socket APIs with different types. We can usually just
// typecast, and rather than have different versions of the code we can name the typecasts
// and declare them differently per platform. For Unix the casts usually are "nothing".
#define v_ioctlsocket ioctl /* varargs make this easier than an inline function */
#define SelectSockIDTypeCast
#define SendBufferPtrTypeCast
#define RecvBufferPtrTypeCast
#define SendRecvByteCountTypeCast (VSizeType)
#define SendRecvResultTypeCast (int)
#define SetSockOptValueTypeCast

namespace vault {
inline int closeSocket(VSocketID fd) { return ::close(fd); }
}

#endif /* vsocket_platform_h */

