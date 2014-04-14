/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.1
http://www.bombaydigital.com/
*/

/** @file */

#include "vsocket.h"

#include "vexception.h"

#include <signal.h>

#ifdef sun
#include <sys/proc.h>
#endif

#include <sys/ioctl.h>
#include <ifaddrs.h>

// static
bool VSocket::_platform_staticInit() {
    //lint -e421 -e923 " Caution -- function 'signal(int, void (*)(int))' is considered dangerous [MISRA Rule 123]"
    ::signal(SIGPIPE, SIG_IGN);

    // Should log the error if the call failed.

    return true;
}

// static
VNetworkInterfaceList VSocket::_platform_enumerateNetworkInterfaces() {
    VNetworkInterfaceList interfaces;
    struct ifaddrs* interfacesDataPtr = NULL;
    int result = ::getifaddrs(&interfacesDataPtr);
    if (result != 0) {
        throw VStackTraceException(VSystemError::getSocketError(), VSTRING_FORMAT("VSocket::enumerateNetworkInterfaces: getifaddrs() failed with result %d.", result));
    }

    struct ifaddrs* intfPtr = interfacesDataPtr;
    while (intfPtr != NULL) {
        if (intfPtr->ifa_addr != NULL) {
            VNetworkInterfaceInfo info;
            info.mFamily = intfPtr->ifa_addr->sa_family;
            info.mName.copyFromCString(intfPtr->ifa_name);
            // AF_INET6 will work just fine here, too. But hold off until we can verify callers can successfully use IPV6 address strings to listen, connect, etc.
            if ((info.mFamily == AF_INET) && (info.mName != "lo0")) { // Internet interfaces only, and skip the loopback address.
                info.mAddress.preflight(255);
                char* buffer = info.mAddress.buffer();
                ::inet_ntop(intfPtr->ifa_addr->sa_family, &((struct sockaddr_in*) intfPtr->ifa_addr)->sin_addr, buffer, 255);
                info.mAddress.postflight((int) ::strlen(buffer));

                // Check for "lo0" above should filter out 127.x.x.x (loopback addresses), but in case it doesn't, check it.
                if (! info.mAddress.startsWith("127.")) {
                    interfaces.push_back(info);
                }
            }
        }

        intfPtr = intfPtr->ifa_next;
    }

    ::freeifaddrs(interfacesDataPtr);
    return interfaces;
}

static const int MAX_ADDRSTRLEN = V_MAX(INET_ADDRSTRLEN, INET6_ADDRSTRLEN);
// static
VString VSocket::_platform_addrinfoToIPAddressString(const VString& hostName, const struct addrinfo* info) {
    void* addr;
    if (info->ai_family == AF_INET) {
        addr = (void*) &(((struct sockaddr_in*)info->ai_addr)->sin_addr);
    } else if (info->ai_family == AF_INET6) {
        addr = (void*) &(((struct sockaddr_in6*)info->ai_addr)->sin6_addr);
    } else {
        // We don't know how to access the addr for other family types. They could conceivably be added.
        throw VException(VSTRING_FORMAT("VSocket::_platform_addrinfoToIPAddressString(%s): An invalid family (%d) other than AF_INET or AF_INET6 was specified.", hostName.chars(), info->ai_family));
    }

    VString result;
    result.preflight(MAX_ADDRSTRLEN);
    const char* buf = ::inet_ntop(info->ai_family, addr, result.buffer(), MAX_ADDRSTRLEN);
    if (buf == NULL) {
        throw VException(VSystemError::getSocketError(), VSTRING_FORMAT("VSocket::_platform_addrinfoToIPAddressString(%s): inet_ntop() failed.", hostName.chars()));
    }
    result.postflight((int) ::strlen(buf));

    return result;
}

// static
bool VSocket::_platform_isSocketIDValid(VSocketID socketID) {
    // On Unix:
    // -1 is typical error return value from ::socket()
    // Also, FD_SETSIZE is max num open sockets, sockid over that is sign of a big problem, and would cause FD_SET() during read() to fail.
    return (socketID >= 0) && (socketID <= FD_SETSIZE);
}

int VSocket::_platform_available() {
    int numBytesAvailable = 0;

    int result = ::v_ioctlsocket(mSocketID, FIONREAD, &numBytesAvailable);

    if (result == -1) {
        throw VStackTraceException(VSystemError::getSocketError(), VSTRING_FORMAT("VSocket[%s] available: v_ioctlsocket() failed.", mSocketName.chars()));
    }

    return numBytesAvailable;
}

