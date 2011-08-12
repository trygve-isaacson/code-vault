# Copyright c1997-2008 Trygve Isaacson. All rights reserved.
# This file is part of the Code Vault version 3.0
# http://www.bombaydigital.com/

#
# This is a qmake include file -- to be included from a .pro file.
#
# Because paths are relative to the main file (not this one), this
# file relies on the main file defining the variable $${VAULT_BASE} so
# that this file can specify paths correctly.
#

DEPENDPATH += $${VAULT_BASE}/source/unittest
INCLUDEPATH += $${VAULT_BASE}/source/unittest
HEADERS += $${VAULT_BASE}/source/unittest/vassertunit.h
SOURCES += $${VAULT_BASE}/source/unittest/vassertunit.cpp
HEADERS += $${VAULT_BASE}/source/unittest/vbentounit.h
SOURCES += $${VAULT_BASE}/source/unittest/vbentounit.cpp
HEADERS += $${VAULT_BASE}/source/unittest/vbinaryiounit.h
SOURCES += $${VAULT_BASE}/source/unittest/vbinaryiounit.cpp
HEADERS += $${VAULT_BASE}/source/unittest/vcharunit.h
SOURCES += $${VAULT_BASE}/source/unittest/vcharunit.cpp
HEADERS += $${VAULT_BASE}/source/unittest/vclassregistryunit.h
SOURCES += $${VAULT_BASE}/source/unittest/vclassregistryunit.cpp
HEADERS += $${VAULT_BASE}/source/unittest/vcolorunit.h
SOURCES += $${VAULT_BASE}/source/unittest/vcolorunit.cpp
HEADERS += $${VAULT_BASE}/source/unittest/vexceptionunit.h
SOURCES += $${VAULT_BASE}/source/unittest/vexceptionunit.cpp
HEADERS += $${VAULT_BASE}/source/unittest/vfsnodeunit.h
SOURCES += $${VAULT_BASE}/source/unittest/vfsnodeunit.cpp
HEADERS += $${VAULT_BASE}/source/unittest/vgeometryunit.h
SOURCES += $${VAULT_BASE}/source/unittest/vgeometryunit.cpp
HEADERS += $${VAULT_BASE}/source/unittest/vhexunit.h
SOURCES += $${VAULT_BASE}/source/unittest/vhexunit.cpp
HEADERS += $${VAULT_BASE}/source/unittest/vinstantunit.h
SOURCES += $${VAULT_BASE}/source/unittest/vinstantunit.cpp
HEADERS += $${VAULT_BASE}/source/unittest/vloggerunit.h
SOURCES += $${VAULT_BASE}/source/unittest/vloggerunit.cpp
HEADERS += $${VAULT_BASE}/source/unittest/vmessageunit.h
SOURCES += $${VAULT_BASE}/source/unittest/vmessageunit.cpp
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

