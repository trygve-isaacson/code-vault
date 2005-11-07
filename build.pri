# Copyright c1997-2005 Trygve Isaacson. All rights reserved.
# This file is part of the Code Vault version 2.3.2
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
DEPENDPATH += $${BASE}/common/lib/vault/source/vtypes/_mac
DEPENDPATH += $${BASE}/common/lib/vault/source/threads/_unix
DEPENDPATH += $${BASE}/common/lib/vault/source/sockets/_unix

INCLUDEPATH += $${BASE}/common/lib/vault/source/vtypes/_mac
INCLUDEPATH += $${BASE}/common/lib/vault/source/threads/_unix
INCLUDEPATH += $${BASE}/common/lib/vault/source/sockets/_unix

HEADERS += $${BASE}/common/lib/vault/source/vtypes/_mac/vtypes_platform.h
SOURCES += $${BASE}/common/lib/vault/source/vtypes/_mac/vtypes_platform.cpp
SOURCES += $${BASE}/common/lib/vault/source/containers/_unix/vinstant_platform.cpp
HEADERS += $${BASE}/common/lib/vault/source/threads/_unix/vthread_platform.h
SOURCES += $${BASE}/common/lib/vault/source/threads/_unix/vthread_platform.cpp
HEADERS += $${BASE}/common/lib/vault/source/sockets/_unix/vsocket.h
SOURCES += $${BASE}/common/lib/vault/source/sockets/_unix/vsocket.cpp
}

vault_unix {
DEPENDPATH += $${BASE}/common/lib/vault/source/vtypes/_unix
DEPENDPATH += $${BASE}/common/lib/vault/source/threads/_unix
DEPENDPATH += $${BASE}/common/lib/vault/source/sockets/_unix

INCLUDEPATH += $${BASE}/common/lib/vault/source/vtypes/_unix
INCLUDEPATH += $${BASE}/common/lib/vault/source/threads/_unix
INCLUDEPATH += $${BASE}/common/lib/vault/source/sockets/_unix

HEADERS += $${BASE}/common/lib/vault/source/vtypes/_unix/vtypes_platform.h
SOURCES += $${BASE}/common/lib/vault/source/vtypes/_unix/vtypes_platform.cpp
SOURCES += $${BASE}/common/lib/vault/source/containers/_unix/vinstant_platform.cpp
HEADERS += $${BASE}/common/lib/vault/source/threads/_unix/vthread_platform.h
SOURCES += $${BASE}/common/lib/vault/source/threads/_unix/vthread_platform.cpp
HEADERS += $${BASE}/common/lib/vault/source/sockets/_unix/vsocket.h
SOURCES += $${BASE}/common/lib/vault/source/sockets/_unix/vsocket.cpp
}

vault_win32 {
DEPENDPATH += $${BASE}/common/lib/vault/source/vtypes/_win
DEPENDPATH += $${BASE}/common/lib/vault/source/threads/_win
DEPENDPATH += $${BASE}/common/lib/vault/source/sockets/_win

INCLUDEPATH += $${BASE}/common/lib/vault/source/vtypes/_win
INCLUDEPATH += $${BASE}/common/lib/vault/source/threads/_win
INCLUDEPATH += $${BASE}/common/lib/vault/source/sockets/_win

HEADERS += $${BASE}/common/lib/vault/source/vtypes/_win/vtypes_platform.h
SOURCES += $${BASE}/common/lib/vault/source/vtypes/_win/vtypes_platform.cpp
SOURCES += $${BASE}/common/lib/vault/source/containers/_win/vinstant_platform.cpp
HEADERS += $${BASE}/common/lib/vault/source/threads/_win/vthread_platform.h
SOURCES += $${BASE}/common/lib/vault/source/threads/_win/vthread_platform.cpp
HEADERS += $${BASE}/common/lib/vault/source/sockets/_win/vsocket.h
SOURCES += $${BASE}/common/lib/vault/source/sockets/_win/vsocket.cpp
}

DEPENDPATH += $${BASE}/common/lib/vault/source
DEPENDPATH += $${BASE}/common/lib/vault/source/containers
DEPENDPATH += $${BASE}/common/lib/vault/source/files
DEPENDPATH += $${BASE}/common/lib/vault/source/server
DEPENDPATH += $${BASE}/common/lib/vault/source/sockets
DEPENDPATH += $${BASE}/common/lib/vault/source/streams
DEPENDPATH += $${BASE}/common/lib/vault/source/threads
DEPENDPATH += $${BASE}/common/lib/vault/source/toolbox
DEPENDPATH += $${BASE}/common/lib/vault/source/unittest
DEPENDPATH += $${BASE}/common/lib/vault/source/vtypes

INCLUDEPATH += $${BASE}/common/lib/vault/source
INCLUDEPATH += $${BASE}/common/lib/vault/source/containers
INCLUDEPATH += $${BASE}/common/lib/vault/source/files
INCLUDEPATH += $${BASE}/common/lib/vault/source/server
INCLUDEPATH += $${BASE}/common/lib/vault/source/sockets
INCLUDEPATH += $${BASE}/common/lib/vault/source/streams
INCLUDEPATH += $${BASE}/common/lib/vault/source/threads
INCLUDEPATH += $${BASE}/common/lib/vault/source/toolbox
INCLUDEPATH += $${BASE}/common/lib/vault/source/unittest
INCLUDEPATH += $${BASE}/common/lib/vault/source/vtypes

HEADERS += $${BASE}/common/lib/vault/source/vault.h
HEADERS += $${BASE}/common/lib/vault/source/vtypes/vtypes.h
SOURCES += $${BASE}/common/lib/vault/source/vtypes/vtypes.cpp
HEADERS += $${BASE}/common/lib/vault/source/containers/vbento.h
SOURCES += $${BASE}/common/lib/vault/source/containers/vbento.cpp
HEADERS += $${BASE}/common/lib/vault/source/containers/vchar.h
SOURCES += $${BASE}/common/lib/vault/source/containers/vchar.cpp
HEADERS += $${BASE}/common/lib/vault/source/containers/vexception.h
SOURCES += $${BASE}/common/lib/vault/source/containers/vexception.cpp
HEADERS += $${BASE}/common/lib/vault/source/containers/vinstant.h
SOURCES += $${BASE}/common/lib/vault/source/containers/vinstant.cpp
HEADERS += $${BASE}/common/lib/vault/source/containers/vstring.h
SOURCES += $${BASE}/common/lib/vault/source/containers/vstring.cpp
HEADERS += $${BASE}/common/lib/vault/source/files/vbufferedfilestream.h
SOURCES += $${BASE}/common/lib/vault/source/files/vbufferedfilestream.cpp
HEADERS += $${BASE}/common/lib/vault/source/files/vfilestream.h
SOURCES += $${BASE}/common/lib/vault/source/files/vfilestream.cpp
HEADERS += $${BASE}/common/lib/vault/source/files/vfsnode.h
SOURCES += $${BASE}/common/lib/vault/source/files/vfsnode.cpp
HEADERS += $${BASE}/common/lib/vault/source/server/vlistenersocket.h
SOURCES += $${BASE}/common/lib/vault/source/server/vlistenersocket.cpp
HEADERS += $${BASE}/common/lib/vault/source/server/vlistenerthread.h
SOURCES += $${BASE}/common/lib/vault/source/server/vlistenerthread.cpp
HEADERS += $${BASE}/common/lib/vault/source/server/vmanagementinterface.h
HEADERS += $${BASE}/common/lib/vault/source/sockets/vsocketbase.h
SOURCES += $${BASE}/common/lib/vault/source/sockets/vsocketbase.cpp
HEADERS += $${BASE}/common/lib/vault/source/sockets/vsocketfactory.h
SOURCES += $${BASE}/common/lib/vault/source/sockets/vsocketfactory.cpp
HEADERS += $${BASE}/common/lib/vault/source/sockets/vsocketstream.h
SOURCES += $${BASE}/common/lib/vault/source/sockets/vsocketstream.cpp
HEADERS += $${BASE}/common/lib/vault/source/sockets/vsocketthread.h
SOURCES += $${BASE}/common/lib/vault/source/sockets/vsocketthread.cpp
HEADERS += $${BASE}/common/lib/vault/source/sockets/vsocketthreadfactory.h
HEADERS += $${BASE}/common/lib/vault/source/streams/vbinaryiostream.h
SOURCES += $${BASE}/common/lib/vault/source/streams/vbinaryiostream.cpp
HEADERS += $${BASE}/common/lib/vault/source/streams/viostream.h
SOURCES += $${BASE}/common/lib/vault/source/streams/viostream.cpp
HEADERS += $${BASE}/common/lib/vault/source/streams/vmemorystream.h
SOURCES += $${BASE}/common/lib/vault/source/streams/vmemorystream.cpp
HEADERS += $${BASE}/common/lib/vault/source/streams/vstream.h
SOURCES += $${BASE}/common/lib/vault/source/streams/vstream.cpp
HEADERS += $${BASE}/common/lib/vault/source/streams/vstreamcopier.h
SOURCES += $${BASE}/common/lib/vault/source/streams/vstreamcopier.cpp
HEADERS += $${BASE}/common/lib/vault/source/streams/vtextiostream.h
SOURCES += $${BASE}/common/lib/vault/source/streams/vtextiostream.cpp
HEADERS += $${BASE}/common/lib/vault/source/streams/vwritebufferedstream.h
SOURCES += $${BASE}/common/lib/vault/source/streams/vwritebufferedstream.cpp
HEADERS += $${BASE}/common/lib/vault/source/threads/vmutex.h
SOURCES += $${BASE}/common/lib/vault/source/threads/vmutex.cpp
HEADERS += $${BASE}/common/lib/vault/source/threads/vmutexlocker.h
SOURCES += $${BASE}/common/lib/vault/source/threads/vmutexlocker.cpp
HEADERS += $${BASE}/common/lib/vault/source/threads/vsemaphore.h
SOURCES += $${BASE}/common/lib/vault/source/threads/vsemaphore.cpp
HEADERS += $${BASE}/common/lib/vault/source/threads/vthread.h
SOURCES += $${BASE}/common/lib/vault/source/threads/vthread.cpp
HEADERS += $${BASE}/common/lib/vault/source/toolbox/vclassregistry.h
SOURCES += $${BASE}/common/lib/vault/source/toolbox/vclassregistry.cpp
HEADERS += $${BASE}/common/lib/vault/source/toolbox/vhex.h
SOURCES += $${BASE}/common/lib/vault/source/toolbox/vhex.cpp
HEADERS += $${BASE}/common/lib/vault/source/toolbox/vlogger.h
SOURCES += $${BASE}/common/lib/vault/source/toolbox/vlogger.cpp
HEADERS += $${BASE}/common/lib/vault/source/toolbox/vsettings.h
SOURCES += $${BASE}/common/lib/vault/source/toolbox/vsettings.cpp
HEADERS += $${BASE}/common/lib/vault/source/unittest/vbentounit.h
SOURCES += $${BASE}/common/lib/vault/source/unittest/vbentounit.cpp
HEADERS += $${BASE}/common/lib/vault/source/unittest/vbinaryiounit.h
SOURCES += $${BASE}/common/lib/vault/source/unittest/vbinaryiounit.cpp
HEADERS += $${BASE}/common/lib/vault/source/unittest/vcharunit.h
SOURCES += $${BASE}/common/lib/vault/source/unittest/vcharunit.cpp
HEADERS += $${BASE}/common/lib/vault/source/unittest/vclassregistryunit.h
SOURCES += $${BASE}/common/lib/vault/source/unittest/vclassregistryunit.cpp
HEADERS += $${BASE}/common/lib/vault/source/unittest/vexceptionunit.h
SOURCES += $${BASE}/common/lib/vault/source/unittest/vexceptionunit.cpp
HEADERS += $${BASE}/common/lib/vault/source/unittest/vfsnodeunit.h
SOURCES += $${BASE}/common/lib/vault/source/unittest/vfsnodeunit.cpp
HEADERS += $${BASE}/common/lib/vault/source/unittest/vhexunit.h
SOURCES += $${BASE}/common/lib/vault/source/unittest/vhexunit.cpp
HEADERS += $${BASE}/common/lib/vault/source/unittest/vinstantunit.h
SOURCES += $${BASE}/common/lib/vault/source/unittest/vinstantunit.cpp
HEADERS += $${BASE}/common/lib/vault/source/unittest/vstreamsunit.h
SOURCES += $${BASE}/common/lib/vault/source/unittest/vstreamsunit.cpp
HEADERS += $${BASE}/common/lib/vault/source/unittest/vstringunit.h
SOURCES += $${BASE}/common/lib/vault/source/unittest/vstringunit.cpp
HEADERS += $${BASE}/common/lib/vault/source/unittest/vthreadsunit.h
SOURCES += $${BASE}/common/lib/vault/source/unittest/vthreadsunit.cpp
HEADERS += $${BASE}/common/lib/vault/source/unittest/vunit.h
SOURCES += $${BASE}/common/lib/vault/source/unittest/vunit.cpp
HEADERS += $${BASE}/common/lib/vault/source/unittest/vunitrunall.h
SOURCES += $${BASE}/common/lib/vault/source/unittest/vunitrunall.cpp

