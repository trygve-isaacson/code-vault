[![Build status](https://build.appcenter.ms/v0.1/apps/a95edd61-8e4a-4045-866c-8bad3c65890c/branches/master/badge)](https://appcenter.ms) (`master` branch)

C++ Code Vault
==========
A  C++ cross-platform library.

For more info, see [the Vault home page](http://www.bombaydigital.com/vault/)

(version 4.1)

C++ can give you the best of several worlds: close-to-the-metal performance, high-level object-oriented design, and simple deterministic object lifecycles. But historically the compiler tools, standard libraries, and platform APIs have conspired to make it challenging to create large scale code that compiles and runs right out of the box on any platform. In addition, a variety of basic necessities for programs are only provided by each OS's different low-level native procedural APIs, making cross-platform development a big undertaking.

The Code Vault, or “Vault” for short, provides a cross-platform foundation library under your code that not only protects you from the many little compiler/library/platform quirks, allowing your C++ to easily build and run on multiple platforms, but also provides a powerful toolkit for building cross-platform O-O code for networking, i/o, threading, messaging, logging, and more.

Vault supports Mac OS X, iOS, Linux, and Windows, and is provided under an MIT license.

Key Features
----------

###Toolchain Independence
\#include "vault.h"

One simple include sidesteps the surprising quirks that hit you when you try to compile on a new platform. The list is large, whether it’s byte-swapping behavior, the location of fabs() (a macro? a function? in std namespace or not?), which system includes are inevitable, or how to declare a 64-bit integer.

###Unified Stream I/O
VStream (socket, file, memory), VIOStream (text, binary), ...

Clean and simple stream classes let you do i/o on sockets, files, and memory, in both text and binary form, in a uniform way. Deal with sockets and files in a high-level, O-O way. Do serialization the right way. Access the file system and locate the preferred directory locations in a platform-neutral way. Supports IPv6 network APIs.

###High-Level Containers
VString, VCodePoint, VStringIterator, ...

The std::string is a nice foundation but it is not enough. Use Vault’s efficient native UTF-8 VString with “SSO”, Unicode conversions, and code point-based iterators. Avoid crashes and security issues by elminating the “char*” data type from your code.

###Client and Server Support
VServer, VClientSession, VMessageQueue, VMessageHandler, ...

When writing a server, easily manage listeners, client sessions, queued message i/o, and concurrency. When writing a client, easily manage sync and async message i/o. Leverage the Vault stream APIs. All at a clean, high level.

###Thread Objects
VThread, VMutexLocker, ...

Easily create and manage threads and lifecycles. Leverage the client-server support. The default behavior prevents accidentally leaving a mutex locked (a common source of concurrency bugs). Detect when code holds a mutex for too long. Assists in diagnosing deadlocks and concurrency performance.

###Time Support
VInstant, VDuration, VDateAndTime, VDate, VTimeOfDay, ...

Strongly typed classes for managing time stamps, durations (intervals), and conversions. Makes it impossible to accidentally combine the wrong scale factor. Lets you freeze and simulate time flow for testing. Completely deals with the subtly but painfully different platform time APIs. Spans the entire time range each platform supports, with a single time “instant” type.

###Logging
VLogger, VLogAppender, ...

Highly configurable logging behavior. Multiplexed sources and destinations; console, files, or any output you want to implement. Write log output without incurring unnecessary overhead. Route information based on hierarchical paths or object identities.

###Unit Testing
VUnit, ...

Easily write unit tests to prove that your code does what is expected. The Vault unit test run performs thousands of self-tests using its own unit testing framework.

##More!

A sampling of some of the other classes Vault provides: VBento Bento structured data. VPoint, VRect, VPoloygon, VPoint3D, VSize, VLine Floating-point and integer-based geometry types/utilities. VColor, VColorPair, VColorPalette Color, palette, and color mapping utilities. VException, VSystemError, VStackTraceException Exceptions with cross-platform system error handling and stack traces. VHex Hex dump generation and i/o.

It’s all about leveraging a unified, consistent API across the entire library, and letting you build upon it.
