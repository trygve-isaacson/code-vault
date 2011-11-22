/*
Copyright c1997-2011 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.3
http://www.bombaydigital.com/
*/

/** @file */

#include "vtextiostream.h"

#include "vchar.h"
#include "vexception.h"
#include "vassert.h"

VTextIOStream::VTextIOStream(VStream& rawStream, int lineEndingsWriteKind) :
VIOStream(rawStream),
mLineEndingsReadKind(kLineEndingsUnknown),
mLineEndingsWriteKind(lineEndingsWriteKind),
mPendingCharacter(0),
mReadState(kReadStateReady),
mLineBuffer(),
#ifndef VCOMPILER_MSVC /* GCC EffC++ warnings want this initializer, but then VC++ warns about default initialization. */
mLineEndingChars(),
#endif
mLineEndingCharsLength(0)
    {
    mLineBuffer.preflight(80); // allocate a reasonable buffer size up front to avoid repeated re-allocation
    this->setLineEndingsKind(lineEndingsWriteKind); // install the line ending data to be written
    }

void VTextIOStream::readLine(VString& s, bool includeLineEnding)
    {
    // Note: We append char-by-char, but VString should already be optimized to
    // avoid actually re-allocating its buffer for each single-char expansion.
    
    mLineBuffer = VString::EMPTY();

    bool    readFirstByteOfLine = false;
    Vs64    numBytesToRead = 1;
    Vs64    numBytesRead;
    char    c = 0;
    bool    done = false;
    
    do
        {
        if (mPendingCharacter != 0)
            {
            c = mPendingCharacter;
            mPendingCharacter = 0;
            numBytesRead = 1;
            }
        else
            {
            numBytesRead = this->read(reinterpret_cast<Vu8*> (&c), 1);
            
            // Throw EOF if we fail reading very first byte of line.
            // Otherwise, we'll return whatever we read, and throw next time.
            if (numBytesRead == 0)
                {
                if (readFirstByteOfLine)
                    break; // this line is done
                else
                    throw VEOFException("EOF");
                }
            
            readFirstByteOfLine = true;
            }
        
        switch (mReadState)
            {
            case kReadStateReady:

                if (c == 0x0A)    // found a Unix line end
                    {
                    if (includeLineEnding)
                        mLineBuffer += c;
                    
                    done = true;    // done, bail out and return the string
                    
                    this->_updateLineEndingsReadKind(kLineEndingsUnix);
                    }
                else if (c == 0x0D)    // found a Mac line end, or 1st byte of a DOS line end
                    {
                    mReadState = kReadStateGot0x0D;
                    }
                else    // found a normal character
                    {
                    mLineBuffer += c;
                    }

                break;
            
            case kReadStateGot0x0D:

                if (c == 0x0A)    // found a DOS line end
                    {
                    if (includeLineEnding)
                        {
                        mLineBuffer += static_cast<char> (0x0D);
                        mLineBuffer += static_cast<char> (0x0A);
                        }
                    
                    mReadState = kReadStateReady;
                    done = true;    // done, bail out and return the string

                    this->_updateLineEndingsReadKind(kLineEndingsDOS);
                    }
                else    // found a normal character, so we have a Mac line end pending
                    {
                    if (includeLineEnding)
                        mLineBuffer += static_cast<char> (0x0D);
                        
                    mPendingCharacter = c;

                    mReadState = kReadStateReady;
                    done = true;

                    this->_updateLineEndingsReadKind(kLineEndingsMac);
                    }

                break;
            }

        } while ((numBytesRead == numBytesToRead) && !done);

    s = mLineBuffer;
    }

VChar VTextIOStream::readCharacter()
    {
    char c;
    
    this->readGuaranteed(reinterpret_cast<Vu8*> (&c), 1);

    return c;
    }

void VTextIOStream::readAll(VString& s, bool includeLineEndings)
    {
    try
        {
        VString line;
        for (;;)
            {
            this->readLine(line, includeLineEndings);
            s += line;
            }
        }
    catch (const VEOFException&) {}
    }

void VTextIOStream::readAll(VStringVector& lines)
    {
    try
        {
        VString line;
        for (;;)
            {
            this->readLine(line);
            lines.push_back(line);
            }
        }
    catch (const VEOFException&) {}
    }

void VTextIOStream::writeLine(const VString& s)
    {
    this->writeString(s);
    this->writeLineEnd();
    }

void VTextIOStream::writeString(const VString& s)
    {
    (void) this->write(s.getDataBufferConst(), static_cast<Vs64> (s.length()));
    }

void VTextIOStream::writeLineEnd()
    {
    if (mLineEndingCharsLength != 0)
        (void) this->write(mLineEndingChars, static_cast<Vs64>(mLineEndingCharsLength));
    }

void VTextIOStream::_updateLineEndingsReadKind(int lineEndingKind)
    {
    switch (mLineEndingsReadKind)
        {
        case kLineEndingsUnknown:
            switch (lineEndingKind)
                {
                case kLineEndingsUnknown:
                case kLineEndingsMixed:
                    VASSERT(false);    // invalid input value
                    break;

                case kLineEndingsUnix:
                case kLineEndingsDOS:
                case kLineEndingsMac:
                    mLineEndingsReadKind = lineEndingKind;
                    break;
                }
            break;

        case kLineEndingsUnix:
            switch (lineEndingKind)
                {
                case kLineEndingsUnknown:
                case kLineEndingsMixed:
                    VASSERT(false);    // invalid input value
                    break;

                case kLineEndingsUnix:
                    // no change
                    break;

                case kLineEndingsDOS:
                case kLineEndingsMac:
                    mLineEndingsReadKind = kLineEndingsMixed;
                    break;

                }
            break;

        case kLineEndingsDOS:
            switch (lineEndingKind)
                {
                case kLineEndingsUnknown:
                case kLineEndingsMixed:
                    VASSERT(false);    // invalid input value
                    break;

                case kLineEndingsUnix:
                case kLineEndingsMac:
                    mLineEndingsReadKind = kLineEndingsMixed;
                    break;

                case kLineEndingsDOS:
                    // no change
                    break;

                }
            break;

        case kLineEndingsMac:
            switch (lineEndingKind)
                {
                case kLineEndingsUnknown:
                case kLineEndingsMixed:
                    VASSERT(false);    // invalid input value
                    break;

                case kLineEndingsUnix:
                case kLineEndingsDOS:
                    mLineEndingsReadKind = kLineEndingsMixed;
                    break;

                case kLineEndingsMac:
                    // no change
                    break;

                }
            break;

        case kLineEndingsMixed:
            // Once we detect mixed input, it stays that way.
            break;
        }

    }

int VTextIOStream::getLineEndingsReadKindForWrite() const
    {
    int writeKind = kUseNativeLineEndings;

    switch (mLineEndingsReadKind)
        {
        case kLineEndingsUnix:
            writeKind = kUseUnixLineEndings;
            break;

        case kLineEndingsDOS:
            writeKind = kUseDOSLineEndings;
            break;

        case kLineEndingsMac:
            writeKind = kUseMacLineEndings;
            break;

        }
    
    return writeKind;
    }

void VTextIOStream::setLineEndingsKind(int kind)
    {
    switch (kind)
        {
        case kUseUnixLineEndings:
            mLineEndingChars[0] = 0x0A;
            mLineEndingChars[1] = 0x00; // will not be referenced nor used
            mLineEndingCharsLength = 1;
            break;
        case kUseDOSLineEndings:
            mLineEndingChars[0] = 0x0D;
            mLineEndingChars[1] = 0x0A;
            mLineEndingCharsLength = 2;
            break;
        case kUseMacLineEndings:
            mLineEndingChars[0] = 0x0D;
            mLineEndingChars[1] = 0x00; // will not be referenced nor used
            mLineEndingCharsLength = 1;
            break;
        case kUseNativeLineEndings:
            {
            const Vu8* lineEndingChars = vault::VgetNativeLineEnding(mLineEndingCharsLength);
            for (int i = 0; i < mLineEndingCharsLength; ++i)
                mLineEndingChars[i] = lineEndingChars[i];
            }
            break;
        case kUseSuppliedLineEndings:
            // Line endings will be supplied by caller and already written into the line data.
            mLineEndingChars[0] = 0x00; // will not be referenced nor used
            mLineEndingChars[1] = 0x00; // will not be referenced nor used
            mLineEndingCharsLength = 0;
            break;
        default:
            throw VStackTraceException(VSTRING_FORMAT("VTextIOStream::writeLine using invalid line ending mode %d.", kind));
            break;
        }

    mLineEndingsWriteKind = kind;
    }


