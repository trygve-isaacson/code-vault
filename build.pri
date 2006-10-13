# Copyright c1997-2006 Trygve Isaacson. All rights reserved.
# This file is part of the Code Vault version 2.5
# http://www.bombaydigital.com/

#
# This is a qmake include file -- to be included from a .pro file.
#
# Because paths are relative to the main file (not this one), this
# file relies on the main file defining the variable $${VAULT_BASE} so
# that this file can specify paths correctly.
#

win32:LIBS += wsock32.lib
win32:LIBS += psapi.lib

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
DEPENDPATH += $${VAULT_BASE}/source/vtypes/_mac
DEPENDPATH += $${VAULT_BASE}/source/threads/_unix
DEPENDPATH += $${VAULT_BASE}/source/sockets/_unix
DEPENDPATH += $${VAULT_BASE}/source/files/_unix

INCLUDEPATH += $${VAULT_BASE}/source/vtypes/_mac
INCLUDEPATH += $${VAULT_BASE}/source/threads/_unix
INCLUDEPATH += $${VAULT_BASE}/source/sockets/_unix
INCLUDEPATH += $${VAULT_BASE}/source/files/_unix

HEADERS += $${VAULT_BASE}/source/vtypes/_mac/vtypes_platform.h
SOURCES += $${VAULT_BASE}/source/vtypes/_mac/vtypes_platform.cpp
SOURCES += $${VAULT_BASE}/source/containers/_unix/vinstant_platform.cpp
SOURCES += $${VAULT_BASE}/source/files/_unix/vfsnode_platform.cpp
HEADERS += $${VAULT_BASE}/source/threads/_unix/vthread_platform.h
SOURCES += $${VAULT_BASE}/source/threads/_unix/vthread_platform.cpp
HEADERS += $${VAULT_BASE}/source/sockets/_unix/vsocket.h
SOURCES += $${VAULT_BASE}/source/sockets/_unix/vsocket.cpp
}

vault_unix {
DEPENDPATH += $${VAULT_BASE}/source/vtypes/_unix
DEPENDPATH += $${VAULT_BASE}/source/threads/_unix
DEPENDPATH += $${VAULT_BASE}/source/sockets/_unix
DEPENDPATH += $${VAULT_BASE}/source/files/_unix

INCLUDEPATH += $${VAULT_BASE}/source/vtypes/_unix
INCLUDEPATH += $${VAULT_BASE}/source/threads/_unix
INCLUDEPATH += $${VAULT_BASE}/source/sockets/_unix
INCLUDEPATH += $${VAULT_BASE}/source/files/_unix

HEADERS += $${VAULT_BASE}/source/vtypes/_unix/vtypes_platform.h
SOURCES += $${VAULT_BASE}/source/vtypes/_unix/vtypes_platform.cpp
SOURCES += $${VAULT_BASE}/source/containers/_unix/vinstant_platform.cpp
SOURCES += $${VAULT_BASE}/source/files/_unix/vfsnode_platform.cpp
HEADERS += $${VAULT_BASE}/source/threads/_unix/vthread_platform.h
SOURCES += $${VAULT_BASE}/source/threads/_unix/vthread_platform.cpp
HEADERS += $${VAULT_BASE}/source/sockets/_unix/vsocket.h
SOURCES += $${VAULT_BASE}/source/sockets/_unix/vsocket.cpp
}

vault_win32 {
DEPENDPATH += $${VAULT_BASE}/source/vtypes/_win
DEPENDPATH += $${VAULT_BASE}/source/threads/_win
DEPENDPATH += $${VAULT_BASE}/source/sockets/_win
DEPENDPATH += $${VAULT_BASE}/source/files/_win

INCLUDEPATH += $${VAULT_BASE}/source/vtypes/_win
INCLUDEPATH += $${VAULT_BASE}/source/threads/_win
INCLUDEPATH += $${VAULT_BASE}/source/sockets/_win
INCLUDEPATH += $${VAULT_BASE}/source/files/_win

HEADERS += $${VAULT_BASE}/source/vtypes/_win/vtypes_platform.h
SOURCES += $${VAULT_BASE}/source/vtypes/_win/vtypes_platform.cpp
SOURCES += $${VAULT_BASE}/source/containers/_win/vinstant_platform.cpp
SOURCES += $${VAULT_BASE}/source/files/_win/vfsnode_platform.cpp
HEADERS += $${VAULT_BASE}/source/threads/_win/vthread_platform.h
SOURCES += $${VAULT_BASE}/source/threads/_win/vthread_platform.cpp
HEADERS += $${VAULT_BASE}/source/sockets/_win/vsocket.h
SOURCES += $${VAULT_BASE}/source/sockets/_win/vsocket.cpp

LINKER_LIBRARIES += Psapi.lib
}

DEPENDPATH += $${VAULT_BASE}/source
DEPENDPATH += $${VAULT_BASE}/source/containers
DEPENDPATH += $${VAULT_BASE}/source/files
DEPENDPATH += $${VAULT_BASE}/source/server
DEPENDPATH += $${VAULT_BASE}/source/sockets
DEPENDPATH += $${VAULT_BASE}/source/streams
DEPENDPATH += $${VAULT_BASE}/source/threads
DEPENDPATH += $${VAULT_BASE}/source/toolbox
DEPENDPATH += $${VAULT_BASE}/source/unittest
DEPENDPATH += $${VAULT_BASE}/source/vtypes

INCLUDEPATH += $${VAULT_BASE}/source
INCLUDEPATH += $${VAULT_BASE}/source/containers
INCLUDEPATH += $${VAULT_BASE}/source/files
INCLUDEPATH += $${VAULT_BASE}/source/server
INCLUDEPATH += $${VAULT_BASE}/source/sockets
INCLUDEPATH += $${VAULT_BASE}/source/streams
INCLUDEPATH += $${VAULT_BASE}/source/threads
INCLUDEPATH += $${VAULT_BASE}/source/toolbox
INCLUDEPATH += $${VAULT_BASE}/source/unittest
INCLUDEPATH += $${VAULT_BASE}/source/vtypes

HEADERS += $${VAULT_BASE}/source/vault.h
HEADERS += $${VAULT_BASE}/source/vconfigure.h
HEADERS += $${VAULT_BASE}/source/vtypes/vtypes.h
SOURCES += $${VAULT_BASE}/source/vtypes/vtypes.cpp
HEADERS += $${VAULT_BASE}/source/containers/vbento.h
SOURCES += $${VAULT_BASE}/source/containers/vbento.cpp
HEADERS += $${VAULT_BASE}/source/containers/vchar.h
SOURCES += $${VAULT_BASE}/source/containers/vchar.cpp
HEADERS += $${VAULT_BASE}/source/containers/vexception.h
SOURCES += $${VAULT_BASE}/source/containers/vexception.cpp
HEADERS += $${VAULT_BASE}/source/containers/vinstant.h
SOURCES += $${VAULT_BASE}/source/containers/vinstant.cpp
HEADERS += $${VAULT_BASE}/source/containers/vstring.h
SOURCES += $${VAULT_BASE}/source/containers/vstring.cpp
HEADERS += $${VAULT_BASE}/source/files/vabstractfilestream.h
SOURCES += $${VAULT_BASE}/source/files/vabstractfilestream.cpp
HEADERS += $${VAULT_BASE}/source/files/vbufferedfilestream.h
SOURCES += $${VAULT_BASE}/source/files/vbufferedfilestream.cpp
HEADERS += $${VAULT_BASE}/source/files/vdirectiofilestream.h
SOURCES += $${VAULT_BASE}/source/files/vdirectiofilestream.cpp
HEADERS += $${VAULT_BASE}/source/files/vfsnode.h
SOURCES += $${VAULT_BASE}/source/files/vfsnode.cpp
HEADERS += $${VAULT_BASE}/source/server/vlistenersocket.h
SOURCES += $${VAULT_BASE}/source/server/vlistenersocket.cpp
HEADERS += $${VAULT_BASE}/source/server/vlistenerthread.h
SOURCES += $${VAULT_BASE}/source/server/vlistenerthread.cpp
HEADERS += $${VAULT_BASE}/source/server/vmanagementinterface.h
HEADERS += $${VAULT_BASE}/source/sockets/vsocketbase.h
SOURCES += $${VAULT_BASE}/source/sockets/vsocketbase.cpp
HEADERS += $${VAULT_BASE}/source/sockets/vsocketfactory.h
SOURCES += $${VAULT_BASE}/source/sockets/vsocketfactory.cpp
HEADERS += $${VAULT_BASE}/source/sockets/vsocketstream.h
SOURCES += $${VAULT_BASE}/source/sockets/vsocketstream.cpp
HEADERS += $${VAULT_BASE}/source/sockets/vsocketthread.h
SOURCES += $${VAULT_BASE}/source/sockets/vsocketthread.cpp
HEADERS += $${VAULT_BASE}/source/sockets/vsocketthreadfactory.h
HEADERS += $${VAULT_BASE}/source/streams/vbinaryiostream.h
SOURCES += $${VAULT_BASE}/source/streams/vbinaryiostream.cpp
HEADERS += $${VAULT_BASE}/source/streams/viostream.h
SOURCES += $${VAULT_BASE}/source/streams/viostream.cpp
HEADERS += $${VAULT_BASE}/source/streams/vmemorystream.h
SOURCES += $${VAULT_BASE}/source/streams/vmemorystream.cpp
HEADERS += $${VAULT_BASE}/source/streams/vstream.h
SOURCES += $${VAULT_BASE}/source/streams/vstream.cpp
HEADERS += $${VAULT_BASE}/source/streams/vstreamcopier.h
SOURCES += $${VAULT_BASE}/source/streams/vstreamcopier.cpp
HEADERS += $${VAULT_BASE}/source/streams/vtextiostream.h
SOURCES += $${VAULT_BASE}/source/streams/vtextiostream.cpp
HEADERS += $${VAULT_BASE}/source/streams/vwritebufferedstream.h
SOURCES += $${VAULT_BASE}/source/streams/vwritebufferedstream.cpp
HEADERS += $${VAULT_BASE}/source/threads/vmutex.h
SOURCES += $${VAULT_BASE}/source/threads/vmutex.cpp
HEADERS += $${VAULT_BASE}/source/threads/vmutexlocker.h
SOURCES += $${VAULT_BASE}/source/threads/vmutexlocker.cpp
HEADERS += $${VAULT_BASE}/source/threads/vsemaphore.h
SOURCES += $${VAULT_BASE}/source/threads/vsemaphore.cpp
HEADERS += $${VAULT_BASE}/source/threads/vthread.h
SOURCES += $${VAULT_BASE}/source/threads/vthread.cpp
HEADERS += $${VAULT_BASE}/source/toolbox/vclassregistry.h
SOURCES += $${VAULT_BASE}/source/toolbox/vclassregistry.cpp
HEADERS += $${VAULT_BASE}/source/toolbox/vhex.h
SOURCES += $${VAULT_BASE}/source/toolbox/vhex.cpp
HEADERS += $${VAULT_BASE}/source/toolbox/vlogger.h
SOURCES += $${VAULT_BASE}/source/toolbox/vlogger.cpp
HEADERS += $${VAULT_BASE}/source/toolbox/vsettings.h
SOURCES += $${VAULT_BASE}/source/toolbox/vsettings.cpp
HEADERS += $${VAULT_BASE}/source/toolbox/vshutdownregistry.h
SOURCES += $${VAULT_BASE}/source/toolbox/vshutdownregistry.cpp
HEADERS += $${VAULT_BASE}/source/toolbox/vsingleton.h
HEADERS += $${VAULT_BASE}/source/unittest/vbentounit.h
SOURCES += $${VAULT_BASE}/source/unittest/vbentounit.cpp
HEADERS += $${VAULT_BASE}/source/unittest/vbinaryiounit.h
SOURCES += $${VAULT_BASE}/source/unittest/vbinaryiounit.cpp
HEADERS += $${VAULT_BASE}/source/unittest/vcharunit.h
SOURCES += $${VAULT_BASE}/source/unittest/vcharunit.cpp
HEADERS += $${VAULT_BASE}/source/unittest/vclassregistryunit.h
SOURCES += $${VAULT_BASE}/source/unittest/vclassregistryunit.cpp
HEADERS += $${VAULT_BASE}/source/unittest/vexceptionunit.h
SOURCES += $${VAULT_BASE}/source/unittest/vexceptionunit.cpp
HEADERS += $${VAULT_BASE}/source/unittest/vfsnodeunit.h
SOURCES += $${VAULT_BASE}/source/unittest/vfsnodeunit.cpp
HEADERS += $${VAULT_BASE}/source/unittest/vhexunit.h
SOURCES += $${VAULT_BASE}/source/unittest/vhexunit.cpp
HEADERS += $${VAULT_BASE}/source/unittest/vinstantunit.h
SOURCES += $${VAULT_BASE}/source/unittest/vinstantunit.cpp
HEADERS += $${VAULT_BASE}/source/unittest/vplatformunit.h
SOURCES += $${VAULT_BASE}/source/unittest/vplatformunit.cpp
HEADERS += $${VAULT_BASE}/source/unittest/vstreamsunit.h
SOURCES += $${VAULT_BASE}/source/unittest/vstreamsunit.cpp
HEADERS += $${VAULT_BASE}/source/unittest/vstringunit.h
SOURCES += $${VAULT_BASE}/source/unittest/vstringunit.cpp
HEADERS += $${VAULT_BASE}/source/unittest/vthreadsunit.h
SOURCES += $${VAULT_BASE}/source/unittest/vthreadsunit.cpp
HEADERS += $${VAULT_BASE}/source/unittest/vunit.h
SOURCES += $${VAULT_BASE}/source/unittest/vunit.cpp
HEADERS += $${VAULT_BASE}/source/unittest/vunitrunall.h
SOURCES += $${VAULT_BASE}/source/unittest/vunitrunall.cpp

