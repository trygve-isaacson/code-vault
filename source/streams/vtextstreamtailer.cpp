/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.1
http://www.bombaydigital.com/
License: MIT. See LICENSE.md in the Vault top level directory.
*/

/** @file */

#include "vtextstreamtailer.h"

#include "vexception.h"

// VTailRunnerTextInputStream ------------------------------

class VTailRunnerTextInputStream : public VTextIOStream {
    public:
        VTailRunnerTextInputStream(VTextTailRunner* runner, VStream& rawStream, VDuration sleepDuration, int lineEndingsWriteKind = kUseNativeLineEndings);
        virtual ~VTailRunnerTextInputStream();
    
        // We just override the two ways of reading bytes; if desired bytes are not yet available, we sleep.
        virtual void readGuaranteed(Vu8* targetBuffer, Vs64 numBytesToRead);
        virtual Vu8 readGuaranteedByte();

    private:
    
        VTextTailRunner*    mRunner;
        VDuration           mSleepDuration;
};

// VTextTailRunnerThread ------------------------------

class VTailRunnerThread : public VThread {
    public:
    
        VTailRunnerThread(VTailRunnerTextInputStreamPtr inputStream, VTailHandler& handler, bool processByLine, VDuration sleepDuration);
        virtual ~VTailRunnerThread() {}
    
        virtual void run();

    private:
    
        VTailRunnerTextInputStreamPtr   mInputStream;
        VTailHandler&                   mHandler;
        bool                            mProcessByLine;
        VDuration                       mSleepDuration;
};

// VTextTailRunnerThread ------------------------------

VTailRunnerThread::VTailRunnerThread(VTailRunnerTextInputStreamPtr inputStream, VTailHandler& handler, bool processByLine, VDuration sleepDuration)
    : VThread("VTailRunnerThread", VString::EMPTY(), kDeleteSelfAtEnd, kCreateThreadJoinable, nullptr)
    , mInputStream(inputStream)
    , mHandler(handler)
    , mProcessByLine(processByLine)
    , mSleepDuration(sleepDuration)
    {
}

void VTailRunnerThread::run() {
    VString line;
    while (this->isRunning()) {
        try {
            if (mProcessByLine) {
                mInputStream->readLine(line);
                mHandler.processLine(line);
            } else {
                VCodePoint c = mInputStream->readUTF8CodePoint();
                mHandler.processCodePoint(c);
            }
        } catch (const VEOFException&) { // just keep trying
            VThread::sleep(mSleepDuration);
        }
    }
}

// VTailRunnerTextInputStream ------------------------------

VTailRunnerTextInputStream::VTailRunnerTextInputStream(VTextTailRunner* runner, VStream& rawStream, VDuration sleepDuration, int lineEndingsWriteKind)
    : VTextIOStream(rawStream, lineEndingsWriteKind)
    , mRunner(runner)
    , mSleepDuration(sleepDuration)
    {
}

VTailRunnerTextInputStream::~VTailRunnerTextInputStream() {
    mRunner = nullptr;
}

void VTailRunnerTextInputStream::readGuaranteed(Vu8* targetBuffer, Vs64 numBytesToRead) {
    while (mRunner->isRunning()) {
        if (this->available() >= numBytesToRead) {
            return VTextIOStream::readGuaranteed(targetBuffer, numBytesToRead);
        } else {
            VThread::sleep(mSleepDuration);
        }
    }

    throw VEOFException(VString::EMPTY());
}

Vu8 VTailRunnerTextInputStream::readGuaranteedByte() {
    while (mRunner->isRunning()) {
        if (this->available() > 0) {
            return VTextIOStream::readGuaranteedByte();
        } else {
            VThread::sleep(mSleepDuration);
        }
    }

    throw VEOFException(VString::EMPTY());
}


// VTextTailRunner ------------------------------

VTextTailRunner::VTextTailRunner(VStream& inputStream, VTailHandler& handler, bool processByLine, VDuration sleepDuration, const VString& loggerName)
    : mInputFileStream()
    , mInputStream(new VTailRunnerTextInputStream(this, inputStream, sleepDuration))
    , mHandler(handler)
    , mProcessByLine(processByLine)
    , mSleepDuration(sleepDuration)
    , mLoggerName(loggerName)
    , mMutex("VTextTailRunner")
    , mTailThread(nullptr)
    {
}

VTextTailRunner::VTextTailRunner(const VFSNode& inputFile, VTailHandler& handler, bool processByLine, VDuration sleepDuration, const VString& loggerName)
    : mInputFileStream(inputFile)
    , mInputStream()
    , mHandler(handler)
    , mProcessByLine(processByLine)
    , mSleepDuration(sleepDuration)
    , mLoggerName(loggerName)
    , mMutex("VTextTailRunner")
    , mTailThread(nullptr)
    {
    mInputFileStream.openReadOnly();
    mInputFileStream.seek0();
    
    mInputStream = VTailRunnerTextInputStreamPtr(new VTailRunnerTextInputStream(this, mInputFileStream, sleepDuration));
}

VTextTailRunner::~VTextTailRunner() {
    this->stop();
    VThread::sleep(mSleepDuration + (250 * VDuration::MILLISECOND())); // Allow mTailThread to wake and wind down before we destruct our raw mInputFileStream.
}

void VTextTailRunner::start() {
    mTailThread = new VTailRunnerThread(mInputStream, mHandler, mProcessByLine, mSleepDuration);
    mTailThread->start();
}

void VTextTailRunner::stop() {
    VMutexLocker locker(&mMutex, "VTextTailRunner::stop");
    if (mTailThread != nullptr) {
        mTailThread->stop();
        mTailThread = nullptr;
    }
}

bool VTextTailRunner::isRunning() const {
    VMutexLocker locker(&mMutex, "VTextTailRunner::isRunning");
    return (mTailThread != nullptr) && mTailThread->isRunning();
}
