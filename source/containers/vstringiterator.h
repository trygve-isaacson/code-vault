/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.1
http://www.bombaydigital.com/
License: MIT. See LICENSE.md in the Vault top level directory.
*/

#ifndef vstringiterator_h
#define vstringiterator_h

/** @file */

#include "vtypes.h"

#include "vcodepoint.h"

class VString;

// External linkage so that this header isn't dependent upon vexception.h.
extern void VStringIteratorThrowOutOfBoundsBegin();
extern void VStringIteratorThrowOutOfBoundsEnd();

/**
VStringIterator iterates through a VString over its UTF-8 code points.
That is, as you iterate, the (*()) operator returns the VCodePoint at the current iterator position.
Normal forward iteration is done by calling VString::begin() to get the start, and testing inquality with VString::end().
You simply increment/decrement the iterator to move forwards and backwards.
Reverse iteration is done by calling VString::rbegin() to get the "start" (really the last code point) and testing
inquality with VString::rend(). Increment a reverse iterator goes backwards in the string.
You can also use +/- and +=/-= operators to increment and decrement, in addition to the more common ++/--.
If you go out of bounds with incrementing/decrementing/addition/subtraction, a VRangeException will be thrown.

VStringIterator is the proper way to grab some random-indexed Unicode value out of a string.
In contrast, if you just write this:
    char c = s[5];
then you are just getting the 5th byte as a C char, which is fine and works for many things like search or
search-and-replace, but is not Unicode-aware in terms of say, "get me the 5th character of the string".
Instead, if you need to be Unicode-aware in your string manipulation, you would write:
    VCodePoint cp = *(s.begin() + 5);

(TODO: Maybe I should rewrite the VString operators to do this internally, and rename the byte-oriented accessors.)

Internal implementation notes:
Think of the string as an array of n code points. (In reality they are of different sizes.)
We of course keep an index into the string's array of bytes, but always incrementing and decrementing across actual
code point boundaries.
- For forward iteration, "begin" has the byte index of code point index 0, and "end" has the byte index of code point index n.
    Operator *() simply examines code point [i], which involves scanning the current index byte and possibly what follows.
- For reverse iteration, "begin" has the byte index of code point index n, and "end" has the byte index of code point index 0.
    Operator *() examines code point [i-1], which involves examining the bytes prior to the current byte index to find the start of the previous code point.
Fortunately, both of these operations are pretty efficient because UTF-8 encoding is fast to scan forward and backwards.
*/
template<typename vstring_ref> // vstring_ref may be 'const VString&' or 'VString&'
class VStringIterator {

    public:
        VStringIterator(const VStringIterator& iter)
            : mSource(iter.mSource)
            , mIsForwardIterator(iter.mIsForwardIterator)
            , mCurrentCodePointOffset(iter.mCurrentCodePointOffset)
            , mSourceLength(iter.mSourceLength)
            {
        }

        VStringIterator(vstring_ref source, bool isForwardIterator, bool goToEnd=false)
            : mSource(source)
            , mIsForwardIterator(isForwardIterator)
            , mCurrentCodePointOffset(0)
            , mSourceLength(source.length())
            {
            // If going to end on forward iterator, or not going to end on reverse iterator, then seek to end of our data.
            if (goToEnd == isForwardIterator) {
                this->_seekToEnd();
            }
        }

        ~VStringIterator() {}

        VStringIterator& operator=(const VStringIterator& iter) {
            if (this != &iter) {
                mSource = iter.mSource;
                mIsForwardIterator = iter.mIsForwardIterator;
                mCurrentCodePointOffset = iter.mCurrentCodePointOffset;
                mSourceLength = iter.mSourceLength;
            }
            
            return *this;
        }
        
        VCodePoint operator*() const {
            if (mIsForwardIterator) {
                return VCodePoint(mSource.getDataBufferConst(), mCurrentCodePointOffset);
            } else {
                int previousCodePointOffset = VCodePoint::getPreviousUTF8CodePointOffset(mSource.getDataBufferConst(), mCurrentCodePointOffset);
                return VCodePoint(mSource.getDataBufferConst(), previousCodePointOffset);
            }
        }

        VStringIterator operator+(int n) const {
            VStringIterator i(*this);
            i += n;
            return i;
        }
        
        VStringIterator operator-(int n) const {
            VStringIterator i(*this);
            i -= n;
            return i;
        }
        
        VStringIterator& operator+=(int n) {
            this->_increment(n);
            return *this;
        }

        VStringIterator& operator-=(int n) {
            this->_decrement(n);
            return *this;
        }

        VStringIterator& operator++() {
            this->_increment(1);
            return *this;
        }

        VStringIterator& operator--() {
            this->_decrement(1);
            return *this;
        }
        
        int getCurrentOffset() const {
            return mCurrentCodePointOffset;
        }
        
        friend inline bool operator==(const VStringIterator<vstring_ref>& i1, const VStringIterator<vstring_ref>& i2) {
            return (i1.mSource == i2.mSource) && (i1.mCurrentCodePointOffset == i2.mCurrentCodePointOffset);
        }

        friend inline bool operator!=(const VStringIterator<vstring_ref>& i1, const VStringIterator<vstring_ref>& i2) {
            return !operator==(i1, i2);
        }
    
    private:
    
        void _seekToEnd() {
            mCurrentCodePointOffset = mSourceLength;
        }

        void _increment(int n) {
            if (mIsForwardIterator) {
                this->_moveOffsetForwardInBuffer(n);
            } else {
                this->_moveOffsetBackwardInBuffer(n);
            }
        }

        void _decrement(int n) {
            if (mIsForwardIterator) {
                this->_moveOffsetBackwardInBuffer(n);
            } else {
                this->_moveOffsetForwardInBuffer(n);
            }
        }
    
        void _moveOffsetForwardInBuffer(int n) {
            // If we are at offset -1 (typical for "end" of reverse iterator), there is no valid buffer to examine at [-1],
            // so we just move forward to the first byte, at [0]. After that, normal examine-code-point-and-see-how-big-it-is
            // incrementing works.
            if ((mCurrentCodePointOffset == -1) && (n > 0)) {
                mCurrentCodePointOffset = 0;
                --n;
            }

            for (int i = 0; i < n; ++i) {
                if (mCurrentCodePointOffset == mSourceLength) {
                    VStringIteratorThrowOutOfBoundsEnd();
                }

                VCodePoint cp(mSource.getDataBufferConst(), mCurrentCodePointOffset);
                mCurrentCodePointOffset += cp.getUTF8Length();
            }
        }
        
        void _moveOffsetBackwardInBuffer(int n) {
            // If we are at [0], we just move backwards to [-1], and we never move beyond that. (Iterating off the end of
            // a container is undefined behavior. At least)
            const Vu8* bufferPtr = mSource.getDataBufferConst() + mCurrentCodePointOffset;
            for (int i = 0; i < n; ++i) {
            
                do {
                    if (mCurrentCodePointOffset == 0) {
                        VStringIteratorThrowOutOfBoundsBegin();
                    }

                    --bufferPtr;
                    --mCurrentCodePointOffset;
                } while (VCodePoint::isUTF8ContinuationByte(*bufferPtr)); 

            }
        }
    
        vstring_ref mSource;                    ///< The string we are iterating over. May be a const& or non-const&.
        bool        mIsForwardIterator;         ///< Set true for forward iteration, false for reverse iteration.
        int         mCurrentCodePointOffset;    ///< The 0-based offset into the string's byte array where the next character is.
        int         mSourceLength;              ///< The VString::length() of mSource, meaning our iterator end() value.
};

#endif /* vstringiterator_h */
