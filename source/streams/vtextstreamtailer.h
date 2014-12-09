/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.1
http://www.bombaydigital.com/
License: MIT. See LICENSE.md in the Vault top level directory.
*/

#ifndef vtextstreamtailer_h
#define vtextstreamtailer_h

/** @file */

#include "vtextiostream.h"
#include "vthread.h"
#include "vmutexlocker.h"

/**
    @ingroup viostream_derived
*/

/**
This class defines the interface that you must implement to handle VTextTailRunner data.
If you are doing line-by-line tailing, provide an implementation of processLine(), which will be
called for each tailed line.
If you are doing codepoint-by-codepoint tailing, provide an implementation of processCodePoint(),
which will be called for each tailed code point.
Only one of the functions will be called, based on what you specify for the processByLine parameter
of the VTextTailRunner constructor.
*/
class VTailHandler {
    public:
        VTailHandler() {}
        virtual ~VTailHandler() {}

        /**
        Called to process a complete tailed line, if tailing by line. The call is executed
        on the tail runner's separate thread.
        Line endings are not included when tailing by line.
        @param  line    the line to process
        */
        virtual void processLine(const VString& /*line*/) {}
        /**
        Called to process a code point, if tailing by code point. The call is executed
        on the tail runner's separate thread.
        Line endings are included when tailing by code point.
        @param  c   the code point to process
        */
        virtual void processCodePoint(const VCodePoint& /*c*/) {}
};

// VTailRunnerTextInputStream is privately implemented; use VSharedPtr since we can't embed.
class VTailRunnerTextInputStream;
typedef VSharedPtr<VTailRunnerTextInputStream> VTailRunnerTextInputStreamPtr;

/**
This class manages tailing an input stream (such as a file). You instantiate it and then start it running.
While running, it will call your handler, on a separate thread, with either the lines or code points
of the input stream, starting at the initial stream offset, and over time as further data exists or is
appended to the stream. This allows you to, for example, either set the input stream offset to the start
of a file in order to read and tail the entire file and any data that is subsequently appended to the file,
or set the offset to the end of the file in order to tail only the data that is subsequently appended to
the file.
*/
class VTextTailRunner {
    public:
        /**
        Constructs a tail runner for an arbitrary input stream.
        @param  inputStream     the stream to tail
        @param  handler         the handler to call with tailed lines or code points
        @param  processByLine   true if lines are to be handled; false if individual code points are to be handled
        @param  sleepDuration   the interval to sleep when there is no data available to read
        @param  loggerName      the logger name to be used when emitting log output
        */
        VTextTailRunner(VStream& inputStream, VTailHandler& handler, bool processByLine=true, VDuration sleepDuration=VDuration::SECOND(), const VString& loggerName=VString::EMPTY());
        /**
        Constructs a tail runner for an input file.
        @param  inputFile       the file to tail
        @param  handler         the handler to call with tailed lines or code points
        @param  processByLine   true if lines are to be handled; false if individual code points are to be handled
        @param  sleepDuration   the interval to sleep when there is no data available to read
        @param  loggerName      the logger name to be used when emitting log output
        */
        VTextTailRunner(const VFSNode& inputFile, VTailHandler& handler, bool processByLine=true, VDuration sleepDuration=VDuration::SECOND(), const VString& loggerName=VString::EMPTY());
        virtual ~VTextTailRunner();

        /**
        Starts the tailing thread.
        */
        void start();
        /**
        Stops the tailing thread, which will subsequently destruct.
        */
        void stop();
        /**
        Returns true if the tailing thread is running.
        */
        bool isRunning() const;

    private:
    
        VBufferedFileStream             mInputFileStream;   ///< The file stream, if VFSNode constructor form was used.
        VTailRunnerTextInputStreamPtr   mInputStream;       ///< The input stream, either as supplied or for the file.
        VTailHandler&                   mHandler;           ///< The handler to be called with each line or code point tailed.
        bool                            mProcessByLine;     ///< True if we are tailing line-by-line, vs. by code point.
        VDuration                       mSleepDuration;     ///< The interval to sleep when there is no data available to read.
        VString                         mLoggerName;        ///< The logger name to be used when emitting log output.
    
        mutable VMutex                  mMutex;             ///< Synchronizes access to mTailThread.
        VThread*                        mTailThread;        ///< The thread that does the actual tailing.
    
};

#endif /* vtextstreamtailer_h */
