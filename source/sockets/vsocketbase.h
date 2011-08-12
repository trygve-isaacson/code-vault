/*
Copyright c1997-2011 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.2
http://www.bombaydigital.com/
*/

#ifndef vsocketbase_h
#define vsocketbase_h

/** @file */

#include "vinstant.h"
#include "vstring.h"

/**

    @defgroup vsocket Vault Sockets

    The Vault implements platform-independent sockets, for both clients and
    servers. The abstract base class VSocketBase defines the low-level API for dealing
    with sockets. It's low-level in the sense that most Vault users will not have
    to use this API other than calling init() to connect the socket when implementing
    a client-side connection. Instead, you'll be using classes like VSocketStream to
    associate a stream with a socket, and the upper layer stream classes to perform
    the actual socket i/o. And server implementors will similarly just be attaching
    a stream object to each socket that gets created for an incoming client connection.

    Each socket platform needs to define an implementation of the
    concrete subclass VSocket, so there is currently one defined when compiling for
    BSD/Unix sockets and one for WinSock, though these are very similar because their
    platform APIs are different mostly in small ways.

    Client code that needs to connect will instantiate a VSocket (whether this is the
    BSD version or the WinSock version just depends on what platform you are compiling
    on), and then presumably use a VSocketStream and a VIOStream to do i/o
    over the socket. Server code will typically create a VListenerThread, which will
    use a VListenerSocket, and turn each incoming connection into a VSocket and
    VSocketThread, both created via a factory that you can supply to define the
    concrete classes that are instantiated for each.

*/

// We have to define VSocketID here because we don't include platform (it includes us).
#ifdef VPLATFORM_WIN
    typedef SOCKET VSocketID;    ///< The platform-dependent definition of a socket identifier.
    #define V_NO_SOCKET_ID_CONSTANT INVALID_SOCKET ///< Used internally to initialize kNoSocketID
#else
    typedef int VSocketID;    ///< The platform-dependent definition of a socket identifier.
    #define V_NO_SOCKET_ID_CONSTANT -1 ///< Used internally to initialize kNoSocketID
#endif

typedef Vu32 VNetAddr;    ///< A 32-bit IP address, in network byte order (think of it as an array of 4 bytes, not as a 32-bit integer).

class VSocket;

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
class VNetworkInterfaceInfo
    {
    public:
        VNetworkInterfaceInfo() : mFamily(0), mName(), mAddress() {}
        ~VNetworkInterfaceInfo() {}
        int mFamily;
        VString mName;
        VString mAddress;
    };
typedef std::vector<VNetworkInterfaceInfo> VNetworkInterfaceList;

/**
VSocketBase is the abstract base class from which each platform's VSocket
implementation is derived. So you instantiate VSocket objects, but you
can see the API by looking at the VSocketBase class.

The basic way of using a client socket is to instantiate a VSocket (either
directly or through a VSocketFactory), and then call init() to connect to
the server.

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
class VSocketBase
    {
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
        static void getLocalHostIPAddress(VString& ipAddress, bool refresh=false);
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
        Converts an IP address string in dot notation to a 4-byte value
        that can be stored in a stream as an int. Note that the return
        value is in network byte order by definition--think of it as
        an array of 4 bytes, not a 32-bit integer.
        @param    ipAddress    the string to convert (must be in x.x.x.x
                                notation)
        @return a 4-byte IP address
        */
        static VNetAddr ipAddressStringToNetAddr(const VString& ipAddress);
        /**
        Converts a 4-byte IP address value into a string in the dot notation
        format. Note that the input value is in network byte order by
        definition--think of it as an array of 4 bytes, not a 32-bit integer.
        @param    netAddr        the 4-byte IP address
        @param    ipAddress    the string in which to place the dot notation
                                version of the address
        */
        static void netAddrToIPAddressString(VNetAddr netAddr, VString& ipAddress);

        /**
        Constructs the object with an already-opened low-level
        socket connection identified by its ID.
        */
        VSocketBase(VSocketID id);
        /**
        Constructs the object to use the specified host and port, but
        does NOT open a connection yet (you must call connect() later to
        establish the connection).
        */
        VSocketBase(const VString& hostName, int portNumber);
        /**
        Destructor, cleans up by closing the socket.
        */
        virtual ~VSocketBase();

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
        Initializes the socket object by opening a connection to a server at the
        specified host and port.
        If the connection cannot be opened, a VException is thrown.
        @param    hostName    the host name, numeric or not is fine
        @param    portNumber    the port number to connect to on the host
        */
        virtual void setHostAndPort(const VString& hostName, int portNumber);

        // --------------- These are the various utility and accessor methods.

        /**
        Returns the socket id.
        @return    the socket id
        */
        VSocketID getSockID() const;
        /**
        Returns the name or address of the host to which this socket is
        connected.
        @param    hostName    the string to format
        */
        void getHostName(VString& hostName) const;
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
        Connects to the server; only valid for a newly created VSocket that
        was initialized with host and port in the constructor or with a
        call to setHostAndPort().
        */
        virtual void connect();
        /**
        Returns the number of bytes that are available to be read on this
        socket. If you do a read() on that number of bytes, you know that
        it will not block.
        @return the number of bytes currently available for reading
        */
        virtual int available() = 0;
        /**
        Reads data from the socket.

        If you don't have a read timeout set up for this socket, then
        read will block until all requested bytes have been read.

        @param    buffer            the buffer to read into
        @param    numBytesToRead    the number of bytes to read from the socket
        @return    the number of bytes read
        */
        virtual int read(Vu8* buffer, int numBytesToRead) = 0;
        /**
        Writes data to the socket.

        If you don't have a write timeout set up for this socket, then
        write will block until all requested bytes have been written.

        @param    buffer            the buffer to read out of
        @param    numBytesToWrite    the number of bytes to write to the socket
        @return    the number of bytes written
        */
        virtual int write(const Vu8* buffer, int numBytesToWrite) = 0;
        /**
        Flushes any unwritten bytes to the socket.
        */
        virtual void flush();
        /**
        Sets the host name and port number properties of this socket by
        asking the lower level services to whom the socket is connected.
        */
        virtual void discoverHostAndPort() = 0;
        /**
        Shuts down just the read side of the connection.
        */
        virtual void closeRead() = 0;
        /**
        Shuts down just the write side of the connection.
        */
        virtual void closeWrite() = 0;
        /**
        Sets a specified socket option.
        @param    level        the option level
        @param    name        the option name
        @param    valuePtr    a pointer to the new option value data
        @param    valueLength    the length of the data pointed to by valuePtr
        */
        virtual void setSockOpt(int level, int name, void* valuePtr, int valueLength) = 0;
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
        virtual void _connect() = 0;
        /**
        Starts listening for incoming connections. Only useful to call
        from a VListenerSocket subclass that exposes a public listen() API.
        @param  bindAddress if empty, the socket will bind to INADDR_ANY (usually a good
                                default); if a value is supplied the socket will bind to the
                                supplied IP address (can be useful on a multi-homed server)
        @param  backlog     the backlog value to supply to the ::listen() function
        */
        virtual void _listen(const VString& bindAddress, int backlog) = 0;

        VSocketID       mSocketID;              ///< The socket id.
        VString         mHostName;              ///< The name of the host to which the socket is connected.
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
    };

/**
VSocketInfo is essentially a structure that just contains a copy of
information about a VSocket as it existed at the point in time when
the VSocketInfo was created.
*/
class VSocketInfo
    {
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
        VString     mHostName;          ///< The name of the host to which the socket is connected.
        int         mPortNumber;        ///< The port number on the host to which the socket is connected.
        Vs64        mNumBytesRead;      ///< Number of bytes read from this socket.
        Vs64        mNumBytesWritten;   ///< Number of bytes written to this socket.
        VDuration   mIdleTime;          ///< Amount of time elapsed since last activity.
    };

/**
VSocketInfoVector is simply a vector of VSocketInfo objects.
*/
typedef std::vector<VSocketInfo> VSocketInfoVector;

#endif /* vsocketbase_h */

