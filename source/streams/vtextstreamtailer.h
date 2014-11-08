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
#include "vchar.h"

/**
    @ingroup viostream_derived
*/

class VTextStreamTailer;

// This class works only in conjunction with a VTextStreamTailer object.
class VTailingTextInputStream : public VTextIOStream {
    public:
        VTailingTextInputStream(VStream& rawStream, int lineEndingsWriteKind = kUseNativeLineEndings);
        virtual ~VTailingTextInputStream();
    
        void attachTailer(VTextStreamTailer* tailer);

        // We just override the two ways of reading bytes; if desired bytes are not yet available, we sleep.
        virtual void readGuaranteed(Vu8* targetBuffer, Vs64 numBytesToRead);
        virtual Vu8 readGuaranteedByte();

    private:
    
        VTextStreamTailer* mTailer;
};

/**
VTextStreamTailer is a utility class that will 'tail' a text stream (such as a
text file on disk). It is an abstract base class; you implement the two callback
methods to process the tailed data.

A separate thread is used, and it will call you with each line or character
(as you specify). Whenever there is no more data ready to read on the input stream,
the tailer will sleep for a configurable amount of time before attempting to read
again. Thus, the callbacks will be called as soon as data is ready, with a certain
amount of latency.
*/
class VTextStreamTailer : public VThread {
    public:

        /**
        Constructs the object with an underlying input stream that will be tailed.
        @param  inputStream     the stream to tail
        @param  processLines    if true, each line tailed will be passed to the
            processLine() callback; if false, each codepoint tailed will be
            passed to the processCodePoint() callback
        @param  sleepInterval   duration to sleep before each attempt to read more data
        @param  loggerName      logger to which any errors will be logged
        */
        VTextStreamTailer(VTailingTextInputStream& inputStream, bool processLines=true, VDuration sleepInterval=VDuration::SECOND(), const VString& loggerName=VString::EMPTY());
        /**
        Destructor.
        */
        virtual ~VTextStreamTailer();

        virtual void start();
        virtual void stop();
        virtual void run();

        VDuration getSleepInterval() const { return mSleepInterval; }
    
        virtual void processLine(const VString& line) = 0;
        virtual void processCodePoint(const VCodePoint& c) = 0;

    private:
    
        VTailingTextInputStream&    mInputStream;
        bool                        mProcessLines;
        VDuration                   mSleepInterval;
};

#endif /* vtextstreamtailer_h */
