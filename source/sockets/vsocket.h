/*
Copyright c1997-2013 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.0
http://www.bombaydigital.com/
*/

#ifndef vsocket_h
#define vsocket_h

/** @file */

#include "vinstant.h"
#include "vstring.h"

// This pulls in any platform-specific declarations and includes:
#include "vsocket_platform.h"

/**

    @defgroup vsocket Vault Sockets

    The Vault implements platform-independent sockets, for both clients and
    servers. The class VSocket defines the low-level API for dealing with sockets.
    It's low-level in the sense that most Vault users will not have to use this API
    other than calling connectToHostName() or connectToIPAddress() to connect the
    socket when implementing a client-side connection. Instead, you'll mostly be
    using classes like VSocketStream to associate a stream with a socket, and the upper
    layer stream classes to perform the actual socket i/o. And server implementors will
    similarly just be attaching a stream object to each socket that gets created for an
    incoming client connection.

    Each socket platform (BSD/Unix sockets, or Winsock) has a slightly different API,
    but they are very close. The platform-specific quirks are separated out in the
    vsocket_platform.h and .cpp files per-platform.

    Client code that needs to connect will instantiate a VSocket, and then presumably
    use a VSocketStream and a VIOStream to do i/o over the socket. Server code will
    typically create a VListenerThread, which will use a VListenerSocket, and turn
    each incoming connection into a VSocket and VSocketThread, both created via a
    factory that you can supply to define the concrete classes that are instantiated
    for each. There are related classes for server-side client session/thread/socket/io
    management that you will find very helpful, located in the server directory: key
    classes are VServer, VClientSession, and related classes for messages, message queues,
    and i/o threads.

*/

typedef Vu32 VNetAddr;    ///< A 32-bit IP address, in network byte order (think of it as an array of 4 bytes, not as a 32-bit integer).

/**
    @ingroup vsocket
*/

/**
VNetworkInterfaceInfo describes an Internet interface on this computer. You can
get a list of the interfaces present by calling VSocket::enumerateNetworkInterfaces.
If you are just looking for the "local ip address", you should just call
VSocket::getLocalHostIPAddress(), which takes this information into account along
with the concept of a "preferred" interface on a multi-home system (where multiple
interfaces are active).
*/
class VNetworkInterfaceInfo {
    public:
        VNetworkInterfaceInfo() : mFamily(0), mName(), mAddress() {}
        ~VNetworkInterfaceInfo() {}
        int mFamily;        ///< Indicator of the type of interface.
        VString mName;      ///< Interface name.
        VString mAddress;   ///< IP address of the interface.
};
typedef std::vector<VNetworkInterfaceInfo> VNetworkInterfaceList;

class VSocketConnectionStrategy;

/**
VSocket is the class that defines a BSD or Winsock socket connection.

The basic way of using a client socket is to instantiate a VSocket (either
directly or through a VSocketFactory), and then call connectToHostName() or
connectToIPAddress() to connect to the desired server.

The basic way of using a server socket is to instantiate a VListenerThread
and supplying it a VSocketFactory and a subclass of VSocketThreadFactory
that will create your kind of VSocketThread to handle requests for a given
connection.

The best way to do i/o on a socket once it's connected, is to instantiate
a VSocketStream for the socket, and then instantiate the appropriate subclass
of VIOStream to do well-typed reads and writes on the socket.

Here is how you would connect to a host, write a 32-bit integer in network
byte order, receive a similar response, and clean up:

<tt>
Vu32 exchangeSingleMessage(const VString& host, int port, Vu32 request)<br>
{<br>
VSocket    socket;<br>
socket.init(host, port);<br>
<br>
VSocketStream    stream(socket);<br>
VBinaryIOStream    io(stream);<br>
Vu32            response;<br>
<br>
io.writeU32(request);<br>
io.flush();<br>
response = io.readU32();<br>
<br>
return response;<br>
}
</tt>

@see    VSocketFactory
@see    VSocketStream
@see    VListenerSocket
@see    VIOStream

FIXME:
According to some docs I have found, the proper shutdown sequence for a connected
socket is:
1. call shutdown(id, 1)    // well, not 1, so SHUT_WR or SHUT_RDWR? SHUT_RD would seem to make the next step fail
2. loop on recv until it returns 0 or any error
3. call close (or closesocket + WSACleanup on Win (is WSACleanup gone in WS2?))
*/
class VSocket {
    public:

        /**
        This function lets you specify the preferred network interface to be used
        when locating the local IP address from among multiple available interfaces.
        By default, "en0" (ethernet 0) is used. Note: This technique has no effect
        on Windows because the interfaces do not have names; you will need to use
        setPreferredLocalIPAddressPrefix() to achieve a similar effect.
        @param  interfaceName   the name of a network interface to choose first as
                                the local IP address when calling getLocalHostIPAddress()
        */
        static void setPreferredNetworkInterface(const VString& interfaceName);
        /**
        This function lets you specify the preferred address to be used when locating
        the local IP address from among multiple available interfaces, by specifying
        part or all of the address. By default, there is no setting. If you set a
        value that is a complete address (for example, "10.40.5.210") then when you
        call getLocalHostIPAddress(), if that address exists on any interface it will
        be returned. If you set a value this is a partial/prefix of an address (for
        example, "10.40.") then the first interface whose address starts with that
        prefix will be returned; this may be useful in DHCP situations where the
        address changes but is somewhat consistent and different from the range of
        unwanted addresses (e.g., ethernet vs. wifi).
        @param  addressPrefix   the partial (prefix) or full local IP address to
                                return if found on any interface when calling
                                getLocalHostIPAddress()
        */
        static void setPreferredLocalIPAddressPrefix(const VString& addressPrefix);
        /**
        Returns the current processor's IP address. If an error occurs, this
        function throws an exception containing the error code and message.
        Normally the first call gets the address, and later calls return a
        cached value; but you can pass refresh=true to force it to get the
        address again. If you have set a preferred interface by calling
        setPreferredNetworkInterface(), it will return that interface's address
        if found. Otherwise, the first interface's address is returned.
        The 127.0.0.1 loopback address is never returned.
        @param    ipAddress    a string in which to place the host name
        @param    refresh      set true to force the address to be re-obtained by
                                the call; otherwise, we obtain it once and cache
                                that to be returned on subsequent calls
        */
        static void getLocalHostIPAddress(VString& ipAddress, bool refresh = false);
        /**
        Returns a list of network interface info elements, by probing the
        network APIs and getting all of the AF_INET elements. Note that this
        may include multiple addresses if you have multiple network interfaces
        such as ethernet, wi-fi, etc. To obtain the "local host" IP address,
        you should call VSocket::getLocalHostIPAddress() rather than examining
        the results of this call, because it examines this information but
        takes into account the "preferred" interface that you can set with
        VSocket::setPreferredNetworkInterface().
        @return a list of zero or more network interfaces
        */
        static VNetworkInterfaceList enumerateNetworkInterfaces();
        /**
        Converts an IPv4 address string in dot notation to a 4-byte binary value
        that can be stored in a stream. Note that the return value is in network
        byte order by definition--think of it as an array of 4 bytes, not a
        32-bit integer. Note that the VNetAddr data type should be avoided as
        much as possible, since it is IPv4 only.
        @param    ipAddress    the string to convert (must be in x.x.x.x
                                notation)
        @return a 4-byte binary IPv4 address
        */
        static VNetAddr ipAddressStringToNetAddr(const VString& ipAddress);
        /**
        Converts a 4-byte binary IPv4 address value into a string in the dot
        notation format. Note that the input value is in network byte order by
        definition--think of it as an array of 4 bytes, not a 32-bit integer.
        Note that the VNetAddr data type should be avoided as much as possible,
        since it is IPv4 only.
        @param    netAddr       the 4-byte binary IPv4 address
        @param    ipAddress     the string in which to place the dot notation
                                version of the address
        */
        static void netAddrToIPAddressString(VNetAddr netAddr, VString& ipAddress);
        /**
        Resolves a internet host name to one or more numeric IP address strings.
        Typically you will use this for a user-entered address that you want to
        then open a socket to. If multiple addresses are returned, you have to
        decide what strategy to use when connecting: a) use the first address only,
        b) try each one in sequence until you succeed, or c) try all or several in
        parallel and go with the fastest one to succed. Note that the returned
        strings may be in IPv4 dotted decimal format (n.n.n.n) or IPv6 format
        (x:x:x:x::n for example; there are several related forms, see RFC 2373).
        If there is an error, or if no addresses are resolved, this function will
        throw a VException. It will never return an empty vector.
        @param  hostName    the host name to resolve; a numeric IP address is allowed
                            and will presumably resolve to itself
        @return one or more numeric IP address strings that the OS has resolved the
                host name to; if there are none, a VException is thrown instead of
                returning an empty vector
        */
        static VStringVector resolveHostName(const VString& hostName);
        /**
        Returns a string representation of the specified addrinfo internet address.
        It may be an IPv4 dotted decimal format (n.n.n.n) or IPv6 format.
        This function is used by resolveHostName() to convert each address it resolves.
        @param  hostName    optional value to be used in an error message if we need to throw an exception
        @param  info        the addrinfo containing the low-level information about the address;
                            you must only pass IPv4 (AF_INET) or IPv6 (AF_INET6) values; other types
                            will result in an exception being thrown
        @return a string in IPv4 dotted decimal format (n.n.n.n) or IPv6 format
        */
        static VString addrinfoToIPAddressString(const VString& hostName, const struct addrinfo* info);
        /**
        Returns true if the supplied numeric IP address string appears to be in IPv4
        dotted decimal format (n.n.n.n). We just check basic contents: dots, decimals,
        and minimum length. RFC 2373 describes all the possible variants.
        @param  ipAddress   a string to examine
        @return obvious
        */
        static bool isIPv4NumericString(const VString& ipAddress);
        /**
        Returns true if the supplied numeric IP address string appears to be in IPv6
        format. We just check basic contents: colons, dots, hexadecimals, and minimum length.
        @param  ipAddress   a string to examine
        @return obvious
        */
        static bool isIPv6NumericString(const VString& ipAddress);
        /**
        Returns true if the supplied numeric IP address string appears to be in IPv6
        or IPv4 format. This is a combined check that is more efficient than calling
        both of the above checks separately since it only has to scan the string once.
        Use this to distinguish from a host name.
        @param  ipAddress   a string to examine
        @return obvious
        */
        static bool isIPNumericString(const VString& ipAddress);

        /**
        The usual way to construct a VSocket is with the default constructor. You then subsequently
        call one of the connect() functions to cause it to connect using the appropriate strategy.
        You could also call setSockID() with an already-open low-level socket ID.
        */
        VSocket();
        /**
        The other way to construct a VSocket is to supply it an already-opened low-level socket ID.
        The VSocket will take ownership of the socket, and unless you later call setSockID(kNoSocketID),
        it will close the underlying socket upon destruction or a call to close().
        Constructs the object with an already-opened low-level
        socket connection identified by its ID.
        @param  id  an existing already-opened low-level socket ID
        */
        VSocket(VSocketID id);
        /**
        Destructor, cleans up by closing the socket.
        */
        virtual ~VSocket();

        /**
        Connects to the server using the specified numeric IP address and port. If the connection
        cannot be opened, a VException is thrown. This is also the API that the strategy object
        used by connectToHostName() will call (possibly on a temporary VSocket object) in
        order to establish a connection on a particular resolved IP address.
        @param  ipAddress   the IPv4 or IPv6 numeric address to connect to
        @param  portNumber  the port number to connect to
        */
        virtual void connectToIPAddress(const VString& ipAddress, int portNumber);
        /**
        Connects to the server using the specified host name and port; DNS resolution is performed
        on the host name to determine the IP addresses; the first resolved address is used. To
        choose a specific strategy for connecting to multiple resolved addresses, use the overloaded
        version of this API that takes a VSocketConnectionStrategy object. This version is
        equivalent to supplying a VSocketConnectionStrategySingle object.
        If the connection cannot be opened, a VException is thrown.
        @param  hostName    the name to resolve and then connect to
        @param  portNumber  the port number to connect to
        */
        virtual void connectToHostName(const VString& hostName, int portNumber);
        /**
        Connects to the server using the specified host name and port; DNS resolution is performed
        on the host name to determine the IP address. The supplied strategy determines how we
        connect when multiple addresses are returned by DNS resolution.
        If the connection cannot be opened, a VException is thrown.
        @param  hostName            the name to resolve and then connect to
        @param  portNumber          the port number to connect to
        @param  connectionStrategy  a strategy for connecting to a host name that resolves to multiple IP addresses
                                    (@see VSocketConnectionStrategySingle, VSocketConnectionStrategyLinear, VSocketConnectionStrategyThreaded)
        */
        virtual void connectToHostName(const VString& hostName, int portNumber, const VSocketConnectionStrategy& connectionStrategy);

        /**
        Associates this socket object with the specified socket id. This is
        something you might use if you are managing sockets externally and
        want to have a socket object own a socket temporarily.

        Note that this method does not cause a previous socket to be closed,
        nor does it update the name and port number properties of this object.
        If you want those things to happen, you can call close() and
        discoverHostAndPort() separately.

        @param    id    the socket id of the socket to manage
        */
        void setSockID(VSocketID id);
        /**
        Sets the host IP address and port for a subsequent connect() call.
        @param    hostIPAddress the host IP address, an IPv4 or IPv6 numeric string
        @param    portNumber    the port number to connect to on the host
        */
        virtual void setHostIPAddressAndPort(const VString& hostIPAddress, int portNumber);

        // --------------- These are the various utility and accessor methods.

        /**
        Returns the socket id.
        @return    the socket id
        */
        VSocketID getSockID() const;
        /**
        Returns the IP address of the host to which this socket is connected or has
        been set via setHostIPAddressAndPort().
        @return the host IP address; either an IPv4 or IPv6 numeric string
        */
        VString getHostIPAddress() const;
        /**
        Returns the port number on the host to which this socket is
        connected.
        @return     the host's port number to which this socket is connected
        */
        int getPortNumber() const;
        /**
        Returns a string concatenating the host name and port number, for purposes
        of socket connection identification when debugging and logging.
        @return a string with this socket's address and port
        */
        const VString& getName() const { return mSocketName; }
        /**
        Closes the socket. This terminates the connection.
        */
        void close();
        /**
        Sets the linger value for the socket.
        @param    val    the linger value in seconds
        */
        void setLinger(int val);
        /**
        Removes the read timeout setting for the socket.
        */
        void clearReadTimeOut();
        /**
        Sets the read timeout setting for the socket.
        @param    timeout    the read timeout value
        */
        void setReadTimeOut(const struct timeval& timeout);
        /**
        Removes the write timeout setting for the socket.
        */
        void clearWriteTimeOut();
        /**
        Sets the write timeout setting for the socket.
        @param    timeout    the write timeout value
        */
        void setWriteTimeOut(const struct timeval& timeout);
        /**
        Sets the socket options to their default values.
        */
        void setDefaultSockOpt();
        /**
        Returns the number of bytes that have been read from this socket.
        @return    the number of bytes read from this socket
        */
        Vs64 numBytesRead() const;
        /**
        Returns the number of bytes that have been written to this socket.
        @return    the number of bytes written to this socket
        */
        Vs64 numBytesWritten() const;
        /**
        Returns the amount of time since the last read or write activity
        occurred on this socket.
        */
        VDuration getIdleTime() const;

        // --------------- These are the pure virtual methods that only a platform
        // subclass can implement.

        /**
        Returns the number of bytes that are available to be read on this
        socket. If you do a read() on that number of bytes, you know that
        it will not block.
        @return the number of bytes currently available for reading
        */
        virtual int available() { return this->_platform_available(); }
        /**
        Reads data from the socket.

        If you don't have a read timeout set up for this socket, then
        read will block until all requested bytes have been read.

        @param    buffer            the buffer to read into
        @param    numBytesToRead    the number of bytes to read from the socket
        @return    the number of bytes read
        */
        virtual int read(Vu8* buffer, int numBytesToRead);
        /**
        Writes data to the socket.

        If you don't have a write timeout set up for this socket, then
        write will block until all requested bytes have been written.

        @param    buffer            the buffer to read out of
        @param    numBytesToWrite    the number of bytes to write to the socket
        @return    the number of bytes written
        */
        virtual int write(const Vu8* buffer, int numBytesToWrite);
        /**
        Flushes any unwritten bytes to the socket.
        */
        virtual void flush();
        /**
        Sets the host name and port number properties of this socket by
        asking the lower level services to whom the socket is connected.
        */
        virtual void discoverHostAndPort();
        /**
        Shuts down just the read side of the connection.
        */
        virtual void closeRead();
        /**
        Shuts down just the write side of the connection.
        */
        virtual void closeWrite();
        /**
        Sets a specified socket option.
        @param    level        the option level
        @param    name        the option name
        @param    valuePtr    a pointer to the new option value data
        @param    valueLength    the length of the data pointed to by valuePtr
        */
        virtual void setSockOpt(int level, int name, void* valuePtr, int valueLength);
        /**
        For "int" socket options, this helper function simplifies use of setSockOpt.
        @param  level   the option level
        @param  name    the option name
        @param  value   the option value
        */
        void setIntSockOpt(int level, int name, int value);

        static const VSocketID kNoSocketID = V_NO_SOCKET_ID_CONSTANT; ///< The sock id for a socket that is not connected.
        static const int kDefaultBufferSize = 65535;    ///< The default buffer size.
        static const int kDefaultServiceType = 0x08;    ///< The default service type.
        static const int kDefaultNoDelay = 1;           ///< The default nodelay value.

    protected:

        /**
        Connects to the server.
        */
        virtual void _connectToIPAddress(const VString& ipAddress, int portNumber);
        /**
        Starts listening for incoming connections. Only useful to call
        from a VListenerSocket subclass that exposes a public listen() API.
        @param  bindAddress if empty, the socket will bind to INADDR_ANY (usually a good
                                default); if a value is supplied the socket will bind to the
                                supplied IP address (can be useful on a multi-homed server)
        @param  backlog     the backlog value to supply to the ::listen() function
        */
        virtual void _listen(const VString& bindAddress, int backlog);

        VSocketID       mSocketID;              ///< The socket id.
        VString         mHostIPAddress;         ///< The IP address of the host to which the socket is connected.
        int             mPortNumber;            ///< The port number on the host to which the socket is connected.
        bool            mReadTimeOutActive;     ///< True if reads should time out.
        struct timeval  mReadTimeOut;           ///< The read timeout value, if used.
        bool            mWriteTimeOutActive;    ///< True if writes should time out.
        struct timeval  mWriteTimeOut;          ///< The write timeout value, if used.
        bool            mRequireReadAll;        ///< True if we throw when read returns less than # bytes asked for.
        Vs64            mNumBytesRead;          ///< Number of bytes read from this socket.
        Vs64            mNumBytesWritten;       ///< Number of bytes written to this socket.
        VInstant        mLastEventTime;         ///< Timestamp of last read or write.
        VString         mSocketName;            ///< Returned by getName(), useful purely for logging and debugging.

        static VString gPreferredNetworkInterfaceName;
        static VString gPreferredLocalIPAddressPrefix;
        static VString gCachedLocalHostIPAddress;

    protected: // will be private when complete

        static bool gStaticInited;  ///< Used internally to initialize at startup.
        static bool staticInit();   ///< Used internally to initialize at startup.
    
        // Work in progress: I'm restructuring VSocket platform-specific code.
        // Like VFSNode, these are the known-to-be-specific functions. But we
        // don't need a subclass, just separate implementations.
        
        /**
        Returns true if the specified socket ID (e.g., returned from ::socket())
        is valid. BSD and Winsock have different ranges of good values.
        */
        bool _platform_isSocketIDValid(VSocketID socketID) const;
        /**
        Returns the number of bytes that are available to be read on this
        socket. If you do a read() on that number of bytes, you know that
        it will not block.
        @return the number of bytes currently available for reading
        */
        int _platform_available();
};

/**
VSocketInfo is essentially a structure that just contains a copy of
information about a VSocket as it existed at the point in time when
the VSocketInfo was created.
*/
class VSocketInfo {
    public:

        /**
        Constructs the object by copying the info from the socket.
        @param    socket    the socket to take the information from
        */
        VSocketInfo(const VSocket& socket);
        /**
        Destructor.
        */
        virtual ~VSocketInfo() {}

        VSocketID   mSocketID;          ///< The sock id.
        VString     mHostIPAddress;     ///< The IP address of the host to which the socket is connected.
        int         mPortNumber;        ///< The port number on the host to which the socket is connected.
        Vs64        mNumBytesRead;      ///< Number of bytes read from this socket.
        Vs64        mNumBytesWritten;   ///< Number of bytes written to this socket.
        VDuration   mIdleTime;          ///< Amount of time elapsed since last activity.
};

/**
VSocketInfoVector is simply a vector of VSocketInfo objects.
*/
typedef std::vector<VSocketInfo> VSocketInfoVector;

/**
A socket connection strategy determines how to connect a socket in the face of DNS resolution,
when an IP may resolve to more than one IP address. Provided concrete classes handle single,
multiple+synchronous, and multiple+multithreaded approaches.
*/
class VSocketConnectionStrategy {

    public:
        VSocketConnectionStrategy() : mDebugIPAddresses() {}
        virtual ~VSocketConnectionStrategy() {}

        /**
        The VSocketConnectionStrategy interface to be implemented by concrete subclasses.
        Connects to the host name by resolving it and then applying the strategy to the resolved
        IP addresses. If the strategy fails (for whatever reason, be it a connect failure or a
        timeout) it shall throw a VException.
        @param  hostName        the name to resolve and connect to
        @param  portNumber      the port number to connect to
        @param  socketToConnect if successful, this Vault socket object is to be mutated upon
                                return to have the connected socket's host IP address and low-level
                                VSocketID; for simple non-threaded strategies it is acceptable to
                                simply call this object's connectToIPAddress() and leave the results
                                in place if successful
        */
        virtual void connect(const VString& hostName, int portNumber, VSocket& socketToConnect) const = 0;

        // For testing purposes, you can inject a specific set of IP addresses, which
        // will cause the hostName supplied with connect() to be ignored. You could supply
        // specific bad IP addresses or slow-to-succeed IP addresses, in order to see
        // what happens. Our unit tests do this.
        void injectDebugIPAddresses(const VStringVector debugIPAddresses) { mDebugIPAddresses = debugIPAddresses; }

    protected:

        VStringVector mDebugIPAddresses; ///< If non-empty, use instead of resolving hostName in connect().
};

/**
Connects to the first DNS resolved IP address for a host name. Others are ignored.
*/
class VSocketConnectionStrategySingle : public VSocketConnectionStrategy {

    public:
        VSocketConnectionStrategySingle() {}
        virtual ~VSocketConnectionStrategySingle() {}

        // VSocketConnectionStrategy implementation:
        virtual void connect(const VString& hostName, int portNumber, VSocket& socketToConnect) const;
};

/**
Connects to  each DNS resolved IP address for a host name, in order, until one succeeds
or a specified timeout is reached. This strategy makes most sense with IPv4 where DNS is
supposed to return a randomized list of resolved addresses, thus achieving a form of round-robin
connection balancing by always using the first address.
*/
class VSocketConnectionStrategyLinear : public VSocketConnectionStrategy {

    public:
        VSocketConnectionStrategyLinear(const VDuration& timeout);
        virtual ~VSocketConnectionStrategyLinear() {}

        // VSocketConnectionStrategy implementation:
        virtual void connect(const VString& hostName, int portNumber, VSocket& socketToConnect) const;

    private:
        const VDuration mTimeout;
};

/**
Connects to the all DNS resolved IP addresses for a host name, in parallel batches,
until one succeeds or a specified timeout is reached. This strategy makes most sense with IPv6
where DNS is supposed to return a preferred-order list of resolved names (rather than round-
robining) but where we have an opportunity to use the one that responds fastest.
*/
class VSocketConnectionStrategyThreaded : public VSocketConnectionStrategy {

    public:
        VSocketConnectionStrategyThreaded(const VDuration& timeoutInterval, int maxNumThreads = 4);
        virtual ~VSocketConnectionStrategyThreaded() {}

        // VSocketConnectionStrategy implementation:
        virtual void connect(const VString& hostName, int portNumber, VSocket& socketToConnect) const;

    private:

        VDuration   mTimeoutInterval;
        int         mMaxNumThreads;
};

#endif /* vsocket_h */

