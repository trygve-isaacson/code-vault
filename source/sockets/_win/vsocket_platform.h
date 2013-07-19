/*
Copyright c1997-2013 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.0
http://www.bombaydigital.com/
*/

#ifndef vsocket_platform_h
#define vsocket_platform_h

/** @file */

// This file is intended to be include *by* the generic vsocket.h to get
// any platform-specific declarations or includes.

#ifdef VCOMPILER_MSVC
    #pragma warning(disable: 6386)  // the library file doesn't past muster
#endif
#include <ws2tcpip.h> // For the WSA calls in enumerateNetworkInterfaces(), and addrinfo in vsocketbase.cpp when on Windows.
#ifdef VCOMPILER_MSVC
    #pragma warning(default: 6386)
#endif

// On Mac OS X, we disable SIGPIPE in VSocket::setDefaultSockOpt(), so these flags are 0.
// On Winsock, it is irrelevant so these flags are 0.
// For other Unix platforms, we specify it in the flags of each send()/recv() call via this parameter.
#define VSOCKET_DEFAULT_SEND_FLAGS 0
#define VSOCKET_DEFAULT_RECV_FLAGS 0

/*
There are a couple of Unix APIs we call that take a socklen_t parameter.
On Windows the parameter is defined as an int. The cleanest way of dealing
with this is to have our own conditionally-defined type here and use it there.
We do something similar in the _unix version of this file for HP-UX, which
also uses int instead of socklen_t.
*/
typedef int VSocklenT;

typedef SOCKET VSocketID;    ///< The platform-dependent definition of a socket identifier.
#define V_NO_SOCKET_ID_CONSTANT INVALID_SOCKET ///< Used internally to initialize kNoSocketID

// Winsock uses slightly incompatible socket APIs with different types. We can usually just
// typecast, and rather than have different versions of the code we can name the typecasts
// and declare them differently per platform. For Unix the casts usually are "nothing".
#define v_ioctlsocket ioctlsocket /* varargs make this easier than an inline function */
#define SelectSockIDTypeCast (int)
#define SendBufferPtrTypeCast (const char*)
#define RecvBufferPtrTypeCast (char*)
#define SendRecvByteCountTypeCast
#define SendRecvResultTypeCast
#define SetSockOptValueTypeCast (const char*)

// Used in VSocket BSD sock_addr structures, but not defined by Winsock headers.
#define in_port_t USHORT

namespace vault {
inline int closeSocket(VSocketID fd) { return ::closesocket(fd); }
}

#endif /* vsocket_platform_h */

