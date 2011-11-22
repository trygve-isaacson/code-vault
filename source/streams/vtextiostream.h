/*
Copyright c1997-2011 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.3
http://www.bombaydigital.com/
*/

#ifndef vtextiostream_h
#define vtextiostream_h

/** @file */

#include "viostream.h"
#include "vstring.h"

/**
    @ingroup viostream_derived
*/

/**
VTextIOStream is a concrete VIOStream subclass that provides text
line-oriented stream i/o.

You can specify a line ending mode when writing, or request to use
the native mode for the platform the code is running on.

You can find out what the line ending mode is when reading, in case
you need to tell the user (imagine implementing a line ending selection
the way the CodeWarrior IDE does).
*/
class VTextIOStream : public VIOStream
    {
    public:
    
        /** Values for mLineEndingsReadKind, set as we read the stream and figure out what its format is. */
        enum {
            kLineEndingsUnknown,    ///< Indicates that we have not yet read a line ending.
            kLineEndingsUnix,        ///< Indicates that we have only seen Unix line endings (0x0A).
            kLineEndingsDOS,        ///< Indicates that we have only seen DOS line endings (0x0D0A).
            kLineEndingsMac,        ///< Indicates that we have only seen Mac line endings (0x0D).
            kLineEndingsMixed,        ///< Indicates that we have seen a mixture of line ending types.

            kNumLineEndingsReadKinds
            };

        /** Values for mLineEndingsWriteKind, which the caller tells us to use as we write to the stream. */
        enum {
            kUseUnixLineEndings,        ///< Indicates that we are writing Unix line endings (0x0A).
            kUseDOSLineEndings,            ///< Indicates that we are writing DOS line endings (0x0D0A).
            kUseMacLineEndings,            ///< Indicates that we are writing Mac line endings (0x0D).
            kUseSuppliedLineEndings,    ///< Indicates that the caller is giving us the line endings, so we don't actually write them.
            kUseNativeLineEndings,        ///< Indicates that we are writing line endings native to the OS we are compiled for.
            
            kNumLineEndingsWriteKinds
            };

        /**
        Constructs the object with an underlying raw stream and the kind of
        line endings to use during write.
        @param    rawStream                the raw stream on which I/O will be performed
        @param    lineEndingsWriteKind    the kind of line endings to write
        */
        VTextIOStream(VStream& rawStream, int lineEndingsWriteKind=kUseNativeLineEndings);
        /**
        Destructor.
        */
        virtual ~VTextIOStream() {}
        
        /**
        Reads the next line of text from the stream. Throws a VException if
        EOF is encountered. Sets the mLineEndingsReadKind property depending
        on the line ending characters encountered.

        @param    s                    a VString to format
        @param    includeLineEnding    true if you want the line ending character(s)
                                    to be included in the string that is returned
        */
        void readLine(VString& s, bool includeLineEnding=false);
        
        /**
        Reads the next character from the stream, even if that character is part
        of a line ending. Throws a VException if EOF is encountered. You probably
        don't want to mix use of readLine and readCharacter in the same input
        stream, unless you are prepared to deal with line ending characters
        being read by readCharacter, and the result that calls to readLine will
        not see those characters and be able to set the mLineEndingsReadKind
        property except for the line endings that it does see.

        @return    the next character
        */
        VChar readCharacter();
        
        /**
        Primarily useful for reading from an underlying file stream, reads until
        eof is encountered, and returns the entire stream as a single string, by
        reading every line and appending it to the supplied string.
        Note that the default value for includeLineEndings is the opposite
        of VTextIOStream::readLine(), because here you probably want the whole
        file with lines separated, whereas when you read one line you probably
        don't want the end of line characters.
        @param    s                    a VString to append to
        @param    includeLineEndings   true if you want the line ending character(s)
                                    to be included in the string that is returned
        */
        void readAll(VString& s, bool includeLineEndings=true);
        /**
        This convenience function is like the other readAll, but it returns the
        stream's contents as a vector of strings rather than a single giant string.
        The lines do not have line ending characters at the end.
        @param  lines   a vector of strings; lines are appended to this vector
        */
        void readAll(VStringVector& lines);
        
        /**
        This method is equivalent to writeString(s) + writeLineEnd().
        Writes a line of text to the stream, with line ending character(s).
        If the mLineEndingsWriteKind property is kUseSuppliedLineEndings, then
        it assumed that you are supplying a string that has the line ending
        character(s) in it, and the method will not write them itself;
        otherwise, it writes them.
        @param    s    the line of text to write
        */
        void writeLine(const VString& s);
        
        /**
        Writes a string of text to the stream, WITHOUT line ending character(s).
        @param    s    the string of text to write
        */
        void writeString(const VString& s);

        /**
        Writes just the line ending character(s). Does nothing if the
        mLineEndingsWriteKind property is kUseSuppliedLineEndings, because
        that means you supply the line endings in the strings you write.
        */
        void writeLineEnd();
        
        /**
        Returns the mLineEndingsReadKind property, describing the kind of
        line endings that the file has encountered while reading from the
        stream.
        @return    one of the mLineEndingsReadKind enum values 
        */
        int getLineEndingsReadKind() const { return mLineEndingsReadKind; }
        /**
        Returns the mLineEndingsReadKind property, converted to a value
        suitable for supplying as the mLineEndingsWriteKind value for
        an output stream.
        @return    one of the mLineEndingsReadKind enum values 
        */
        int getLineEndingsReadKindForWrite() const;
        /**
        Returns the mLineEndingsWriteKind property, describing the kind of
        line endings that this object has been set up to write to the stream.
        @return    one of the mLineEndingsReadKind enum values 
        */
        int getLineEndingsWriteKind() const { return mLineEndingsWriteKind; }
        /**
        Sets the mLineEndingsWriteKind property, which determines what the
        writeLine method behavior is with regard to writing the line ending
        character(s) at the end of each line written to the stream.
        @param    kind    one of the mLineEndingsWriteKind enum values
        */
        void setLineEndingsKind(int kind);
    
    private:
    
        /** Updates the mLineEndingsReadKind based on the kind of line ending just detected. */
        void _updateLineEndingsReadKind(int lineEndingKind);

        int     mLineEndingsReadKind;   ///< During read, the kind of line endings we think the file is using.
        int     mLineEndingsWriteKind;  ///< During write, the kind of line endings the caller wants us to use.
        char    mPendingCharacter;      ///< During read we may have a pending character while reading DOS line endings.
        int     mReadState;             ///< During read we have to maintain parsing state.
        VString mLineBuffer;            ///< Temporarily holds each line of the file as we read it.
        Vu8     mLineEndingChars[2];    ///< One or both bytes may be used, as indicated by mLineEndingCharsLength.
        int     mLineEndingCharsLength; ///< Describes how much of mLineEndingChars array should be written as line ending.

        enum {
            kReadStateReady,    ///< We are ready to read any character.
            kReadStateGot0x0D,  ///< We have just read a 0x0D, which could be a Mac line ending, or the first byte of a DOS line ending.

            kNumReadStates
            };

    };

#endif /* vtextiostream_h */
