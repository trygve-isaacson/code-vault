/*
Copyright c1997-2011 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.2
http://www.bombaydigital.com/
*/

/** @file */

#include "vstring.h"
#include "vtypes_internal.h"

#include "vchar.h"
#include "vexception.h"
#include "vlogger.h"

#ifndef V_EFFICIENT_SPRINTF
#include "vmutex.h"
#include "vmutexlocker.h"
#endif

V_STATIC_INIT_TRACE

#undef strlen
#undef strcmp
#undef sscanf

// Is ASSERT_INVARIANT enabled/disabled specifically for VString?
#ifdef V_ASSERT_INVARIANT_VSTRING_ENABLED
    #undef ASSERT_INVARIANT
    #if V_ASSERT_INVARIANT_VSTRING_ENABLED == 1
        #define ASSERT_INVARIANT() this->_assertInvariant() ///< Macro to call this->_assertInvariant().
    #else
        #define ASSERT_INVARIANT() ((void) 0) ///< No-op.
    #endif
#endif

/*
The "empty string" constant constructs to an empty string.
When you want to pass "" to a function that takes a "const VString&" parameter,
it's much more efficient to pass the empty VString constant because it avoids
constructing a temporary empty VString object on the fly.
*/
// static
const VString& VString::EMPTY()
    {
    static const VString kEmptyString;
    return kEmptyString;
    }

// static
const VString& VString::NATIVE_LINE_ENDING()
    {
#ifdef VPLATFORM_WIN
    static const VString kLineEndingString(VSTRING_ARGS("%c%c", (char) 0x0D, (char) 0x0A));
#else /* Unix and Mac OS X use Unix style */
    static const VString kLineEndingString((char) 0x0A);
#endif
    return kLineEndingString;
    }

VString::VString() :
mStringLength(0),
mBufferLength(0),
mBuffer(NULL)
    {
    ASSERT_INVARIANT();
    }

VString::VString(const VChar& c) :
mStringLength(0),
mBufferLength(0),
mBuffer(NULL)
    {
    this->preflight(1);
    mBuffer[0] = c.charValue();
    this->_setLength(1);

    ASSERT_INVARIANT();
    }

VString::VString(const VString& s) :
mStringLength(0),
mBufferLength(0),
mBuffer(NULL)
    {
    int theLength = s.length();
    if (theLength > 0)
        {
        this->preflight(theLength);
        s.copyToBuffer(mBuffer, theLength+1);
        this->_setLength(theLength);
        }

    ASSERT_INVARIANT();
    }

VString::VString(char c) :
mStringLength(0),
mBufferLength(0),
mBuffer(NULL)
    {
    this->preflight(1);
    mBuffer[0] = c;
    this->_setLength(1);

    ASSERT_INVARIANT();
    }

#ifdef VAULT_VSTRING_STRICT_FORMATTING
VString::VString(const char* s) :
#else
VString::VString(char* s) :
#endif
mStringLength(0),
mBufferLength(0),
mBuffer(NULL)
    {
    if ((s != NULL) && (s[0] != VCHAR_NULL_TERMINATOR)) // if s is NULL or zero length, leave as initialized to empty
        {
        int    theLength = (int) ::strlen(s);
        this->preflight(theLength);
        ::memcpy(mBuffer, s, (VSizeType) mBufferLength);    // faster than strcpy?
        this->_setLength(theLength);
        }

    ASSERT_INVARIANT();
    }

#ifdef VAULT_VARARG_STRING_FORMATTING_SUPPORT

#ifndef VAULT_VSTRING_STRICT_FORMATTING
/**
Utility used in the vararg constructor's optimization; returns -1 if the string
contains possible formatting directives (i.e., any '%' character). Otherwise, returns
the string length.
@param  s   NULL or a null-terminated C string
@return -1 if s is NULL or contains any '%' character; the length of the string otherwise.
*/
static int _getStrlenIfNonFormatting(const char* s)
    {
    if (s == NULL)
        return -1;

    int len = 0;
    while (s[len] != VCHAR_NULL_TERMINATOR)
        {
        if (s[len] == '%')
            return -1;

        ++len;
        }

    return len;
    }
#endif /* VAULT_VSTRING_STRICT_FORMATTING */

#ifdef VAULT_VSTRING_STRICT_FORMATTING
VString::VString(Vs8 /*dummy*/, const char* formatText, ...) :
#else /* non-strict formatting */
VString::VString(const char* formatText, ...) :
#endif /* VAULT_VSTRING_STRICT_FORMATTING */
mStringLength(0),
mBufferLength(0),
mBuffer(NULL)
    {
    if ((formatText != NULL) && (formatText[0] != VCHAR_NULL_TERMINATOR)) // if formatText is NULL or zero length, leave as initialized to empty
        {
#ifndef VAULT_VSTRING_STRICT_FORMATTING
        // A common problem is constructing a string as "%", arriving that this constructor rather than the (char*) constructor.
        // It fails to format as desired. We can special case for this.
        if (formatText[0] == '%' && formatText[1] == VCHAR_NULL_TERMINATOR)
            {
            this->copyFromBuffer("%", 0, 1);
            }
        else
#endif
            {
#ifndef VAULT_VSTRING_STRICT_FORMATTING
            // Scan for formatting directives. If none exist, avoid overhead of trying to format, and just copy.
            int nonFormattingLen = _getStrlenIfNonFormatting(formatText);
#else
            int nonFormattingLen = -1; // not necessary if strict formatting in use
#endif
            if (nonFormattingLen == -1)
                {
                va_list args;
                va_start(args, formatText);

                this->vaFormat(formatText, args);

                va_end(args);
                }
            else
                {
                this->copyFromBuffer(formatText, 0, nonFormattingLen);
                }
            }
        }

    ASSERT_INVARIANT();
    }
#endif /* VAULT_VARARG_STRING_FORMATTING_SUPPORT */

#ifdef VAULT_QT_SUPPORT
VString::VString(const QString& s) :
mStringLength(0),
mBufferLength(0),
mBuffer(NULL)
    {
    this->copyFromBuffer(s.toUtf8(), 0, s.length());

    ASSERT_INVARIANT();
    }
#endif

#ifdef VAULT_BOOST_STRING_FORMATTING_SUPPORT
VString::VString(const boost::format& fmt) :
mStringLength(0),
mBufferLength(0),
mBuffer(NULL)
    {
    this->copyFromBuffer(fmt.str().c_str(), 0, fmt.size());

    ASSERT_INVARIANT();
    }
#endif

#ifdef VAULT_CORE_FOUNDATION_SUPPORT
VString::VString(const CFStringRef& s) :
mStringLength(0),
mBufferLength(0),
mBuffer(NULL)
    {
    this->_assignFromCFString(s);

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
            s.copyToBuffer(mBuffer, theLength+1);
            }

        this->_setLength(theLength);
        }

    ASSERT_INVARIANT();

    return *this;
    }

VString& VString::operator=(const VString* s)
    {
    ASSERT_INVARIANT();

    if (s == NULL)
        this->_setLength(0);
    else if (this != s)
        {
        int theLength = s->length();

        if (theLength != 0)
            {
            this->preflight(theLength);
            s->copyToBuffer(mBuffer, theLength+1);
            }

        this->_setLength(theLength);
        }

    ASSERT_INVARIANT();

    return *this;
    }

#ifdef VAULT_QT_SUPPORT
VString& VString::operator=(const QString& s)
    {
    ASSERT_INVARIANT();

    this->copyFromBuffer(s.toUtf8(), 0, s.length());

    ASSERT_INVARIANT();

    return *this;
    }
#endif

#ifdef VAULT_BOOST_STRING_FORMATTING_SUPPORT
VString& VString::operator=(const boost::format& fmt)
    {
    ASSERT_INVARIANT();

    this->copyFromBuffer(fmt.str().c_str(), 0, fmt.size());

    ASSERT_INVARIANT();

    return *this;
    }
#endif

#ifdef VAULT_CORE_FOUNDATION_SUPPORT
VString& VString::operator=(const CFStringRef& s)
    {
    ASSERT_INVARIANT();

    this->_assignFromCFString(s);

    ASSERT_INVARIANT();

    return *this;
    }
#endif

VString& VString::operator=(const VChar& c)
    {
    ASSERT_INVARIANT();

    this->preflight(1);
    mBuffer[0] = c.charValue();
    this->_setLength(1);

    ASSERT_INVARIANT();

    return *this;
    }

VString& VString::operator=(char c)
    {
    ASSERT_INVARIANT();

    this->preflight(1);
    mBuffer[0] = c;
    this->_setLength(1);

    ASSERT_INVARIANT();

    return *this;
    }

VString& VString::operator=(const char* s)
    {
    ASSERT_INVARIANT();

    if (s == NULL)
        this->_setLength(0);
    else
        {
        int theLength = static_cast<int> (::strlen(s));

        if (theLength != 0)
            {
            this->preflight(theLength);
            //lint -e668 "Possibly passing a null pointer to function" [OK: Preflight guarantees mBuffer exists.]
            ::memcpy(mBuffer, s, static_cast<VSizeType>(theLength)); // faster than strcpy?
            }

        this->_setLength(theLength);
        }

    ASSERT_INVARIANT();

    return *this;
    }

VString& VString::operator=(int i)
    {
    ASSERT_INVARIANT();

    this->format(VSTRING_FORMATTER_INT, i);

    ASSERT_INVARIANT();

    return *this;
    }

VString& VString::operator=(Vu8 i)
    {
    ASSERT_INVARIANT();

    this->format(VSTRING_FORMATTER_U8, (int) i);

    ASSERT_INVARIANT();

    return *this;
    }

VString& VString::operator=(Vs8 i)
    {
    ASSERT_INVARIANT();

    this->format(VSTRING_FORMATTER_S8, (int) i);

    ASSERT_INVARIANT();

    return *this;
    }

VString& VString::operator=(Vu16 i)
    {
    ASSERT_INVARIANT();

    this->format(VSTRING_FORMATTER_U16, i);

    ASSERT_INVARIANT();

    return *this;
    }

VString& VString::operator=(Vs16 i)
    {
    ASSERT_INVARIANT();

    this->format(VSTRING_FORMATTER_S16, i);

    ASSERT_INVARIANT();

    return *this;
    }

VString& VString::operator=(Vu32 i)
    {
    ASSERT_INVARIANT();

    this->format(VSTRING_FORMATTER_U32, i);

    ASSERT_INVARIANT();

    return *this;
    }

VString& VString::operator=(Vs32 i)
    {
    ASSERT_INVARIANT();

    this->format(VSTRING_FORMATTER_S32, i);

    ASSERT_INVARIANT();

    return *this;
    }

VString& VString::operator=(Vu64 i)
    {
    ASSERT_INVARIANT();

    this->format(VSTRING_FORMATTER_U64, i);

    ASSERT_INVARIANT();

    return *this;
    }

VString& VString::operator=(Vs64 i)
    {
    ASSERT_INVARIANT();

    this->format(VSTRING_FORMATTER_S64, i);

    ASSERT_INVARIANT();

    return *this;
    }

VString& VString::operator=(VDouble f)
    {
    ASSERT_INVARIANT();

    this->format(VSTRING_FORMATTER_DOUBLE, f);

    ASSERT_INVARIANT();

    return *this;
    }

VString VString::operator+(const char c) const
    {
    VString newString(VSTRING_ARGS("%s%c", this->chars(), c));
    return newString;
    }

VString VString::operator+(const char* s) const
    {
    VString newString(VSTRING_ARGS("%s%s", this->chars(), s));
    return newString;
    }

VString VString::operator+(const VString& s) const
    {
    VString newString(VSTRING_ARGS("%s%s", this->chars(), s.chars()));
    return newString;
    }

#ifdef VAULT_BOOST_STRING_FORMATTING_SUPPORT
VString VString::operator+(const boost::format& fmt) const
    {
    VString newString(*this);
    newString += fmt;
    return newString;
    }
#endif

VString& VString::operator+=(const VChar& c)
    {
    ASSERT_INVARIANT();

    this->preflight(1 + mStringLength);

    mBuffer[mStringLength] = c.charValue();
    this->_setLength(1 + mStringLength);

    ASSERT_INVARIANT();

    return *this;
    }

VString& VString::operator+=(const VString& s)
    {
    ASSERT_INVARIANT();

    // We have to be careful copying the buffer if &s==this
    // because things morph under us as we work in that case.

    int theLength = s.length();

    this->preflight(theLength + mStringLength);

    //lint -e668 "Possibly passing a null pointer to function" [OK: Preflight guarantees mBuffer exists.]
    ::memcpy(&(mBuffer[mStringLength]), s.chars(), static_cast<VSizeType> (theLength));

    this->_setLength(theLength + mStringLength);

    ASSERT_INVARIANT();

    return *this;
    }

VString& VString::operator+=(char c)
    {
    ASSERT_INVARIANT();

    this->preflight(1 + mStringLength);

    mBuffer[mStringLength] = c;
    this->_setLength(1 + mStringLength);

    ASSERT_INVARIANT();

    return *this;
    }

VString& VString::operator+=(const char* s)
    {
    ASSERT_INVARIANT();

    int theLength = (int) ::strlen(s);

    this->preflight(theLength + mStringLength);
    //lint -e668 "Possibly passing a null pointer to function" [OK: Preflight guarantees mBuffer exists.]
    ::memcpy(&(mBuffer[mStringLength]), s, static_cast<VSizeType> (theLength));
    this->_setLength(theLength + mStringLength);

    ASSERT_INVARIANT();

    return *this;
    }

#ifdef VAULT_BOOST_STRING_FORMATTING_SUPPORT
VString& VString::operator+=(const boost::format& fmt)
    {
    ASSERT_INVARIANT();

    int theLength = (int) fmt.size();

    this->preflight(theLength + mStringLength);
    //lint -e668 "Possibly passing a null pointer to function" [OK: Preflight guarantees mBuffer exists.]
    ::memcpy(&(mBuffer[mStringLength]), fmt.str().c_str(), static_cast<VSizeType> (theLength));
    this->_setLength(theLength + mStringLength);

    ASSERT_INVARIANT();

    return *this;
    }
#endif

VString& VString::operator+=(int i)
    {
    ASSERT_INVARIANT();

    *this += VSTRING_INT(i);

    ASSERT_INVARIANT();

    return *this;
    }

VString& VString::operator+=(Vu8 i)
    {
    ASSERT_INVARIANT();

    *this += VSTRING_U8((int) i); // int case req'd for some compilers

    ASSERT_INVARIANT();

    return *this;
    }

VString& VString::operator+=(Vs8 i)
    {
    ASSERT_INVARIANT();

    *this += VSTRING_S8((int) i); // int case req'd for some compilers

    ASSERT_INVARIANT();

    return *this;
    }

VString& VString::operator+=(Vu16 i)
    {
    ASSERT_INVARIANT();

    *this += VSTRING_U16(i);

    ASSERT_INVARIANT();

    return *this;
    }

VString& VString::operator+=(Vs16 i)
    {
    ASSERT_INVARIANT();

    *this += VSTRING_S16(i);

    ASSERT_INVARIANT();

    return *this;
    }

VString& VString::operator+=(Vu32 i)
    {
    ASSERT_INVARIANT();

    *this += VSTRING_U32(i);

    ASSERT_INVARIANT();

    return *this;
    }

VString& VString::operator+=(Vs32 i)
    {
    ASSERT_INVARIANT();

    *this += VSTRING_S32(i);

    ASSERT_INVARIANT();

    return *this;
    }

VString& VString::operator+=(Vu64 i)
    {
    ASSERT_INVARIANT();

    *this += VSTRING_U64(i);

    ASSERT_INVARIANT();

    return *this;
    }

VString& VString::operator+=(Vs64 i)
    {
    ASSERT_INVARIANT();

    *this += VSTRING_S64(i);

    ASSERT_INVARIANT();

    return *this;
    }

VString& VString::operator+=(VDouble d)
    {
    ASSERT_INVARIANT();

    *this += VSTRING_DOUBLE(d);

    ASSERT_INVARIANT();

    return *this;
    }

void VString::readFromIStream(std::istream& in)
    {
    ASSERT_INVARIANT();

    *this = VString::EMPTY();

    this->appendFromIStream(in);

    ASSERT_INVARIANT();
    }

void VString::appendFromIStream(std::istream& in)
    {
    ASSERT_INVARIANT();

    char c;

    in >> c;

    while (c != 0)
        {
        // Trying here to avoid reallocating buffer on every incremental character. Do it every 10 instead.
        int currentLength = this->length();
        int roundedUpLength = (currentLength + 11) - (currentLength % 10);

        this->preflight(roundedUpLength);

        *this += c;

        in >> c;
        }

    ASSERT_INVARIANT();
    }

#ifdef VAULT_VARARG_STRING_FORMATTING_SUPPORT
void VString::format(const char* formatText, ...)
    {
    ASSERT_INVARIANT();

    va_list args;
    va_start(args, formatText);

    this->vaFormat(formatText, args);

    va_end(args);

    ASSERT_INVARIANT();
    }
#endif

void VString::insert(char c, int offset)
    {
    ASSERT_INVARIANT();

    // We could make a VString of c, and then insert it, but it seems
    // much more efficient to move a single char by itself.

    int addedLength = 1;
    int oldLength = this->length();
    int newLength = oldLength + addedLength;

    // constrain to guard against bad offset; perhaps an out-of-bounds exception would be better?
    int actualOffset = V_MIN(oldLength, V_MAX(0, offset));
    int numBytesToMove = oldLength - actualOffset;

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
    const char* source = s.chars();

    if (this == &s)
        {
        tempCopy = s;    // this will cause tempCopy to allocate a buffer to hold a copy of s (which is 'this')
        source = tempCopy.chars();    // we'll do our final memcpy() from the copy, not from the 'this' buffer that's split by the memmove()
        }

    int addedLength = s.length();
    int oldLength = this->length();
    int newLength = oldLength + addedLength;

    // constrain to guard against bad offset; perhaps an out-of-bounds exception would be better?
    int actualOffset = V_MIN(oldLength, V_MAX(0, offset));
    int numBytesToMove = oldLength - actualOffset;

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
        this->_setLength(maxLength);

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

    if (i > mStringLength)
        throw VRangeException(VSTRING_FORMAT("VString::at(%d) index out of range for length %d.", i, mStringLength));
    else if (i == 0 && mStringLength == 0)
        return VChar::NULL_CHAR();

    return VChar(mBuffer[i]);
    }

VChar VString::operator[](int i) const
    {
    ASSERT_INVARIANT();

    if (i > mStringLength)
        throw VRangeException(VSTRING_FORMAT("VString::operator[%d] index out of range for length %d.", i, mStringLength));
    else if (i == 0 && mStringLength == 0)
        return VChar::NULL_CHAR();

    return VChar(mBuffer[i]);
    }

char& VString::operator[](int i)
    {
    ASSERT_INVARIANT();

    if (i >= mStringLength)
        throw VRangeException(VSTRING_FORMAT("VString::operator[%d] index out of range for length %d.", i, mStringLength));

    return mBuffer[i];
    }

char VString::charAt(int i) const
    {
    ASSERT_INVARIANT();

    if (i > mStringLength)
        throw VRangeException(VSTRING_FORMAT("VString::charAt(%d) index out of range for length %d.", i, mStringLength));
    else if (i == 0 && mStringLength == 0)
        return (char) 0;

    return mBuffer[i];
    }

VString::operator const char*() const
    {
    ASSERT_INVARIANT();

    if (mBuffer == NULL)
        return "";
    else
        return mBuffer;
    }

const char* VString::chars() const
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

    return QString::fromUtf8(this->chars(), this->length());
    }
#endif

#ifdef VAULT_CORE_FOUNDATION_SUPPORT
CFStringRef VString::cfstring() const
    {
    ASSERT_INVARIANT();

    return CFStringCreateWithCString(NULL, this->chars(), kCFStringEncodingUTF8);
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

    if (mBuffer == NULL)
        return vault::strcasecmp("", s);
    else
        return vault::strcasecmp(mBuffer, s);
    }

bool VString::startsWith(const VString& s) const
    {
    ASSERT_INVARIANT();

    return this->regionMatches(0, s, 0, s.length());
    }

bool VString::startsWithIgnoreCase(const VString& s) const
    {
    ASSERT_INVARIANT();

    return this->regionMatches(0, s, 0, s.length(), /* caseSensitive = */ false);
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

bool VString::endsWithIgnoreCase(const VString& s) const
    {
    ASSERT_INVARIANT();

    return this->regionMatches(mStringLength - s.length(), s, 0, s.length(), /* caseSensitive = */ false);
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
    
    if ((mBuffer != NULL) && (fromIndex >= 0))
        for (int i = fromIndex; i < mStringLength; ++i)
            {
            if (mBuffer[i] == c)
                return i;
            }

    return -1;
    }

int VString::indexOfIgnoreCase(char c, int fromIndex) const
    {
    ASSERT_INVARIANT();

    if ((mBuffer != NULL) && (fromIndex >= 0))
        for (int i = fromIndex; i < mStringLength; ++i)
            {
            if (VChar::equalsIgnoreCase(mBuffer[i], c))
                return i;
            }

    return -1;
    }

int VString::indexOf(const VString& s, int fromIndex) const
    {
    ASSERT_INVARIANT();
    
    if (fromIndex < 0)
        return -1;

    int otherLength = s.length();

    for (int i = fromIndex; i < mStringLength; ++i)
        {
        if (this->regionMatches(i, s, 0, otherLength))
            return i;
        }

    return -1;
    }

int VString::indexOfIgnoreCase(const VString& s, int fromIndex) const
    {
    ASSERT_INVARIANT();

    if (fromIndex < 0)
        return -1;

    int otherLength = s.length();

    for (int i = fromIndex; i < mStringLength; ++i)
        {
        if (this->regionMatches(i, s, 0, otherLength, /* caseSensitive = */ false))
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

int VString::lastIndexOfIgnoreCase(char c, int fromIndex) const
    {
    ASSERT_INVARIANT();

    if (fromIndex == -1)
        fromIndex = mStringLength - 1;

    if (mBuffer != NULL)
        for (int i = fromIndex; i >= 0; --i)
            {
            if (VChar::equalsIgnoreCase(mBuffer[i], c))
                return i;
            }

    return -1;
    }

int VString::lastIndexOf(const VString& s, int fromIndex) const
    {
    ASSERT_INVARIANT();

    int otherLength = s.length();

    if (fromIndex == -1)
        fromIndex = mStringLength;

    for (int i = fromIndex; i >= 0; --i)
        {
        if (this->regionMatches(i, s, 0, otherLength))
            return i;
        }

    return -1;
    }

int VString::lastIndexOfIgnoreCase(const VString& s, int fromIndex) const
    {
    ASSERT_INVARIANT();

    int otherLength = s.length();

    if (fromIndex == -1)
        fromIndex = mStringLength;

    for (int i = fromIndex; i >= 0; --i)
        {
        if (this->regionMatches(i, s, 0, otherLength, /* caseSensitive = */ false))
            return i;
        }

    return -1;
    }

bool VString::regionMatches(int thisOffset, const VString& otherString, int otherOffset, int regionLength, bool caseSensitive) const
    {
    ASSERT_INVARIANT();

    int result;
    int otherStringLength = otherString.length();

    // Buffer offset safety checks first. If they fail, return false.
    if ((thisOffset < 0) ||
        (thisOffset >= mStringLength) ||
        (thisOffset + regionLength > mStringLength) ||
        (otherOffset < 0) ||
        (otherOffset >= otherStringLength) ||
        (otherOffset + regionLength > otherStringLength))
        return false;

    if (caseSensitive)
        result = ::strncmp(this->chars() + thisOffset, otherString.chars() + otherOffset, static_cast<VSizeType> (regionLength));
    else
        result = vault::strncasecmp(this->chars() + thisOffset, otherString.chars() + otherOffset, static_cast<VSizeType> (regionLength));

    return (result == 0);
    }

bool VString::contains(char c, int fromIndex) const
    {
    ASSERT_INVARIANT();

    return this->indexOf(c, fromIndex) != -1;
    }

bool VString::containsIgnoreCase(char c, int fromIndex) const
    {
    ASSERT_INVARIANT();

    return this->indexOfIgnoreCase(c, fromIndex) != -1;
    }

bool VString::contains(const VString& s, int fromIndex) const
    {
    ASSERT_INVARIANT();

    return this->indexOf(s, fromIndex) != -1;
    }

bool VString::containsIgnoreCase(const VString& s, int fromIndex) const
    {
    ASSERT_INVARIANT();

    return this->indexOfIgnoreCase(s, fromIndex) != -1;
    }

int VString::replace(const VString& searchString, const VString& replacementString, bool caseSensitiveSearch)
    {
    ASSERT_INVARIANT();

    int searchLength = searchString.length();

    if (searchLength == 0)
        return 0;

    int replacementLength = replacementString.length();
    int numReplacements = 0;
    int currentOffset = caseSensitiveSearch ? this->indexOf(searchString) : this->indexOfIgnoreCase(searchString);

    while ((currentOffset != -1) && (mBuffer != NULL))
        {
        /*
        The optimization trick here is that we can place a zero byte to artificially
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

        FIXME: On second thought, this in-place replacement is dangerous IF we run out of memory
        because we've broken the invariants. The problem is that we irrevocably alter our data
        before we're guaranteed we'll succeed. This is very unlikely to actually happen, but it's possible.
        */

        mBuffer[currentOffset] = 0;    // terminate the C string buffer to create the BEFORE part
        char*   beforePart = mBuffer;
        char*   afterPart = &mBuffer[currentOffset + searchLength];
        VString alteredString(VSTRING_ARGS("%s%s%s", beforePart, replacementString.chars(), afterPart));

        // Assign the new string to ourself -- copies its buffer into ours correctly.
        // (Could be optimized to just swap buffers if we defined a new friend function or two.)
        *this = alteredString;

        // Finally we have to move currentOffset forward past the replacement part.
        currentOffset += replacementLength;

        ++numReplacements;

        // See if there is another occurrence to replace.
        currentOffset = caseSensitiveSearch ? this->indexOf(searchString, currentOffset) : this->indexOfIgnoreCase(searchString, currentOffset);
        }

    ASSERT_INVARIANT();

    return numReplacements;
    }

int VString::replace(const VChar& searchChar, const VChar& replacementChar, bool caseSensitiveSearch)
    {
    ASSERT_INVARIANT();

    int     numReplacements = 0;
    char    match = searchChar.charValue();
    char    replacement = replacementChar.charValue();

    if (mBuffer != NULL)
        for (int i = 0; i < mStringLength; ++i)
            {
            if (mBuffer[i] == match || (!caseSensitiveSearch && VChar::equalsIgnoreCase(mBuffer[i], searchChar)))
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

int VString::parseInt() const
    {
    ASSERT_INVARIANT();

    Vs64 result = this->_parseSignedInteger();
    Vs64 maxValue = V_MAX_S32;
    Vs64 minValue = V_MIN_S32;

    if (sizeof(int) == 1)
        {
        maxValue = V_MAX_S8;
        minValue = V_MIN_S8;
        }
    else if (sizeof(int) == 2)
        {
        maxValue = V_MAX_S16;
        minValue = V_MIN_S16;
        }
    else if (sizeof(int) == 8)
        {
        maxValue = V_MAX_S64;
        minValue = V_MIN_S64;
        }

    if ((result < minValue) || (result > maxValue))
        throw VRangeException(VSTRING_FORMAT("VString::parseInt %s value is out of range.", mBuffer));

    return static_cast<int>(result);
    }

Vs64 VString::parseS64() const
    {
    ASSERT_INVARIANT();

    Vs64 result = this->_parseSignedInteger();

    return result;
    }

Vu64 VString::parseU64() const
    {
    ASSERT_INVARIANT();

    Vu64 result = this->_parseUnsignedInteger();

    return result;
    }

VDouble VString::parseDouble() const
    {
    ASSERT_INVARIANT();

    if (mStringLength == 0)
        return 0.0;

    VDouble result;
    int n = ::sscanf(mBuffer, VSTRING_FORMATTER_DOUBLE, &result);
    if (n == 0)
        throw VRangeException(VSTRING_FORMAT("VString::parseDouble '%s' is invalid format.", mBuffer));

    return result;
    }

void VString::set(int i, const VChar& c)
    {
    ASSERT_INVARIANT();

    if (i >= mStringLength)
        throw VRangeException(VSTRING_FORMAT("VString::set(%d,%c) index out of range for string length %d.", i, c.charValue(), mStringLength));

    if (mBuffer != NULL)
        mBuffer[i] = c.charValue();

    ASSERT_INVARIANT();
    }

void VString::getSubstring(VString& toString, int startIndex, int endIndex) const
    {
    ASSERT_INVARIANT();

    int theLength = this->length();

    startIndex = V_MAX(0, startIndex);        // prevent negative start index
    startIndex = V_MIN(theLength, startIndex);    // prevent start past end

    if (endIndex == -1)    // -1 means to end of string
        endIndex = theLength;

    endIndex = V_MIN(theLength, endIndex);        // prevent stop past end
    endIndex = V_MAX(startIndex, endIndex);    // prevent stop before start

    if (mBuffer == NULL)
        toString = VString::EMPTY();
    else
        toString.copyFromBuffer(mBuffer, startIndex, endIndex);
    }

void VString::substringInPlace(int startIndex, int endIndex)
    {
    ASSERT_INVARIANT();

    int theLength = this->length();

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
        this->_setLength(newLength);
        }

    ASSERT_INVARIANT();
    }

void VString::split(VStringVector& result, const VChar& delimiter, int limit, bool stripTrailingEmpties)
    {
    result.clear();
    VString nextItem;
    const int len = this->length();
    for (int index = 0; index < len; ++index)
        {
        VChar c = mBuffer[index];
        if (c == delimiter)
            {
            result.push_back(nextItem);
            nextItem = VString::EMPTY();
            
            if ((limit != 0) && (((int) result.size()) == limit-1))
                {
                // We are 1 less than the limit, so the rest of the string is the remaining item.
                this->getSubstring(nextItem, index+1);
                result.push_back(nextItem);
                nextItem = VString::EMPTY();
                break;
                }
            }
        else
            {
            nextItem += c;
            }
        }

    if (nextItem.isNotEmpty())
        result.push_back(nextItem);
    
    // Strip trailing empty strings if specified.
    if (stripTrailingEmpties)
        {
        while (result[result.size() - 1].isEmpty())
            result.erase(result.end() - 1);
        }
    }

VStringVector VString::split(const VChar& delimiter, int limit, bool stripTrailingEmpties)
    {
    VStringVector result;
    this->split(result, delimiter, limit, stripTrailingEmpties);
    return result;
    }

void VString::trim()
    {
    ASSERT_INVARIANT();

    int theLength = mStringLength;

    // optimize for empty string condition
    if (theLength == 0)
        return;

    int indexOfFirstNonWhitespace = -1;
    int indexOfLastNonWhitespace = -1;

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
        this->_setLength(0);
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

        this->_setLength(numBytesAfterTrimming);
        }

    ASSERT_INVARIANT();
    }

void VString::copyToBuffer(char* toBuffer, int bufferSize) const
    {
    ASSERT_INVARIANT();

    if (toBuffer == NULL)
        throw VRangeException("VString::copyToBuffer: target buffer pointer is null.");

    if ((mBuffer == NULL) || (mBufferLength == 0) || (mStringLength == 0))
        {
        toBuffer[0] = VCHAR_NULL_TERMINATOR;
        }
    else if (mStringLength < bufferSize)
        {
        ::memcpy(toBuffer, mBuffer, static_cast<VSizeType>(1 + mStringLength)); // includes our null terminator
        }
    else
        {
        ::memcpy(toBuffer, mBuffer, static_cast<VSizeType>(bufferSize - 1)); // only copying the part that will fit
        toBuffer[bufferSize-1] = VCHAR_NULL_TERMINATOR;
        }
    }

void VString::copyFromBuffer(const char* fromBuffer, int startIndex, int endIndex)
    {
    ASSERT_INVARIANT();

    if (startIndex < 0)
        throw VRangeException(VSTRING_FORMAT("VString::copyFromBuffer: out of range start index %d.", startIndex));

    // We allow endIndex to be less than startIndex, and compensate for that
    if (endIndex < startIndex)
        endIndex = startIndex;

    this->preflight(endIndex - startIndex);
    //lint -e668 "Possibly passing a null pointer to function" [OK: Preflight guarantees mBuffer exists.]
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
        Vu8 constrainedLength = static_cast<Vu8> (V_MIN(255, mStringLength));

        pascalBuffer[0] = static_cast<char> (constrainedLength);
        ::memcpy(&(pascalBuffer[1]), mBuffer, constrainedLength);
        }
    }

void VString::copyFromPascalString(const char* pascalBuffer)
    {
    ASSERT_INVARIANT();

    int theLength = static_cast<int> (static_cast<Vu8> (pascalBuffer[0]));

    this->preflight(theLength);
    //lint -e668 "Possibly passing a null pointer to function" [OK: Preflight guarantees mBuffer exists.]
    ::memcpy(mBuffer, &(pascalBuffer[1]), static_cast<VSizeType> (theLength));
    this->postflight(theLength);

    ASSERT_INVARIANT();
    }

void VString::setFourCharacterCode(Vu32 fourCharacterCode)
    {
    ASSERT_INVARIANT();

    char codeChars[4];

    for (int i = 0; i < 4; ++i)
        {
        int bitsToShift = 8 * (3-i);
        Vu8 byteValue = static_cast<Vu8>((fourCharacterCode & (static_cast<Vu32>(0x000000FF) << bitsToShift)) >> bitsToShift);
        codeChars[i] = byteValue;

        if (codeChars[i] == 0)
            throw VRangeException(VSTRING_FORMAT("VString::setFourCharacterCode: Code 0x%08X has a zero byte.", fourCharacterCode));
        }

    this->copyFromBuffer(codeChars, 0, 4);

    ASSERT_INVARIANT();
    }

Vu32 VString::getFourCharacterCode() const
    {
    ASSERT_INVARIANT();

    Vu32 result = 0;

    for (int i = 0; i < 4; ++i)
        {
        Vu8 byteValue;

        if (mStringLength > i)
            byteValue = static_cast<Vu8>(mBuffer[i]);
        else
            byteValue = static_cast<Vu8>(' ');

        int bitsToShift = 8 * (3-i);
        Vu32 orValue = (byteValue << bitsToShift) & (0x000000FF << bitsToShift);

        result |= orValue;
        }

    return result;
    }

static const int kStringBufferChunkSize = 32;

void VString::preflight(int stringLength)
    {
    ASSERT_INVARIANT();

    if (stringLength < 0)
        throw VRangeException(VSTRING_FORMAT("VString::preflight: negative length %d.", stringLength));

    if ((mBuffer == NULL) || (stringLength >= mBufferLength))    // buffer must be at least stringLength + 1, or we need to reallocate
        {
        try
            {
            // Allocate the buffer in reasonable sized chunks rather than using the exact size
            // requested; this easily yields an order of magnitude improvement when a string is
            // created with multiple appends.
            int newBufferLength = stringLength + 1;
            if (kStringBufferChunkSize != 1)
                {
                int remainder = newBufferLength % kStringBufferChunkSize;
                int extra = kStringBufferChunkSize - remainder;
                newBufferLength += extra;
                }

            // This allocation will throw an exception if we run out of memory. Catch below.
            char* newBuffer = new char[newBufferLength];

            // Copy our old string, including the null terminator, to the new buffer.
            if (mBufferLength == 0)
                {
                // If the old buffer length was zero, there are no characters to copy.
                // Just set the null terminator byte at the start of the new buffer.
                newBuffer[0] = 0;
                }
            else
                {
                // Copy our old data, including the null terminator byte, to the new buffer.
                // We know that stringLength is greater than mStringLength, or we wouldn't be growing
                // the buffer in the first place.
                //lint -e668 -e669 -e670 "Possibly passing a null pointer to function" [OK: if() clause handles null mBuffer condition.]
                ::memcpy(newBuffer, mBuffer, static_cast<VSizeType>(mStringLength + 1));
                }

            // Release the old buffer and assign the new buffer and length.
            // We haven't changed the mStringLength; we've merely copied data to a larger buffer.
            delete [] mBuffer;
            mBuffer = newBuffer;
            mBufferLength = newBufferLength;
            }
        // About the exceptions we catch here:
        // We don't know all the possible behaviors of the exceptions that might be thrown on different platforms.
        // There are really three uses cases to handle, and we can handle them uniformly with the two catch blocks
        // below. We may catch a VException if SEH translation is installed and a low-level failure occurred. The known
        // case for this is a garbage (but almost good-looking) VString, whereby we memcpy() above and that fails.
        // The SEH will kick in, we translate it to a VException and land here. Because VException inherits from
        // std::exception, that is covered in the std::exception catch block. The other likely failure mode is when
        // we run out of memory and cannot satisfy the request to allocate the buffer via "new char[]" above. This can
        // also be the case if the requested length is garbage and requests a huge amount of memory that we do not have.
        // Since we don't know for sure whether this will be a std::exception or something else, we also need a "..." catch
        // block so that we can re-throw with a little extra information about where we are. In both cases we test the
        // invariants to make sure that our VString remains in a good state as it did on entry.
        catch (const std::exception& ex)
            {
            ASSERT_INVARIANT();
            throw VStackTraceException(VSTRING_FORMAT("VString::preflight caught exception preflighting buffer of length %d: %s", (stringLength + 1), ex.what()));
            }
        catch (...)
            {
            ASSERT_INVARIANT();
            throw VStackTraceException(VSTRING_FORMAT("VString::preflight caught exception preflighting buffer of length %d.", (stringLength + 1)));
            }
        }

    ASSERT_INVARIANT();
    }

void VString::preflightWithSimulatedFailure()
    {
    ASSERT_INVARIANT();

    throw VStackTraceException("VString::preflight unable to allocate buffer. (Simulated failure)");
    }

char* VString::buffer()
    {
    ASSERT_INVARIANT();

    return mBuffer;
    }

Vu8* VString::getDataBuffer()
    {
    ASSERT_INVARIANT();

    return reinterpret_cast<Vu8*>(mBuffer);
    }

const Vu8* VString::getDataBufferConst() const
    {
    ASSERT_INVARIANT();

    return reinterpret_cast<const Vu8*>(mBuffer);
    }

char* VString::orphanDataBuffer()
    {
    ASSERT_INVARIANT();

    char* orphanedBuffer = mBuffer;

    mStringLength = 0;
    mBufferLength = 0;
    mBuffer = NULL;

    ASSERT_INVARIANT();

    return orphanedBuffer;
    }

void VString::postflight(int stringLength)
    {
    /*
    Do not assert invariant on entry -- postflight's job is to clean up
    after a buffer copy that presumably has made the invariant state
    invalid.
    */

    this->_setLength(stringLength);

    ASSERT_INVARIANT();
    }

#ifdef VAULT_VARARG_STRING_FORMATTING_SUPPORT
void VString::vaFormat(const char* formatText, va_list args)
    {
    ASSERT_INVARIANT();

    if (formatText == NULL)
        {
        this->_setLength(0);
        }
    else
        {
        int newStringLength = VString::_determineSprintfLength(formatText, args);

        if (newStringLength == -1)
            {
            // We were unable to determine the buffer length needed. Log an error and make the preflight
            // use as big a buffer as we dare: how about the size of the temporary formatting buffer.
            const int kTruncatedStringLength = 32768;
            VLOGGER_ERROR(VSTRING_FORMAT("VString: formatted string will be truncated to %d characaters.", kTruncatedStringLength));
            newStringLength = kTruncatedStringLength;
            }

        this->preflight(newStringLength);

        (void) vault::vsnprintf(mBuffer, static_cast<VSizeType> (mBufferLength), formatText, args);

        this->_setLength(newStringLength); // could call postflight, but would do extra assertion check
        }

    ASSERT_INVARIANT();
    }
#endif

void VString::_setLength(int stringLength)
    {
    if (stringLength < 0)
        throw VRangeException(VSTRING_FORMAT("VString::_setLength: Out of bounds negative value %d.", stringLength));

    if ((mBuffer == NULL) && (stringLength != 0))
        throw VRangeException(VSTRING_FORMAT("VString::_setLength: Out of bounds value %d with null buffer pointer.", stringLength));

    if (mBuffer != NULL)
        {
        if (stringLength >= mBufferLength)
            throw VRangeException(VSTRING_FORMAT("VString::_setLength: Out of bounds value %d exceeds buffer length of %d.", stringLength, mBufferLength));

        if (stringLength == 0)
            {
            // String length is being set to zero. Get rid of our buffer.
            delete [] mBuffer;
            mBuffer = NULL;
            mBufferLength = 0;
            }
        else /* */
            {
            mBuffer[stringLength] = 0;
            }
        }

    // At this point we have validated the stringLength, even if mBuffer is NULL.
    mStringLength = stringLength;
    }

Vs64 VString::_parseSignedInteger() const
    {
    Vs64 result = CONST_S64(0);
    Vs64 multiplier = CONST_S64(1);

    if (mStringLength != 0)
        {
        // Iterate over the characters backwards, building the result.
        // If we encounter something illegal, throw the VRangeException.
        for (int i = mStringLength-1; i >= 0; --i)
            {
            switch (mBuffer[i])
                {
                case '-':
                    if (i != 0)
                        throw VRangeException(VSTRING_FORMAT("VString::_parseSignedInteger %c at index %d is invalid format.", mBuffer[i], i));

                    result = -result;
                    break;

                case '+':
                    if (i != 0)
                        throw VRangeException(VSTRING_FORMAT("VString::_parseSignedInteger %c at index %d is invalid format.", mBuffer[i], i));
                    break;

                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    result += (multiplier * (static_cast<int>(mBuffer[i] - '0')));
                    break;

                default:
                    throw VRangeException(VSTRING_FORMAT("VString::_parseSignedInteger %c at index %d is invalid format.", mBuffer[i], i));
                    break;
                }

            multiplier *= 10;
            }
        }

    return result;
    }

Vu64 VString::_parseUnsignedInteger() const
    {
    Vu64 result = CONST_U64(0);
    Vs64 multiplier = CONST_S64(1);

    if (mStringLength != 0)
        {
        // Iterate over the characters backwards, building the result.
        // If we encounter something illegal, throw the VRangeException.
        for (int i = mStringLength-1; i >= 0; --i)
            {
            switch (mBuffer[i])
                {
                case '-':
                    throw VRangeException(VSTRING_FORMAT("VString::_parseUnsignedInteger %c at index %d is invalid format.", mBuffer[i], i));
                    break;

                case '+':
                    if (i != 0)
                        throw VRangeException(VSTRING_FORMAT("VString::_parseUnsignedInteger %c at index %d is invalid format.", mBuffer[i], i));
                    break;

                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    result += (multiplier * (static_cast<int>(mBuffer[i] - '0')));
                    break;

                default:
                    throw VRangeException(VSTRING_FORMAT("VString::_parseUnsignedInteger %c at index %d is invalid format.", mBuffer[i], i));
                    break;
                }

            multiplier *= 10;
            }
        }

    return result;
    }

void VString::_assertInvariant() const
    {
    if (mBuffer == NULL)
        return;

    VASSERT_NOT_EQUAL(mBuffer, VCPP_DEBUG_BAD_POINTER_VALUE);
    VASSERT_GREATER_THAN_OR_EQUAL(mStringLength, 0);
    VASSERT_GREATER_THAN(mBufferLength, mStringLength); // buffer must always have room for null terminator not included in string length
    VASSERT(mBuffer[mStringLength] == VCHAR_NULL_TERMINATOR);
    }

#ifdef VAULT_VARARG_STRING_FORMATTING_SUPPORT
// static
int VString::_determineSprintfLength(const char* formatText, va_list args)
    {
#ifdef V_EFFICIENT_SPRINTF
    /*
    The IEEE 1003.1 standard states that we can call vsnprintf with
    a length of 0 and a null buffer pointer, and it shall return the
    number of bytes that would have been written had n been sufficiently
    large, excluding the terminating null byte. However, this does not
    work with all libraries, so we set V_EFFICIENT_SPRINTF in the
    platform header if we can use this feature.
    */
    int theLength = vault::vsnprintf(NULL, 0, formatText, args);
#else
    if (VString::gSprintfBufferMutex == NULL)
        VString::gSprintfBufferMutex = new VMutex("VString::gSprintfBufferMutex");

    VMutexLocker locker(VString::gSprintfBufferMutex, "VString::_determineSprintfLength");
    int theLength = vault::vsnprintf(gSprintfBuffer, kSprintfBufferSize, formatText, args);
#endif

    va_end(args);

    return theLength;
    }
#endif

#ifdef VAULT_CORE_FOUNDATION_SUPPORT
void VString::_assignFromCFString(const CFStringRef& s)
    {
    const char* cfStringBuffer = CFStringGetCStringPtr(s, kCFStringEncodingUTF8);
    if (cfStringBuffer != NULL) // was able to get fast direct buffer access
        {
        this->copyFromBuffer(cfStringBuffer, 0, strlen(cfStringBuffer));
        }
    else
        {
        bool success = false;
        int originalLength = static_cast<int>(CFStringGetLength(s));
        int finalLength = originalLength;

        // Because of expansion that can happen during transcoding, the number of bytes we need
        // might be more than the number of "characters" in the CFString. We start by trying with
        // a buffer of the exact size, and if that fails, we try with double the space, and finally
        // with quadruple the space. The only way this will fail is if the string contains
        // mostly non-Roman characters and thus has lots of multi-byte data. (If CFString could
        // tell us the output length ahead of time or upon failure, we could use that. Or, we
        // could use a doubling factor and keep trying until we get it.)
        this->preflight(finalLength);
        success = CFStringGetCString(s, mBuffer, static_cast<CFIndex>(finalLength+1), kCFStringEncodingUTF8);

        // try with a 2x buffer
        if (! success)
            {
            finalLength *= 2;
            this->preflight(finalLength);
            success = CFStringGetCString(s, mBuffer, static_cast<CFIndex>(finalLength+1), kCFStringEncodingUTF8);
            }

        // try with a 4x buffer
        if (! success)
            {
            finalLength *= 2;
            this->preflight(finalLength);
            success = CFStringGetCString(s, mBuffer, static_cast<CFIndex>(finalLength+1), kCFStringEncodingUTF8);
            }

        if (! success)
            throw VStackTraceException(VSTRING_FORMAT("VString CFStringRef constructor allocated up to %d bytes, which was insufficient for CFStringRef of length %d.", finalLength, originalLength));

        this->_setLength(finalLength);
        }
    }
#endif

std::istream& operator>>(std::istream& in, VString& s)
    {
    s.readFromIStream(in);

    return in;
    }

#ifndef V_EFFICIENT_SPRINTF

char    VString::gSprintfBuffer[kSprintfBufferSize];
VMutex* VString::gSprintfBufferMutex = new VMutex("VString::gSprintfBufferMutex", true/*this mutex itself must not log*/);

#endif /* V_EFFICIENT_SPRINTF */

