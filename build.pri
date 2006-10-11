# Copyright c1997-2006 Trygve Isaacson. All rights reserved.
# This file is part of the Code Vault version 2.5
# http://www.bombaydigital.com/

#
# This is a qmake include file -- to be included from a .pro file.
#
# Because paths are relative to the main file (not this one), this
# file relies on the main file defining the variable $${BASE} so
# that this file can specify paths correctly.
#

win32:LIBS += wsock32.lib

debug {
DEFINES += V_DEBUG
}

macx {
CONFIG += = vault_mac
} else:unix {
CONFIG += = vault_unix
} else:win32 {
CONFIG += = vault_win32
}

vault_mac {
DEPENDPATH += $${BASE}/source/vtypes/_mac
DEPENDPATH += $${BASE}/source/threads/_unix
DEPENDPATH += $${BASE}/source/sockets/_unix
DEPENDPATH += $${BASE}/source/files/_unix

INCLUDEPATH += $${BASE}/source/vtypes/_mac
INCLUDEPATH += $${BASE}/source/threads/_unix
INCLUDEPATH += $${BASE}/source/sockets/_unix
INCLUDEPATH += $${BASE}/source/files/_unix

HEADERS += $${BASE}/source/vtypes/_mac/vtypes_platform.h
SOURCES += $${BASE}/source/vtypes/_mac/vtypes_platform.cpp
SOURCES += $${BASE}/source/containers/_unix/vinstant_platform.cpp
HEADERS += $${BASE}/source/threads/_unix/vthread_platform.h
SOURCES += $${BASE}/source/threads/_unix/vthread_platform.cpp
HEADERS += $${BASE}/source/sockets/_unix/vsocket.h
SOURCES += $${BASE}/source/sockets/_unix/vsocket.cpp
SOURCES += $${BASE}/source/sockets/_unix/vfsnode_platform.cpp
}

vault_unix {
DEPENDPATH += $${BASE}/source/vtypes/_unix
DEPENDPATH += $${BASE}/source/threads/_unix
DEPENDPATH += $${BASE}/source/sockets/_unix
DEPENDPATH += $${BASE}/source/files/_unix

INCLUDEPATH += $${BASE}/source/vtypes/_unix
INCLUDEPATH += $${BASE}/source/threads/_unix
INCLUDEPATH += $${BASE}/source/sockets/_unix
INCLUDEPATH += $${BASE}/source/files/_unix

HEADERS += $${BASE}/source/vtypes/_unix/vtypes_platform.h
SOURCES += $${BASE}/source/vtypes/_unix/vtypes_platform.cpp
SOURCES += $${BASE}/source/containers/_unix/vinstant_platform.cpp
HEADERS += $${BASE}/source/threads/_unix/vthread_platform.h
SOURCES += $${BASE}/source/threads/_unix/vthread_platform.cpp
HEADERS += $${BASE}/source/sockets/_unix/vsocket.h
SOURCES += $${BASE}/source/sockets/_unix/vsocket.cpp
SOURCES += $${BASE}/source/sockets/_unix/vfsnode_platform.cpp
}

vault_win32 {
DEPENDPATH += $${BASE}/source/vtypes/_win
DEPENDPATH += $${BASE}/source/threads/_win
DEPENDPATH += $${BASE}/source/sockets/_win
DEPENDPATH += $${BASE}/source/files/_win

INCLUDEPATH += $${BASE}/source/vtypes/_win
INCLUDEPATH += $${BASE}/source/threads/_win
INCLUDEPATH += $${BASE}/source/sockets/_win
INCLUDEPATH += $${BASE}/source/files/_win

HEADERS += $${BASE}/source/vtypes/_win/vtypes_platform.h
SOURCES += $${BASE}/source/vtypes/_win/vtypes_platform.cpp
SOURCES += $${BASE}/source/containers/_win/vinstant_platform.cpp
HEADERS += $${BASE}/source/threads/_win/vthread_platform.h
SOURCES += $${BASE}/source/threads/_win/vthread_platform.cpp
HEADERS += $${BASE}/source/sockets/_win/vsocket.h
SOURCES += $${BASE}/source/sockets/_win/vsocket.cpp
SOURCES += $${BASE}/source/sockets/_win/vfsnode_platform.cpp
}

DEPENDPATH += $${BASE}/source
DEPENDPATH += $${BASE}/source/containers
DEPENDPATH += $${BASE}/source/files
DEPENDPATH += $${BASE}/source/server
DEPENDPATH += $${BASE}/source/sockets
DEPENDPATH += $${BASE}/source/streams
DEPENDPATH += $${BASE}/source/threads
DEPENDPATH += $${BASE}/source/toolbox
DEPENDPATH += $${BASE}/source/unittest
DEPENDPATH += $${BASE}/source/vtypes

INCLUDEPATH += $${BASE}/source
INCLUDEPATH += $${BASE}/source/containers
INCLUDEPATH += $${BASE}/source/files
INCLUDEPATH += $${BASE}/source/server
INCLUDEPATH += $${BASE}/source/sockets
INCLUDEPATH += $${BASE}/source/streams
INCLUDEPATH += $${BASE}/source/threads
INCLUDEPATH += $${BASE}/source/toolbox
INCLUDEPATH += $${BASE}/source/unittest
INCLUDEPATH += $${BASE}/source/vtypes

HEADERS += $${BASE}/source/vault.h
HEADERS += $${BASE}/source/vconfigure.h
HEADERS += $${BASE}/source/vtypes/vtypes.h
SOURCES += $${BASE}/source/vtypes/vtypes.cpp
HEADERS += $${BASE}/source/containers/vbento.h
SOURCES += $${BASE}/source/containers/vbento.cpp
HEADERS += $${BASE}/source/containers/vchar.h
SOURCES += $${BASE}/source/containers/vchar.cpp
HEADERS += $${BASE}/source/containers/vexception.h
SOURCES += $${BASE}/source/containers/vexception.cpp
HEADERS += $${BASE}/source/containers/vinstant.h
SOURCES += $${BASE}/source/containers/vinstant.cpp
HEADERS += $${BASE}/source/containers/vstring.h
SOURCES += $${BASE}/source/containers/vstring.cpp
HEADERS += $${BASE}/source/files/vabstractfilestream.h
SOURCES += $${BASE}/source/files/vabstractfilestream.cpp
HEADERS += $${BASE}/source/files/vbufferedfilestream.h
SOURCES += $${BASE}/source/files/vbufferedfilestream.cpp
HEADERS += $${BASE}/source/files/vdirectiofilestream.h
SOURCES += $${BASE}/source/files/vdirectiofilestream.cpp
HEADERS += $${BASE}/source/files/vfsnode.h
SOURCES += $${BASE}/source/files/vfsnode.cpp
HEADERS += $${BASE}/source/server/vlistenersocket.h
SOURCES += $${BASE}/source/server/vlistenersocket.cpp
HEADERS += $${BASE}/source/server/vlistenerthread.h
SOURCES += $${BASE}/source/server/vlistenerthread.cpp
HEADERS += $${BASE}/source/server/vmanagementinterface.h
HEADERS += $${BASE}/source/sockets/vsocketbase.h
SOURCES += $${BASE}/source/sockets/vsocketbase.cpp
HEADERS += $${BASE}/source/sockets/vsocketfactory.h
SOURCES += $${BASE}/source/sockets/vsocketfactory.cpp
HEADERS += $${BASE}/source/sockets/vsocketstream.h
SOURCES += $${BASE}/source/sockets/vsocketstream.cpp
HEADERS += $${BASE}/source/sockets/vsocketthread.h
SOURCES += $${BASE}/source/sockets/vsocketthread.cpp
HEADERS += $${BASE}/source/sockets/vsocketthreadfactory.h
HEADERS += $${BASE}/source/streams/vbinaryiostream.h
SOURCES += $${BASE}/source/streams/vbinaryiostream.cpp
HEADERS += $${BASE}/source/streams/viostream.h
SOURCES += $${BASE}/source/streams/viostream.cpp
HEADERS += $${BASE}/source/streams/vmemorystream.h
SOURCES += $${BASE}/source/streams/vmemorystream.cpp
HEADERS += $${BASE}/source/streams/vstream.h
SOURCES += $${BASE}/source/streams/vstream.cpp
HEADERS += $${BASE}/source/streams/vstreamcopier.h
SOURCES += $${BASE}/source/streams/vstreamcopier.cpp
HEADERS += $${BASE}/source/streams/vtextiostream.h
SOURCES += $${BASE}/source/streams/vtextiostream.cpp
HEADERS += $${BASE}/source/streams/vwritebufferedstream.h
SOURCES += $${BASE}/source/streams/vwritebufferedstream.cpp
HEADERS += $${BASE}/source/threads/vmutex.h
SOURCES += $${BASE}/source/threads/vmutex.cpp
HEADERS += $${BASE}/source/threads/vmutexlocker.h
SOURCES += $${BASE}/source/threads/vmutexlocker.cpp
HEADERS += $${BASE}/source/threads/vsemaphore.h
SOURCES += $${BASE}/source/threads/vsemaphore.cpp
HEADERS += $${BASE}/source/threads/vthread.h
SOURCES += $${BASE}/source/threads/vthread.cpp
HEADERS += $${BASE}/source/toolbox/vclassregistry.h
SOURCES += $${BASE}/source/toolbox/vclassregistry.cpp
HEADERS += $${BASE}/source/toolbox/vhex.h
SOURCES += $${BASE}/source/toolbox/vhex.cpp
HEADERS += $${BASE}/source/toolbox/vlogger.h
SOURCES += $${BASE}/source/toolbox/vlogger.cpp
HEADERS += $${BASE}/source/toolbox/vsettings.h
SOURCES += $${BASE}/source/toolbox/vsettings.cpp
HEADERS += $${BASE}/source/toolbox/vshutdownregistry.h
SOURCES += $${BASE}/source/toolbox/vshutdownregistry.cpp
HEADERS += $${BASE}/source/toolbox/vsingleton.h
HEADERS += $${BASE}/source/unittest/vbentounit.h
SOURCES += $${BASE}/source/unittest/vbentounit.cpp
HEADERS += $${BASE}/source/unittest/vbinaryiounit.h
SOURCES += $${BASE}/source/unittest/vbinaryiounit.cpp
HEADERS += $${BASE}/source/unittest/vcharunit.h
SOURCES += $${BASE}/source/unittest/vcharunit.cpp
HEADERS += $${BASE}/source/unittest/vclassregistryunit.h
SOURCES += $${BASE}/source/unittest/vclassregistryunit.cpp
HEADERS += $${BASE}/source/unittest/vexceptionunit.h
SOURCES += $${BASE}/source/unittest/vexceptionunit.cpp
HEADERS += $${BASE}/source/unittest/vfsnodeunit.h
SOURCES += $${BASE}/source/unittest/vfsnodeunit.cpp
HEADERS += $${BASE}/source/unittest/vhexunit.h
SOURCES += $${BASE}/source/unittest/vhexunit.cpp
HEADERS += $${BASE}/source/unittest/vinstantunit.h
SOURCES += $${BASE}/source/unittest/vinstantunit.cpp
HEADERS += $${BASE}/source/unittest/vplatformunit.h
SOURCES += $${BASE}/source/unittest/vplatformunit.cpp
HEADERS += $${BASE}/source/unittest/vstreamsunit.h
SOURCES += $${BASE}/source/unittest/vstreamsunit.cpp
HEADERS += $${BASE}/source/unittest/vstringunit.h
SOURCES += $${BASE}/source/unittest/vstringunit.cpp
HEADERS += $${BASE}/source/unittest/vthreadsunit.h
SOURCES += $${BASE}/source/unittest/vthreadsunit.cpp
HEADERS += $${BASE}/source/unittest/vunit.h
SOURCES += $${BASE}/source/unittest/vunit.cpp
HEADERS += $${BASE}/source/unittest/vunitrunall.h
SOURCES += $${BASE}/source/unittest/vunitrunall.cpp

