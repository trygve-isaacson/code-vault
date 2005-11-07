/*
Copyright c1997-2005 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.3.2
http://www.bombaydigital.com/
*/

/** @file */

#include "vstring.h"

#include "vchar.h"
#include "vexception.h"

#ifndef V_EFFICIENT_SPRINTF
#include "vmutex.h"
#include "vmutexlocker.h"
#endif

V_STATIC_INIT_TRACE
    
//SN 10/8/04 : CR 22227 & CR 22266 - make sure we get value in acceptable boundry.
#define SAFE_LEN(val,max)((val > max) ? max : val)

// Define these here rather than hardcoding in case we ever need to change for a compiler or library impl.
//lint -e1923 "macro could become const variable -- Effective C++ #1"
#define kFormat_int "%d"

//lint -e1923 "macro could become const variable -- Effective C++ #1"
#define kFormat_Vu8 "%u"

//lint -e1923 "macro could become const variable -- Effective C++ #1"
#define kFormat_Vs8 "%d"

//lint -e1923 "macro could become const variable -- Effective C++ #1"
#define kFormat_Vu16 "%hu"

//lint -e1923 "macro could become const variable -- Effective C++ #1"
#define kFormat_Vs16 "%hd"

//lint -e1923 "macro could become const variable -- Effective C++ #1"
#define kFormat_Vu32 "%lu"

//lint -e1923 "macro could become const variable -- Effective C++ #1"
#define kFormat_Vs32 "%ld"

//lint -e1923 "macro could become const variable -- Effective C++ #1"
#define kFormat_Vu64 "%llu"

//lint -e1923 "macro could become const variable -- Effective C++ #1"
#define kFormat_Vs64 "%lld"

//lint -e1923 "macro could become const variable -- Effective C++ #1"
#define kFormat_VFloat "%f"

//lint -e1923 "macro could become const variable -- Effective C++ #1"
#define kFormat_VDouble "%Lf"

#undef strlen
#undef strcmp

/*
The "empty string" constant constructs to an empty string.
When you want to pass "" to a function that takes a "const VString&" parameter,
it's much more efficient to pass the kEmptyString because it avoids
constructing a temporary empty VString object on the fly.
*/
const VString VString::kEmptyString;

VString::VString()
    {
    this->initialize();
    
    ASSERT_INVARIANT();
    }

VString::VString(const VChar& c)
    {
    this->initialize();

    this->preflight(1);
    mBuffer[0] = c.charValue();
    this->setLength(1);

    ASSERT_INVARIANT();
    }

VString::VString(const VString& s)
    {
    this->initialize();

    int theLength = s.length();
    this->preflight(theLength);
    s.copyToBuffer(mBuffer);
    this->setLength(theLength);

    ASSERT_INVARIANT();
    }

VString::VString(const char c)
    {
    this->initialize();

    this->preflight(1);
    mBuffer[0] = c;
    this->setLength(1);

    ASSERT_INVARIANT();
    }

VString::VString(char* s)
    {
    this->initialize();
    
    if (s != NULL)    // if s is NULL, leave as initialized to empty
        {
        int    theLength = (int) ::strlen(s);
        this->preflight(theLength);
        ::memcpy(mBuffer, s, (VSizeType) mBufferLength);    // faster than strcpy?
        this->setLength(theLength);
        }

    ASSERT_INVARIANT();
    }

VString::VString(const char* formatText, ...)
    {
    this->initialize();

    va_list    args;
    va_start(args, formatText);

    this->vaFormat(formatText, args);

    va_end(args);

    ASSERT_INVARIANT();
    }

/* FIXME   VString::VString(int stringLength)
    {
    this->initialize();

    this->preflight(stringLength);
    this->setLength(0);

    ASSERT_INVARIANT();
    }
*/

#ifdef VAULT_QT_SUPPORT
VString::VString(const QString& s)
    {
    this->initialize();
    
    this->copyFromBuffer(s.ascii(), 0, s.length());

    ASSERT_INVARIANT();
    }
#endif

VString::~VString()
    {
    delete [] mBuffer;
    }

VString& VString::operator=(const VString& s)
    {
    ASSERT_INVARIANT();

    if (this != &s)
        {
        int theLength = s.length();
        
        if (theLength != 0)
            {
            this->preflight(theLength);
            s.copyToBuffer(mBuffer);
            }

        this->setLength(theLength);
        }
    
    ASSERT_INVARIANT();

    return *this;
    }

VString& VString::operator=(const VString* s)
    {
    ASSERT_INVARIANT();

    if (s == NULL)
        this->setLength(0);
    else if (this != s)
        {
        int theLength = s->length();
        
        if (theLength != 0)
            {
            this->preflight(theLength);
            s->copyToBuffer(mBuffer);
            }

        this->setLength(theLength);
        }
    
    ASSERT_INVARIANT();

    return *this;
    }

#ifdef VAULT_QT_SUPPORT
VString& VString::operator=(const QString& s)
    {
    ASSERT_INVARIANT();

    this->copyFromBuffer(s.ascii(), 0, s.length());

    ASSERT_INVARIANT();

    return *this;
    }
#endif

VString& VString::operator=(const VChar& c)
    {
    ASSERT_INVARIANT();

    this->preflight(1);
    mBuffer[0] = c.charValue();
    this->setLength(1);
    
    ASSERT_INVARIANT();

    return *this;
    }

VString& VString::operator=(char c)
    {
    ASSERT_INVARIANT();

    this->preflight(1);
    mBuffer[0] = c;
    this->setLength(1);
    
    ASSERT_INVARIANT();

    return *this;
    }

VString& VString::operator=(const char* s)
    {
    ASSERT_INVARIANT();

    if (s == NULL)
        this->setLength(0);
    else
        {
        int    theLength = static_cast<int> (::strlen(s));

        if (theLength != 0)
            {
            this->preflight(theLength);
            //lint -e668 "Possibly passing a null pointer to function"
            ::memcpy(mBuffer, s, static_cast<VSizeType> (theLength));    // faster than strcpy?
            }

        this->setLength(theLength);
        }
    
    ASSERT_INVARIANT();

    return *this;
    }

VString& VString::operator=(int i)
    {
    ASSERT_INVARIANT();
    
    this->format(kFormat_int, i);
    
    ASSERT_INVARIANT();

    return *this;
    }

VString& VString::operator=(Vu8 i)
    {
    ASSERT_INVARIANT();
    
    this->format(kFormat_Vu8, (int) i);
    
    ASSERT_INVARIANT();

    return *this;
    }

VString& VString::operator=(Vs8 i)
    {
    ASSERT_INVARIANT();
    
    this->format(kFormat_Vs8, (int) i);
    
    ASSERT_INVARIANT();

    return *this;
    }

VString& VString::operator=(Vu16 i)
    {
    ASSERT_INVARIANT();
    
    this->format(kFormat_Vu16, i);
    
    ASSERT_INVARIANT();

    return *this;
    }

VString& VString::operator=(Vs16 i)
    {
    ASSERT_INVARIANT();
    
    this->format(kFormat_Vs16, i);
    
    ASSERT_INVARIANT();

    return *this;
    }

VString& VString::operator=(Vu32 i)
    {
    ASSERT_INVARIANT();
    
    this->format(kFormat_Vu32, i);
    
    ASSERT_INVARIANT();

    return *this;
    }

VString& VString::operator=(Vs32 i)
    {
    ASSERT_INVARIANT();
    
    this->format(kFormat_Vs32, i);
    
    ASSERT_INVARIANT();

    return *this;
    }

VString& VString::operator=(Vu64 i)
    {
    ASSERT_INVARIANT();
    
    this->format(kFormat_Vu64, i);
    
    ASSERT_INVARIANT();

    return *this;
    }

VString& VString::operator=(Vs64 i)
    {
    ASSERT_INVARIANT();
    
    this->format(kFormat_Vs64, i);
    
    ASSERT_INVARIANT();

    return *this;
    }

VString& VString::operator=(VDouble f)
    {
    ASSERT_INVARIANT();
    
    this->format(kFormat_VDouble, f);
    
    ASSERT_INVARIANT();

    return *this;
    }

VString VString::operator+(const char c) const
    {
    VString newString("%s%c", this->chars(), c);
    return newString;
    }

VString VString::operator+(const char* s) const
    {
    VString newString("%s%s", this->chars(), s);
    return newString;
    }

VString VString::operator+(const VString& s) const
    {
    VString newString("%s%s", this->chars(), s.chars());
    return newString;
    }

VString& VString::operator+=(const VChar& c)
    {
    ASSERT_INVARIANT();

    this->preflight(1 + mStringLength);

    mBuffer[mStringLength] = c.charValue();
    this->setLength(1 + mStringLength);
    
    ASSERT_INVARIANT();

    return *this;
    }

VString& VString::operator+=(const VString& s)
    {
    ASSERT_INVARIANT();

    // We have to be careful copying the buffer if &s==this
    // because things morph under us as we work in that case.
    
    int    theLength = s.length();

    this->preflight(theLength + mStringLength);

    //lint -e668 "Possibly passing a null pointer to function"
    ::memcpy(&(mBuffer[mStringLength]), s.chars(), static_cast<VSizeType> (theLength));
    
    this->setLength(theLength + mStringLength);
    
    ASSERT_INVARIANT();

    return *this;
    }

VString& VString::operator+=(char c)
    {
    ASSERT_INVARIANT();

    this->preflight(1 + mStringLength);

    mBuffer[mStringLength] = c;
    this->setLength(1 + mStringLength);

    ASSERT_INVARIANT();

    return *this;
    }

VString& VString::operator+=(const char* s)
    {
    ASSERT_INVARIANT();

    int    theLength = (int) ::strlen(s);

    this->preflight(theLength + mStringLength);
    //lint -e668 "Possibly passing a null pointer to function"
    ::memcpy(&(mBuffer[mStringLength]), s, static_cast<VSizeType> (theLength));
    this->setLength(theLength + mStringLength);
    
    ASSERT_INVARIANT();

    return *this;
    }

VString& VString::operator+=(int i)
    {
    ASSERT_INVARIANT();
    
    VString    s(kFormat_int, i);
    *this += s;
    
    ASSERT_INVARIANT();

    return *this;
    }

VString& VString::operator+=(Vu8 i)
    {
    ASSERT_INVARIANT();
    
    VString    s(kFormat_Vu8, (int) i);
    *this += s;
    
    ASSERT_INVARIANT();

    return *this;
    }

VString& VString::operator+=(Vs8 i)
    {
    ASSERT_INVARIANT();
    
    VString    s(kFormat_Vs8, (int) i);
    *this += s;
    
    ASSERT_INVARIANT();

    return *this;
    }

VString& VString::operator+=(Vu16 i)
    {
    ASSERT_INVARIANT();
    
    VString    s(kFormat_Vu16, i);
    *this += s;
    
    ASSERT_INVARIANT();

    return *this;
    }

VString& VString::operator+=(Vs16 i)
    {
    ASSERT_INVARIANT();
    
    VString    s(kFormat_Vs16, i);
    *this += s;
    
    ASSERT_INVARIANT();

    return *this;
    }

VString& VString::operator+=(Vu32 i)
    {
    ASSERT_INVARIANT();
    
    VString    s(kFormat_Vu32, i);
    *this += s;
    
    ASSERT_INVARIANT();

    return *this;
    }

VString& VString::operator+=(Vs32 i)
    {
    ASSERT_INVARIANT();
    
    VString    s(kFormat_Vs32, i);
    *this += s;
    
    ASSERT_INVARIANT();

    return *this;
    }

VString& VString::operator+=(Vu64 i)
    {
    ASSERT_INVARIANT();
    
    VString    s(kFormat_Vu64, i);
    *this += s;
    
    ASSERT_INVARIANT();

    return *this;
    }

VString& VString::operator+=(Vs64 i)
    {
    ASSERT_INVARIANT();
    
    VString    s(kFormat_Vs64, i);
    *this += s;
    
    ASSERT_INVARIANT();

    return *this;
    }

VString& VString::operator+=(VDouble f)
    {
    ASSERT_INVARIANT();
    
    VString    s(kFormat_VDouble, f);
    *this += s;
    
    ASSERT_INVARIANT();

    return *this;
    }

void VString::readFromIStream(std::istream& in)
    {
    ASSERT_INVARIANT();

    *this = VString::kEmptyString;
    
    this->appendFromIStream(in);
    
    ASSERT_INVARIANT();
    }

void VString::appendFromIStream(std::istream& in)
    {
    ASSERT_INVARIANT();

    char    c;
    
    in >> c;
    
    while (c != 0)
        {
        // Trying here to avoid reallocating buffer on every incremental character. Do it every 10 instead.
        int    currentLength = this->length();
        int    roundedUpLength = (currentLength + 11) - (currentLength % 10);

        this->preflight(roundedUpLength);
        
        *this += c;
        
        in >> c;
        }
    
    ASSERT_INVARIANT();
    }

void VString::format(const char* formatText, ...)
    {
    ASSERT_INVARIANT();

     va_list    args;
    va_start(args, formatText);

    this->vaFormat(formatText, args);

    va_end(args);

    ASSERT_INVARIANT();
    }

void VString::insert(char c, int offset)
    {
    ASSERT_INVARIANT();
    
    // We could make a VString of c, and then insert it, but it seems
    // much more efficient to move a single char by itself.

    int    addedLength = 1;
    int    oldLength = this->length();
    int    newLength = oldLength + addedLength;

    // constrain to guard against bad offset; perhaps an out-of-bounds exception would be better?
    int    actualOffset = V_MIN(oldLength, V_MAX(0, offset));
    int    numBytesToMove = oldLength - actualOffset;

    this->preflight(newLength);

    // Need to use memmove here because memcpy behavior is undefined if ranges overlap.
    ::memmove(&(mBuffer[actualOffset+addedLength]), &(mBuffer[actualOffset]), numBytesToMove);    // shift forward by 1 byte, everything past the offset
    mBuffer[actualOffset] = c;
    
    this->postflight(newLength);

    ASSERT_INVARIANT();
    }

void VString::insert(const VString& s, int offset)
    {
    ASSERT_INVARIANT();
    
    if (s.length() == 0) // optimize the nothing-to-do case
        return;
    
    // If s happens to be 'this', we'll need a temporary copy of it.
    // Otherwise, our memmove() + memcpy() data would be messed up.
    VString    tempCopy;    // note that by just declaring this, we merely have a few bytes on the stack -- nothing on the heap yet
    char*    source = s.chars();
    
    if (this == &s)
        {
        tempCopy = s;    // this will cause tempCopy to allocate a buffer to hold a copy of s (which is 'this')
        source = tempCopy.chars();    // we'll do our final memcpy() from the copy, not from the 'this' buffer that's split by the memmove()
        }
    
    int    addedLength = s.length();
    int    oldLength = this->length();
    int    newLength = oldLength + addedLength;

    // constrain to guard against bad offset; perhaps an out-of-bounds exception would be better?
    int    actualOffset = V_MIN(oldLength, V_MAX(0, offset));
    int    numBytesToMove = oldLength - actualOffset;

    this->preflight(newLength);

    // Need to use memmove here because memcpy behavior is undefined if ranges overlap.
    ::memmove(&(mBuffer[actualOffset+addedLength]), &(mBuffer[actualOffset]), numBytesToMove);    // shift forward by s.length() byte, everything past the offset
    ::memcpy(&(mBuffer[actualOffset]), source, addedLength);
    
    this->postflight(newLength);

    ASSERT_INVARIANT();
    }

int VString::length() const
    {
    ASSERT_INVARIANT();

    return mStringLength;
    }

void VString::truncateLength(int maxLength)
    {
    ASSERT_INVARIANT();

    if ((maxLength >= 0) && (this->length() > maxLength))
        this->setLength(maxLength);

    ASSERT_INVARIANT();
    }

bool VString::isEmpty() const
    {
    ASSERT_INVARIANT();

    return mStringLength == 0;
    }

bool VString::isNotEmpty() const
    {
    ASSERT_INVARIANT();

    return mStringLength != 0;
    }

VChar VString::at(int i) const
    {
    ASSERT_INVARIANT();

    if (i >= mStringLength)
        throw VException("VString::at index out of range.");

    return VChar(mBuffer[i]);
    }

VChar VString::operator[](int i) const
    {
    ASSERT_INVARIANT();

    if (i >= mStringLength)
        throw VException("VString::operator[] index out of range.");

    return VChar(mBuffer[i]);
    }

char& VString::operator[](int i)
    {
    ASSERT_INVARIANT();

    if (i >= mStringLength)
        throw VException("VString::operator[] index out of range.");

    return mBuffer[i];
    }

char VString::charAt(int i) const
    {
    ASSERT_INVARIANT();

    if (i >= mStringLength)
        throw VException("VString::charAt index out of range.");

    return mBuffer[i];
    }

VString::operator char*() const
    {
    ASSERT_INVARIANT();

    if (mBuffer == NULL)
        return "";
    else
        return mBuffer;
    }
        
char* VString::chars() const
    {
    ASSERT_INVARIANT();

    if (mBuffer == NULL)
        return "";
    else
        return mBuffer;
    }

#ifdef VAULT_QT_SUPPORT
QString VString::qstring() const
    {
    ASSERT_INVARIANT();

    return QString(this->chars());
    }
#endif

bool VString::equalsIgnoreCase(const VString& s) const
    {
    ASSERT_INVARIANT();

    return this->equalsIgnoreCase(s.chars());
    }

bool VString::equalsIgnoreCase(const char* s) const
    {
    ASSERT_INVARIANT();

    return this->compareIgnoreCase(s) == 0;
    }

int VString::compare(const VString& s) const
    {
    ASSERT_INVARIANT();

    return this->compare(s.chars());
    }

int VString::compare(const char* s) const
    {
    ASSERT_INVARIANT();
    
    if (mBuffer == NULL)
        return ::strcmp("", s);
    else
        return ::strcmp(mBuffer, s);
    }

int VString::compareIgnoreCase(const VString& s) const
    {
    ASSERT_INVARIANT();

    return this->compareIgnoreCase(s.chars());
    }

int VString::compareIgnoreCase(const char* s) const
    {
    ASSERT_INVARIANT();
    
    /*
    FIXME:
    Need to figure out "correct" way to reference strcasecmp in the
    various compilers (which includes, etc.). For now, do it the slow way.
    fast way: return ::strcasecmp(mBuffer, s);
    */
    VString    thisString((mBuffer == NULL) ? "" : mBuffer);
    VString    otherString(s);
    
    thisString.toLowerCase();
    otherString.toLowerCase();
    
    return ::strcmp(thisString, otherString);
    }

bool VString::startsWith(const VString& s) const
    {
    ASSERT_INVARIANT();

    return this->regionMatches(0, s, 0, s.length());
    }
        
bool VString::startsWith(char aChar) const
    {
    ASSERT_INVARIANT();

    if (mStringLength == 0)
        return false;
    else if (mBuffer == NULL)
        return false;
    else
        return (mBuffer[0] == aChar);
    }
        
bool VString::endsWith(const VString& s) const
    {
    ASSERT_INVARIANT();
    
    return this->regionMatches(mStringLength - s.length(), s, 0, s.length());
    }
        
bool VString::endsWith(char aChar) const
    {
    ASSERT_INVARIANT();

    if (mStringLength == 0)
        return false;
    else if (mBuffer == NULL)
        return false;
    else
        return (mBuffer[mStringLength - 1] == aChar);
    }
        
int VString::indexOf(char c, int fromIndex) const
    {
    ASSERT_INVARIANT();
    
    if (mBuffer != NULL)
        for (int i = fromIndex; i < mStringLength; ++i)
            {
            if (mBuffer[i] == c)
                return i;
            }
        
    return -1;
    }

int VString::indexOf(const VString& s, int fromIndex) const
    {
    ASSERT_INVARIANT();
    
    int    otherLength = s.length();
    
    for (int i = fromIndex; i < mStringLength; ++i)
        {
        if (this->regionMatches(i, s, 0, otherLength))
            return i;
        }
        
    return -1;
    }

int VString::lastIndexOf(char c, int fromIndex) const
    {
    ASSERT_INVARIANT();
    
    if (fromIndex == -1)
        fromIndex = mStringLength - 1;
    
    if (mBuffer != NULL)
        for (int i = fromIndex; i >= 0; --i)
            {
            if (mBuffer[i] == c)
                return i;
            }
        
    return -1;
    }

int VString::lastIndexOf(const VString& s, int fromIndex) const
    {
    ASSERT_INVARIANT();
    
    int    otherLength = s.length();
    
    if (fromIndex == -1)
        fromIndex = mStringLength;
    
    for (int i = fromIndex; i >= 0; --i)
        {
        if (this->regionMatches(i, s, 0, otherLength))
            return i;
        }
        
    return -1;
    }

bool VString::regionMatches(int thisOffset, const VString& otherString, int otherOffset, int regionLength) const
    {
    ASSERT_INVARIANT();

    int    result;
    int    otherStringLength = otherString.length();
    
    // Buffer offset safety checks first. If they fail, return false.
    if ((thisOffset < 0) ||
        (thisOffset >= mStringLength) ||
        (thisOffset + regionLength > mStringLength) ||
        (otherOffset < 0) ||
        (otherOffset >= otherStringLength) ||
        (otherOffset + regionLength > otherStringLength))
        return false;
    
    result = ::strncmp(this->chars() + thisOffset, otherString.chars() + otherOffset, static_cast<VSizeType> (regionLength));

    return (result == 0);
    }

int VString::replace(const VString& searchString, const VString& replacementString)
    {
    ASSERT_INVARIANT();
    
    int    searchLength = searchString.length();
    
    if (searchLength == 0)
        return 0;

    int    replacementLength = replacementString.length();
    int    numReplacements = 0;
    int    currentOffset = this->indexOf(searchString);
    
    while ((currentOffset != -1) && (mBuffer != NULL))
        {
        /*
        The optimization trick here is that we can put a zero byte to artificially
        terminate the C string buffer at the found index, creating the BEFORE part
        as a C string. The AFTER part remains intact. And we've been supplied the
        MIDDLE part. We simply format these 3 pieces together to form the new
        string, and then reassign it back to ourself. This relieves us of having
        to create multiple temporary buffers; we only create 1 temporary buffer as
        a result of using a VString to format into.
        
        We could further optimize by
        combining ourself with 3 memcpy calls instead of letting vsnprintf calculate
        the buffer length:
        char* buffer = new char[this->length() + replacementLength - searchLength + 1];
        1. memcpy from mBuffer[0] to buffer[0] length=currentOffset
        2. memcpy from mBuffer[currentOffset + searchLength] to buffer[currentOffset + replacementLength] length=(this->length() - currentOffset + searchLength)
        3. memcpy from replacementString.mBuffer to buffer[currentOffset] length=replacementLength
        4. null terminate the buffer: buffer[length of buffer] = 0
        (order of 1, 2, 3 is important for correct copying w/o incorrect overwriting)
        (something like that off the top of my head)
        
        And, if replacementLength <= searchLength we could just update in place.
        */

        mBuffer[currentOffset] = 0;    // terminate the C string buffer to create the BEFORE part
        char*    beforePart = mBuffer;
        char*    afterPart = &mBuffer[currentOffset + searchLength];
        VString    alteredString("%s%s%s", beforePart, replacementString.chars(), afterPart);
        
        // Assign the new string to ourself -- copies its buffer into ours correctly.
        // (Could be optimized to just swap buffers if we defined a new friend function or two.)
        *this = alteredString;
        
        // Finally we have to move currentOffset forward past the replacement part.
        currentOffset += replacementLength;
        
        ++numReplacements;
        
        // See if there is another occurrence to replace.
        currentOffset = this->indexOf(searchString, currentOffset);
        }

    ASSERT_INVARIANT();
    
    return numReplacements;
    }

int VString::replace(const VChar& searchChar, const VChar& replacementChar)
    {
    ASSERT_INVARIANT();
    
    int        numReplacements = 0;
    char    match = searchChar.charValue();
    char    replacement = replacementChar.charValue();

    if (mBuffer != NULL)
        for (int i = 0; i < mStringLength; ++i)
            {
            if (mBuffer[i] == match)
                {
                mBuffer[i] = replacement;
                ++numReplacements;
                }
            }

    ASSERT_INVARIANT();
    
    return numReplacements;
    }

void VString::toLowerCase()
    {
    ASSERT_INVARIANT();

    if (mBuffer != NULL)
        for (int i = 0; i < mStringLength; ++i)
            mBuffer[i] = static_cast<char> (::tolower(mBuffer[i]));

    ASSERT_INVARIANT();
    }

void VString::toUpperCase()
    {
    ASSERT_INVARIANT();

    if (mBuffer != NULL)
        for (int i = 0; i < mStringLength; ++i)
            mBuffer[i] = static_cast<char> (::toupper(mBuffer[i]));

    ASSERT_INVARIANT();
    }

void VString::set(int i, const VChar& c)
    {
    ASSERT_INVARIANT();

    if (i >= mStringLength)
        throw VException("VString::set() index out of range.");

    if (mBuffer != NULL)
        mBuffer[i] = c.charValue();

    ASSERT_INVARIANT();
    }

void VString::getSubstring(VString& toString, int startIndex, int endIndex) const
    {
    ASSERT_INVARIANT();

    int    theLength = this->length();

    startIndex = V_MAX(0, startIndex);        // prevent negative start index
    startIndex = V_MIN(theLength, startIndex);    // prevent start past end
    
    if (endIndex == -1)    // -1 means to end of string
        endIndex = theLength;

    endIndex = V_MIN(theLength, endIndex);        // prevent stop past end
    endIndex = V_MAX(startIndex, endIndex);    // prevent stop before start
    
    if (mBuffer != NULL)
        toString.copyFromBuffer(mBuffer, startIndex, endIndex);
    }

void VString::substringInPlace(int startIndex, int endIndex)
    {
    ASSERT_INVARIANT();

    int    theLength = this->length();

    startIndex = V_MAX(0, startIndex);        // prevent negative start index
    startIndex = V_MIN(theLength, startIndex);    // prevent start past end
    
    if (endIndex == -1)    // -1 means to end of string
        endIndex = theLength;

    endIndex = V_MIN(theLength, endIndex);        // prevent stop past end
    endIndex = V_MAX(startIndex, endIndex);    // prevent stop before start
    
    // Only do something if the start/stop are not the whole string.
    int    newLength = endIndex - startIndex;
    if (newLength != theLength)
        {
        ::memmove(&(mBuffer[0]), &(mBuffer[startIndex]), static_cast<VSizeType> (newLength));
        this->setLength(newLength);
        }
    
    ASSERT_INVARIANT();
    }

void VString::trim()
    {
    ASSERT_INVARIANT();
    
    int    theLength = mStringLength;

    // optimize for empty string condition
    if (theLength == 0)
        return;

    int    indexOfFirstNonWhitespace = -1;
    int    indexOfLastNonWhitespace = -1;
    
    if (mBuffer != NULL)
        {
        for (int i = 0; i < theLength; ++i)
            {
            if ((mBuffer[i] > 0x20) && (mBuffer[i] != 0x7F))
                {
                indexOfFirstNonWhitespace = i;
                break;
                }
            }
    
        if (indexOfFirstNonWhitespace != -1)
            {
            for (int i = theLength-1; i >= 0; --i)
                {
                if ((mBuffer[i] > 0x20) && (mBuffer[i] != 0x7F))
                    {
                    indexOfLastNonWhitespace = i;
                    break;
                    }
                }
            }
        }
    
    /*
    Case 1: all whitespace - set length to zero
    Case 2: no leading/trailing whitespace - nothing to do
    Case 3: some leanding and/or trailing whitespace - move data and change length

    Note: we assume at this point that the buffer exists and length>0 because of prior length check
    */
    if (indexOfFirstNonWhitespace == -1)
        {
        // all whitespace - set length to zero
        this->setLength(0);
        }
    else if ((indexOfFirstNonWhitespace == 0) && (indexOfLastNonWhitespace == theLength-1))
        {
        // no leading/trailing whitespace - nothing to do
        }
    else if (mBuffer != NULL)
        {
        // some leanding and/or trailing whitespace - move data and change length

        int    numBytesAfterTrimming = (indexOfLastNonWhitespace - indexOfFirstNonWhitespace) + 1;

        ::memmove(&(mBuffer[0]), &(mBuffer[indexOfFirstNonWhitespace]), static_cast<VSizeType> (numBytesAfterTrimming));
        
        this->setLength(numBytesAfterTrimming);
        }

    ASSERT_INVARIANT();
    }

void VString::copyToBuffer(char* toBuffer, int sizeofBuffer) const
    {
    ASSERT_INVARIANT();

    if ((toBuffer == NULL) || (sizeofBuffer <= 0))
        return;
    else if ((mBuffer == NULL) || (mBufferLength == 0) || (mStringLength == 0))
        {
        toBuffer[0] = 0;
        }
    else if (mStringLength > sizeofBuffer)
        {
        ::memcpy(toBuffer, mBuffer, static_cast<VSizeType> (sizeofBuffer - 1));
        toBuffer[sizeofBuffer-1] = '\0';
        }
    else
        {
        ::memcpy(toBuffer, mBuffer, static_cast<VSizeType> (1 + mStringLength));
        }
    }

void VString::copyFromBuffer(const char* fromBuffer, int startIndex, int endIndex)
    {
    ASSERT_INVARIANT();
    
    if (endIndex == LONG_MAX)
        endIndex = ::strlen(fromBuffer);
    if (endIndex == -1)
        endIndex = startIndex;    // make sure we copy an empty string
    
    this->preflight(endIndex - startIndex);
    //lint -e668 "Possibly passing a null pointer to function"
    ::memcpy(mBuffer, fromBuffer + startIndex, static_cast<VSizeType> (endIndex - startIndex));
    this->postflight(endIndex - startIndex);

    ASSERT_INVARIANT();
    }

void VString::copyToPascalString(char* pascalBuffer) const
    {
    ASSERT_INVARIANT();

    if ((mBuffer == NULL) || (mBufferLength == 0) || (mStringLength == 0))
        {
        pascalBuffer[0] = 0;
        }
    else
        {
        Vu8    constrainedLength = static_cast<Vu8> (V_MIN(255, mStringLength));
        
        pascalBuffer[0] = static_cast<char> (constrainedLength);
        ::memcpy(&(pascalBuffer[1]), mBuffer, constrainedLength);
        }
    }

void VString::copyFromPascalString(const char* pascalBuffer)
    {
    ASSERT_INVARIANT();
    
    int    theLength = static_cast<int> (static_cast<Vu8> (pascalBuffer[0]));

    this->preflight(theLength);
    //lint -e668 "Possibly passing a null pointer to function"
    ::memcpy(mBuffer, &(pascalBuffer[1]), static_cast<VSizeType> (theLength));
    this->postflight(theLength);

    ASSERT_INVARIANT();
    }

void VString::preflight(int stringLength)
    {
    ASSERT_INVARIANT();

    if (stringLength < 0)
        stringLength = 0;
    
    if (stringLength < LONG_MAX)
        if ((mBuffer == NULL) || (mBufferLength <= stringLength))    // buffer must be at least string length + 1
            {
            try
                {
                char*    newBuffer = new char[stringLength + 1L];
                
                if (mBufferLength == 0)
                    newBuffer[0] = 0;
                else
                {
                    //lint -e668 -e669 -e670 "Possibly passing a null pointer to function"
                    //::memcpy(newBuffer, mBuffer, static_cast<VSizeType> (mStringLength + 1));

                    //if (prev_len > curr_len) 
                    //    set new_len == curr_len
                    //if (prev_len <= curr_len)
                    //    set new_len == prev_len
                    //SN 10/8/04 : CR 22227 & CR 22266 - fix buffer
                    ::memcpy(newBuffer, mBuffer, static_cast<VSizeType> (SAFE_LEN(mStringLength,stringLength) + 1));
                }

                delete [] mBuffer;
                mBuffer = newBuffer;
                mBufferLength = stringLength + 1;
                }
            catch (...)
                {
                ASSERT_INVARIANT();
                throw VException("VString::preflight unable to allocate buffer.");
                }
            }

    ASSERT_INVARIANT();
    }

char* VString::buffer()
    {
    ASSERT_INVARIANT();

    return mBuffer;
    }

void VString::postflight(int stringLength)
    {
    /*
    Do not assert invariant on entry -- postflight's job is to clean up
    after a buffer copy that presumably has made the invariant state
    invalid.
    */

    this->setLength(stringLength);

    ASSERT_INVARIANT();
    }

void VString::vaFormat(const char* formatText, va_list args)
    {
    ASSERT_INVARIANT();
    
    if (formatText == NULL)
        {
        this->preflight(0);
        mStringLength = 0;
        //lint -e613 "Possible use of null pointer in left argument to operator '['"
        mBuffer[0] = 0;
        }
    else
        {
        int    newStringLength = VString::determineSprintfLength(formatText, args);
    
        this->preflight(newStringLength);
    
        (void) ::vsnprintf(mBuffer, static_cast<VSizeType> (mBufferLength), formatText, args);
    
        va_end(args);

        //SN 10/8/04 : CR 22227 & CR 22266 - fix buffer 
        mStringLength = SAFE_LEN(newStringLength,mBufferLength);
        }

    ASSERT_INVARIANT();
    }

void VString::initialize()
    {
    mStringLength = 0;
    mBufferLength = 0;
    mBuffer = NULL;
    }

//CAUTION : This function is also used to initialize buffer with 0.
void VString::setLength(int stringLength)
    {
    if (mBuffer != NULL)
        {
        //SN 10/8/04 : CR 22227 & CR 22266 - fix buffer
        mStringLength = SAFE_LEN(stringLength,mBufferLength);
        
        //lint -e661 -e662 "Possible access of out-of-bounds pointer (1 beyond end of data) by operator '['"
        if (mStringLength <= 0)
            mBuffer[0] = 0;
        else
            mBuffer[mStringLength] = 0;
        }
    }

void VString::assertInvariant() const
    {
    if (mBuffer != NULL)    // buffer pointer can be null, making other fields n/a
        {
        V_ASSERT(mStringLength >= 0);                // string length must be non-negative
        V_ASSERT(mBufferLength > mStringLength);    // buffer must be at least string length + 1
        V_ASSERT(mBuffer[mStringLength] == 0);    // null terminator must be in place
        }
    }

// static
int VString::determineSprintfLength(const char* formatText, va_list args)
    {
#ifdef V_EFFICIENT_SPRINTF
    /*
    It is TBD if this works on all platforms implementations of vsnprintf.
    It works on BSD, but the SUSV2 spec language implies different behavior.
    The goal here is simply to determine how large the vsnprintf'd string would
    be, without actually allocating a big ol' buffer to find out.
    */
    char    oneByteBuffer = 0;
    int        theLength = ::vsnprintf(&oneByteBuffer, 1, formatText, args);
#else
    if (VString::smSprintfBufferMutex == NULL)
        VString::smSprintfBufferMutex = new VMutex();

    VMutexLocker    locker(VString::smSprintfBufferMutex);
    int        theLength = ::vsnprintf(smSprintfBuffer, kSprintfBufferSize, formatText, args);
#endif

    va_end(args);
    
    return theLength;
    }

std::istream& operator>>(std::istream& in, VString& s)
    {
    s.readFromIStream(in);

    return in;
    }

#ifndef V_EFFICIENT_SPRINTF

char    VString::smSprintfBuffer[kSprintfBufferSize];
VMutex*    VString::smSprintfBufferMutex = new VMutex();

#endif /* V_EFFICIENT_SPRINTF */

