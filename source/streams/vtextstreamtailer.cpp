/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.1
http://www.bombaydigital.com/
License: MIT. See LICENSE.md in the Vault top level directory.
*/

/** @file */

#include "vtextstreamtailer.h"

#include "vexception.h"

VTailingTextInputStream::VTailingTextInputStream(VStream& rawStream, int lineEndingsWriteKind)
    : VTextIOStream(rawStream, lineEndingsWriteKind)
    {
}

VTailingTextInputStream::~VTailingTextInputStream() {
    mTailer = NULL;
}

void VTailingTextInputStream::attachTailer(VTextStreamTailer* tailer) {
    mTailer = tailer;
}

void VTailingTextInputStream::readGuaranteed(Vu8* targetBuffer, Vs64 numBytesToRead) {
    while (mTailer->isRunning()) {
        if (this->available() >= numBytesToRead) {
            return VTextIOStream::readGuaranteed(targetBuffer, numBytesToRead);
        } else {
            VThread::sleep(mTailer->getSleepInterval());
        }
    }

    throw VEOFException(VString::EMPTY());
}

Vu8 VTailingTextInputStream::readGuaranteedByte() {
    while (mTailer->isRunning()) {
        if (this->available() > 0) {
            return VTextIOStream::readGuaranteedByte();
        } else {
            VThread::sleep(mTailer->getSleepInterval());
        }
    }

    throw VEOFException(VString::EMPTY());
}

// ---------------


VTextStreamTailer::VTextStreamTailer(VTailingTextInputStream& inputStream, bool processLines, VDuration sleepInterval, const VString& loggerName)
    : VThread("VTextStreamTailer", loggerName, kDontDeleteSelfAtEnd, kCreateThreadDetached, nullptr)
    , mInputStream(inputStream)
    , mProcessLines(processLines)
    , mSleepInterval(sleepInterval)
    {
}

VTextStreamTailer::~VTextStreamTailer() {
    if (this->isRunning()) {
        this->stop();
    }
}

void VTextStreamTailer::start() {
    mInputStream.attachTailer(this);
    VThread::start();
}

void VTextStreamTailer::stop() {
    VThread::stop();
    VThread::sleep(this->getSleepInterval());
}

void VTextStreamTailer::run() {
    VString line;
    while (this->isRunning()) {
        try {
            if (mProcessLines) {
                mInputStream.readLine(line);
                this->processLine(line);
            } else {
                this->processCodePoint(mInputStream.readUTF8CodePoint());
            }
        } catch (const VEOFException&) { // just keep trying
            VThread::sleep(this->getSleepInterval());
            // normal end when someone calls stopTailing
            // if still running, something is wrong
            //if (this->isRunning()) {
            //    this->stop();
            //    VLOGGER_ERROR("EOF in tailer while still running");
            //}
        }
    }
}
