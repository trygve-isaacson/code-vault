# This is a qmake project file.
# To use it, cd to this directory and run qmake.
# By default it generates a Makefile, which you can run make on.
# With option flags it can generate a VC++ or ProjectBuilder project file.

TEMPLATE	= app
CONFIG		+= warn_on debug thread staticlib exceptions rtti stl
CONFIG		-= qt
HEADERS		=

TARGET		= unit_test

# Must define BASE for included .pri files themselves to use correct paths,
# so we may as well use it ourself. This should be the base dir where Ant
# scripts are executed from.
BASE = .

# With these conditional sections, take note of the hierarchy:
# On Mac OS X, both "macx" and "unix" are defined.
# On HP-UX, both "hpux" and "unix" defined.
# So we have to use if-else rather than using separate standalone clauses.
# Otherwise, multiple clauses would be evaluated.

unix {

	macx {

    	DESTDIR		= $${BASE}/../../build/vault/macosx
    	OBJECTS_DIR	= $${BASE}/../../build/vault/macosx/obj
    	# Non-library code on Mac OS X should use this option (non-PC-relative code addressing)
    	# to avoid unnecessary function call overhead. Supported by gcc 3.3 and later on Mac OS X.
    	QMAKE_CXXFLAGS += -mdynamic-no-pic

    	# This will give us the ODBC library present on Mac OS X.
    	LIBS += -liodbc

	} else:hpux-g++ {

    	DESTDIR		= $${BASE}/../../build/vault/unix
    	OBJECTS_DIR	= $${BASE}/../../build/vault/unix/obj

    	# This will give us the unixODBC library, which must be installed.
    	INCLUDEPATH += /usr/local/include
    	LIBS += -lodbc -L/usr/local/lib

	} else:linux-g++ {

    	DESTDIR		= $${BASE}/../../build/vault/unix
    	OBJECTS_DIR	= $${BASE}/../../build/vault/unix/obj
    	# I've replaced all use of "#pragma once" with old-skool include guards.
    	# But if you use "#pragma once" you'll need this to prevent GCC on Linux from
    	# complaining about your use of a deprecated and unsupported preprocessor macro:
    	#QMAKE_CXXFLAGS += -Wno-deprecated

    	# This will give us the unixODBC library, which must be installed.
    	INCLUDEPATH += /usr/local/include
    	LIBS += -lodbc

	}

} else:win32 {

    DESTDIR		= $${BASE}/../../build/vault/win
    OBJECTS_DIR	= $${BASE}/../../build/vault/win/obj

    # Command line builds for Windows seem to need these for the SCM and ODBC calls.
    # They are apparently part of the default set of libraries in the Visual Studio IDE.
    LIBS += advapi32.lib
    LIBS += odbc32.lib
    LIBS += User32.lib

}

# Main function for this application build:
SOURCES	= $${BASE}/source/unittest/vplatformcheck_main.cpp

include($${BASE}/build.pri)
