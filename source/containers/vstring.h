/*
Copyright c1997-2005 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.3.2
http://www.bombaydigital.com/
*/

#ifndef vstring_h
#define vstring_h

/** @file */

#include "vtypes.h"

#include <stdarg.h>
#include <vector>
#include <iostream>

class VChar;

#ifndef V_EFFICIENT_SPRINTF
class VMutex;
#endif

#ifdef VAULT_QT_SUPPORT
#include <qstring.h>
#endif

/**

    @defgroup vstring Vault Strings

    The Vault provides a VString class that it uses for all strings.
    It is a good way to avoid the unavoidable bugs associated with
    using raw C strings, and it provides many useful ways of working
    with strings in a safe way.
    
    A related class, VChar, can be used to represent a single character
    and the operations that are useful for characters.
    
*/
    
/**
    @ingroup vstring
*/

/**
VString is a string container class that should help to eliminate almost all
use of char buffer and char* and their inherent dangers, in the Vault APIs
and most source code.

VString objects can be created from raw char buffers, by formatting, or by
reserving space and later assigning. Methods and operators are provided for
concatenation, comparison, assignment, conversion, array access, searching,
and formatting.

Methods that modify the string will expand the buffer as necessary, so you
don't have to worry about overflowing the buffer. If the buffer needs to be
expanded but the object is unable to expand it, a VException will be thrown.
*/
class VString
    {
    public:
    
        static const VString kEmptyString; ///< An empty string. More efficient to use than "" as a const VString& parameter.
    
        /**
        Constructs an empty string.
        */
        VString();
        /**
        Constructs a string from a character.
        @param    c    the character
        */
        VString(const VChar& c);
        /**
        Constructs a string from another string.
        @param    s    the string to copy
        */
        VString(const VString& s);
        /**
        Constructs a string from a char. The explicit keyword is to
        prevent the previous VString s(n) meaning of preflight string
        to size "n" from compiling.
        @param    c    the char
        */
        explicit VString(const char c);
        /**
        Constructs a string from a C string.
        Note that the param is not marked const so that
        we avoid ambiguous linkage with the vararg ctor below.
        @param    s    pointer to C string to copy
        */
        VString(char* s);
        /**
        Constructs a string by sprintf-like formatting.
        @param    formatText    the format text
        @param    ...            varargs to be formatted
        */
        VString(const char* formatText, ...);
#ifdef VAULT_QT_SUPPORT
        /**
        Constructs a string from a QString.
        @param    s    the Qstring to copy
        */
        VString(const QString& s);
#endif
#ifdef VAULT_CORE_FOUNDATION_SUPPORT
        /**
        Constructs a string from a CFStringRef.
        @param    s    the CFStringRef to copy
        */
        VString(const CFStringRef& s);
#endif
        /**
        Destructor.
        */
        virtual ~VString();

        /**
        Copy constructor.
        @param    s    the string to copy
        */
        VString& operator=(const VString& s);
        /**
        Copy constructor.
        @param    s    the string pointer to copy
        */
        VString& operator=(const VString* s);
#ifdef VAULT_QT_SUPPORT
        /**
        Copy constructor from QString.
        @param    s    the QString to copy
        */
        VString& operator=(const QString& s);
#endif
#ifdef VAULT_CORE_FOUNDATION_SUPPORT
        /**
        Copy constructor from CFStringRef.
        @param    s    the CFStringRef to copy
        */
        VString& operator=(const CFStringRef& s);
#endif
        /**
        Assigns the string from a character.
        @param    c    the character
        */
        VString& operator=(const VChar& c);
        /**
        Assigns the string from a char.
        @param    c    the char
        */
        VString& operator=(char c);
        /**
        Assigns the string from a C string.
        @param    s    pointer to the C string to copy
        */
        VString& operator=(const char* s);
        /**
        Assigns the string from an int.
        @param    i    the integer value to be formatted
        */
        VString& operator=(int i);
        /**
        Assigns the string from a Vu8.
        @param    i    the integer value to be formatted
        */
        VString& operator=(Vu8 i);
        /**
        Assigns the string from a Vs8.
        @param    i    the integer value to be formatted
        */
        VString& operator=(Vs8 i);
        /**
        Assigns the string from a Vu16.
        @param    i    the integer value to be formatted
        */
        VString& operator=(Vu16 i);
        /**
        Assigns the string from a Vs16.
        @param    i    the integer value to be formatted
        */
        VString& operator=(Vs16 i);
        /**
        Assigns the string from a Vu32.
        @param    i    the integer value to be formatted
        */
        VString& operator=(Vu32 i);
        /**
        Assigns the string from a Vs32.
        @param    i    the integer value to be formatted
        */
        VString& operator=(Vs32 i);
        /**
        Assigns the string from a Vu64.
        @param    i    the integer value to be formatted
        */
        VString& operator=(Vu64 i);
        /**
        Assigns the string from a Vs64.
        @param    i    the integer value to be formatted
        */
        VString& operator=(Vs64 i);
        /**
        Assigns the string from a VDouble.
        @param    f    the float value to be formatted
        */
        VString& operator=(VDouble f);
        

        /**
        Appends a string + char
        @param    rhs    the string to append
        */
        VString operator+ (const char c) const;
        /**
        Appends 2 strings
        @param    rhs    the string to append
        */
        VString operator+ (const char* s) const;
        /**
        Appends 2 strings
        @param    rhs    the string to append
        */
        VString operator+ (const VString& s) const;
        
        /**
        Appends a character to the string.
        @param    c    the character
        */
        VString& operator+=(const VChar& c);
        /**
        Appends another string to the string.
        @param    s    the string to copy
        */
        VString& operator+=(const VString& s);
        /**
        Appends a char to the string.
        @param    c    the char
        */
        VString& operator+=(char c);
        /**
        Appends a C string to the string.
        @param    s    pointer to the C string to copy
        */
        VString& operator+=(const char* s);
        /**
        Appends to the string from an int.
        @param    i    the integer value to be formatted
        */
        VString& operator+=(int i);
        /**
        Appends to the string from a Vu8.
        @param    i    the integer value to be formatted
        */
        VString& operator+=(Vu8 i);
        /**
        Appends to the string from a Vs8.
        @param    i    the integer value to be formatted
        */
        VString& operator+=(Vs8 i);
        /**
        Appends to the string from a Vu16.
        @param    i    the integer value to be formatted
        */
        VString& operator+=(Vu16 i);
        /**
        Appends to the string from a Vs16.
        @param    i    the integer value to be formatted
        */
        VString& operator+=(Vs16 i);
        /**
        Appends to the string from a Vu32.
        @param    i    the integer value to be formatted
        */
        VString& operator+=(Vu32 i);
        /**
        Appends to the string from a Vs32.
        @param    i    the integer value to be formatted
        */
        VString& operator+=(Vs32 i);
        /**
        Appends to the string from a Vu64.
        @param    i    the integer value to be formatted
        */
        VString& operator+=(Vu64 i);
        /**
        Appends to the string from a Vs64.
        @param    i    the integer value to be formatted
        */
        VString& operator+=(Vs64 i);
        /**
        Appends to the string from a VDouble.
        @param    f    the float value to be formatted
        */
        VString& operator+=(VDouble f);
        /**
        Builds the string by reading from an istream.
        @param    in    the input stream
        */
        void readFromIStream(std::istream& in);
        /**
        Appends to the string by reading from an istream.
        @param    in    the input stream
        */
        void appendFromIStream(std::istream& in);
        
        /**
        Formats the string by sprintf-like formatting.
        @param    formatText    the format text
        @param    ...            varargs to be formatted
        */
        void    format(const char* formatText, ...);
        /**
        Inserts the specified character into the string at the
        specified offset (default is at the front of the string),
        moving the remaining part of the string later in the buffer.
        Thus the length of the string increases by 1 character.
        To append, you are better off using operator+=. 
        @param    c        the char to insert
        @param    offset    the location to insert in front of
        */
        void    insert(char c, int offset=0);
        /**
        Inserts the specified string into the string at the
        specified offset (default is at the front of the string),
        moving the remaining part of the string later in the buffer.
        Thus the length of the string increases by the length of
        the string being inserted.
        To append, you are better off using operator+=. 
        @param    s        the string to insert
        @param    offset    the location to insert in front of
        */
        void    insert(const VString& s, int offset=0);

        /**
        Returns the string length.
        @return the string length
        */
        int        length() const;
        /**
        Truncates the string to specified length; if the string is already
        that length or less, nothing happens.
        @param    length    the length to truncate to
        */
        void    truncateLength(int maxLength);
        /**
        Returns true if the string length is zero.
        @return true if the string length is zero
        */
        bool    isEmpty() const;
        /**
        Returns true if the string length is non-zero.
        @return true if the string length is non-zero
        */
        bool    isNotEmpty() const;
        /**
        Returns the character at the specified index.
        If the index is out of range, a VException is thrown.
        @param    i    the index (0 to length-1)
        @return    the character
        */
        VChar    at(int i) const;
        /**
        Returns the character at the specified index.
        If the index is out of range, a VException is thrown.
        @param    i    the index (0 to length-1)
        @return    the character
        */
        VChar    operator[](int i) const;
        /**
        Returns a reference to the char at the specified index, which
        can be assigned to.
        If the index is out of range, a VException is thrown.
        @param    i    the index (0 to length-1)
        @return    the character
        */
        char&    operator[](int i);
        /**
        Returns the char at the specified index.
        If the index is out of range, a VException is thrown.
        @param    i    the index (0 to length-1)
        @return    the char
        */
        char    charAt(int i) const;
        /**
        Coerces the string to a C string, for use with APIs that take
        a char* parameter.
        In fact, this coercion returns the string buffer pointer.
        @return    the char buffer pointer
        */
        operator char*() const;
        /**
        Returns the string as a C string, for use with APIs that take
        a char* parameter.
        In fact, this coercion returns the string buffer pointer.
        This is the same as operator char*, but is explicit. You need
        an explicit method like this when passing to an API that
        does not ensure a proper cast, such as vararg calls like
        printf and sprintf.
        
        Example:    \c    printf("string='%s'\n", myString->chars());
        
        In the example above, simply passing myString would result in
        garbage output, because printf would not know how to treat
        myString other than as a pointer value.
        @return    the char buffer pointer
        */
        char*    chars() const;
#ifdef VAULT_QT_SUPPORT
        /**
        Returns a QString built from the VString.
        @return    the QString
        */
        QString qstring() const;
#endif
#ifdef VAULT_CORE_FOUNDATION_SUPPORT
        /**
        Returns a CFStringRef built from the VString. The returned
        CFStringRef must be released at some point by the caller since it is a
        new object.
        @return    the CFStringRef
        */
        CFStringRef cfstring() const;
#endif
        /**
        Returns true if this string is equal to the specified string,
        ignoring case, using strcmp semantics.
        @param    s    the string to compare with
        @return    true if the strings are equal, case-insensitive
        */
        bool    equalsIgnoreCase(const VString& s) const;
        /**
        Returns true if this string is equal to the specified C string,
        ignoring case, using strcmp semantics.
        @param    s    the C string to compare with
        @return    true if the strings are equal, case-insensitive
        */
        bool    equalsIgnoreCase(const char* s) const;
        /**
        Returns the comparison value of this string and the supplied
        string, using strcmp semantics.
        @param    s    the string to compare with
        @return    <0, 0, or >0, depending on how the strings compare
        */
        int        compare(const VString& s) const;
        /**
        Returns the comparison value of this string and the supplied
        C string, using strcmp semantics.
        @param    s    the C string to compare with
        @return    <0, 0, or >0, depending on how the strings compare
        */
        int        compare(const char* s) const;
        /**
        Returns the comparison value of this string and the supplied
        string, ignoring case, using strcmp semantics.
        @param    s    the string to compare with
        @return    <0, 0, or >0, depending on how the strings compare
        */
        int        compareIgnoreCase(const VString& s) const;
        /**
        Returns the comparison value of this string and the supplied
        C string, ignoring case, using strcmp semantics.
        @param    s    the C string to compare with
        @return    <0, 0, or >0, depending on how the strings compare
        */
        int        compareIgnoreCase(const char* s) const;
        /**
        Returns true if this string starts with the specified string.
        @param    s    the string to search for
        @return true if this string starts with the specified string
        */
        bool    startsWith(const VString& s) const;
        /**
        Returns true if this string starts with the specified char.
        @param    c    the char to search for
        @return true if this string starts with the specified char
        */
        bool    startsWith(char c) const;
        /**
        Returns true if this string ends with the specified string.
        @param    s    the string to search for
        @return true if this string ends with the specified string
        */
        bool    endsWith(const VString& s) const;
        /**
        Returns true if this string ends with the specified char.
        @param    c    the char to search for
        @return true if this string ends with the specified char
        */
        bool    endsWith(char c) const;
        /**
        Returns the index of the first occurrence of the specified character.
        @param    c            the character to search for
        @param    fromIndex    index in this string to start the search from
        @return    the index where the character was found, or -1 if not found
        */
        int        indexOf(char c, int fromIndex=0) const;
        /**
        Returns the index of the first occurrence of the specified string.
        @param    s            the string to look for
        @param    fromIndex    index in this string to start the search from
        @return    the index where the string was found, or -1 if not found
        */
        int        indexOf(const VString& s, int fromIndex=0) const;
        /**
        Returns the index of the last occurrence of the specified character.
        @param    c            the character to search for
        @param    fromIndex    index in this string to start the backward search from,
                            with -1 indicating a search from the end
        @return    the index where the character was found, or -1 if not found
        */
        int        lastIndexOf(char c, int fromIndex=-1) const;
        /**
        Returns the index of the last occurrence of the specified string.
        @param    s            the string to look for
        @param    fromIndex    index in this string to start the backward search from,
                            with -1 indicating a search from the end
        @return    the index where the string was found, or -1 if not found
        */
        int        lastIndexOf(const VString& s, int fromIndex=-1) const;
        /**
        Returns true if the specified range of this string matches the
        specified range of the specified string. If the characters in
        the ranges do not match, or if either range goes past the end
        of either string, then the result is false.
        
        @param    thisIndex        the start index (0 to this this->length()-1) in this string to start the match
        @param    otherString        the string to match
        @param    otherIndex        the start index (0 to other otherString.length()-1) in the other string to start the match
        @param    regionLength    the number of characters to compare
        @return true if the ranges match
        */
        bool    regionMatches(int thisIndex, const VString& otherString, int otherIndex, int regionLength) const;
        /**
        Replaces every occurrence of the specified search string with the supplied
        replacement string. Returns the number of replacements performed, which may
        be zero.
        @param    searchString        the string to search for
        @param    replacementString    the string to replace the search string with
        @return the number of replaced occurrences
        */
        int        replace(const VString& searchString, const VString& replacementString);
        /**
        Replaces every occurrence of the specified search string with the supplied
        replacement string. Returns the number of replacements performed, which may
        be zero.
        @param    searchChar        the character to search for
        @param    replacementChar    the character to replace the search character with
        @return the number of replaced occurrences
        */
        int        replace(const VChar& searchChar, const VChar& replacementChar);

        /**
        Folds the string to lower case using tolower().
        */
        void    toLowerCase();
        /**
        Folds the string to upper case using toupper().
        */
        void    toUpperCase();
        /**
        Sets the character at the specified index to the specified value.
        If the index is out of range, a VException is thrown.
        @param    i    the index (0 to length-1)
        @param    c    the new character value
        */
        void    set(int i, const VChar& c);
        /**
        Copies a substring of this string to a target string. The start index
        is inclusive, that is it is the index of the first character copied;
        zero means to copy from the start of the string. The stop index is
        exclusive, that is it is one past the index of the last character
        copied; the default value of -1 means to copy all the way to the end
        of the string. Thus with the defaults you get the whole string.
        @param    toString    the string to copy the specified substring into
        @param    startIndex    index of the first char to copy, inclusive
        @param    endIndex    index of the last char to copy, exclusive (end-start is the length)
        */
        void    getSubstring(VString& toString, int startIndex/* = 0*/, int endIndex=-1) const;
        /**
        Makes a substring of this string in place (contrast with getSubsring(),
        which puts the substring into a different object). The start index
        is inclusive, that is it is the index of the first character taken;
        zero means the start of the string is unchanged. The stop index is
        exclusive, that is it is one past the index of the last character
        copied; the default value of -1 means the end of the string is unchanged,
        other than the fact that the string may be shortened. Thus with the
        defaults the string is not changed at all. This function is guaranteed
        not to fail or reallocate memory, because it only shrinks the string and
        thus at most just moves bytes towards the start of the buffer and updates
        the length field.
        @param    startIndex    index of the first char to copy, inclusive
        @param    endIndex    index of the last char to copy, exclusive (end-start is the length)
        */
        void    substringInPlace(int startIndex/* = 0*/, int endIndex=-1);
        /**
        Strips leading and trailing whitespace from the string.
        Whitespace as implemented here is defined as ASCII byte
        values <= 0x20 as well as 0x7F.
        */
        void    trim();
        /**
        Copies the string's chars to the specified buffer, including the null
        terminator byte. The caller is responsible for making sure that the
        buffer is big enough to hold 1 + this->length() bytes.
        @param    toBuffer    the char buffer to copy into
        @param  bufferSize  the length of the target buffer (so we can verify capacity)
        */
        void    copyToBuffer(char* toBuffer, int bufferSize/*=LONG_MAX*/) const; // FIXME: I don't like this implicit parameter.
        /**
        Sets the string by copying a number of characters from the source buffer.
        @param    fromBuffer    the char buffer to copy from
        @param    startIndex    the offset in the buffer to start from, inclusive
        @param    endIndex    the offset in the buffer to stop at, exclusive (end-start is the length);
                            -1 indicates to use strlen on the fromBuffer and use that to locate the end
                            of the buffer, and copy up to there; if endIndex is less than startIndex
                            (including after calling strlen via the -1 parameter), we effectively copy an empty string
        previously, default value endIndex==LONG_MAX meant to calculate strlen(fromBuffer) as endIndex; let's use -1 for that,
        because assuming the source is null terminated etc. is sketchy; make the caller be explicit
        */
        void    copyFromBuffer(const char* fromBuffer, int startIndex/*=0*/, int endIndex/*=LONG_MAX*/); // FIXME: I don't like this implicit parameter.
        /**
        Copies the string's chars to the specified Pascal string buffer, using
        the Pascal string format (length byte plus data). The caller is responsible
        for making sure that the buffer is big enough to hold 1 + this->length()
        bytes, up to a maximum of 256 total bytes.
        @param    pascalBuffer    the Pascal string buffer to copy into
        */
        void    copyToPascalString(char* pascalBuffer) const;
        /**
        Sets the string by copying a Pascal string from the source buffer.
        @param    pascalBuffer    the Pascal string buffer to copy from
        */
        void    copyFromPascalString(const char* pascalBuffer);
        /**
        Sets the string from a "four character code". The input is simply a 32-bit integer
        whose 4 bytes are treated as ASCII characters (most significant byte becomes the
        first character of the string, etc.). An exception is thrown if any of the
        input bytes is zero, since that would cause a null terminator byte to be part of the
        string's character data.
        */
        void    setFourCharacterCode(Vu32 fourCharacterCode);
        /**
        Returns the "four character code" represented by the string. The first four characters
        of the string are used to compose a 32-bit integer (the first character of the string
        becomes the most significant byte, etc.). If the string has fewer than 4 characters,
        the result is padded out to 4 characters with spaces (0x20).
        */
        Vu32    getFourCharacterCode() const;
        /**
        Vararg format method used by the sprintf method, and also available
        if you have a vararg API that needs to then format a string.
        @param    formatText    the format text
        @param    args        the argument list
        */
        void    vaFormat(const char* formatText, va_list args);
    
        /**
        Ensures that the buffer is big enough to hold the specified string length.
        If preflight needs to reallocate the buffer and is unable to do so, a
        VException is thrown.
        @param    stringLength    the length of the string that will need to fit in the buffer
        */
        void    preflight(int stringLength);
        /**
        Returns the string's buffer pointer. This method should only be used
        in special circumstances by code that is also calling preflight() and
        postflight() to manage the buffer.
        @return    the buffer pointer
        */
        char*    buffer();
        /**
        Syncs the internal data to the specified length, setting the length
        and writing a null terminator into the buffer. Again, this should only
        be used in special circumstances by callers who are using preflight()
        and buffer() to do something with the buffer that cannot be done via
        the normal public API.
        */
        void    postflight(int stringLength);
        
        friend inline bool operator==(const VString& s1, const VString& s2);
        friend inline bool operator==(const VString& s1, const char* s2);
        friend inline bool operator==(const char* s1, const VString& s2);
        friend inline bool operator==(const VString& s, char c);
    
        friend inline bool operator!=(const VString& s1, const VString& s2);
        friend inline bool operator!=(const VString& s1, const char* s2);
        friend inline bool operator!=(const char* s1, const VString& s2);
        friend inline bool operator!=(const VString& s, char c);
    
        friend inline bool operator>=(const VString& s1, const VString& s2);
        friend inline bool operator>=(const VString& s1, const char* s2);
        friend inline bool operator>=(const char* s1, const VString& s2);
    
        friend inline bool operator<=(const VString& s1, const VString& s2);
        friend inline bool operator<=(const VString& s1, const char* s2);
        friend inline bool operator<=(const char* s1, const VString& s2);
    
        friend inline bool operator>(const VString& s1, const VString& s2);
        friend inline bool operator>(const VString& s1, const char* s2);
        friend inline bool operator>(const char* s1, const VString& s2);
    
        friend inline bool operator<(const VString& s1, const VString& s2);
        friend inline bool operator<(const VString& s1, const char* s2);
        friend inline bool operator<(const char* s1, const VString& s2);
        
    private:

        void _setLength(int stringLength);
    
        /** Asserts if any invariant is broken. */
        void assertInvariant() const;

        /**
        Determines the length of a string if it were to be sprintf'd into
        a buffer. Used by VString to preflight buffers that will be used
        with sprintf.
        @param    formatText    the format text
        @param    args        the argument list
        */
        static int _determineSprintfLength(const char* formatText, va_list args);
    
        int        mStringLength;    ///< The length of the string.
        int        mBufferLength;    ///< The length of the buffer (at least 1 longer than the string).
        char*    mBuffer;        ///< The character buffer, containing the string plus a null terminator, and possibly unused bytes beyond that.

/*
For implementations of sprintf that don't help us out by returning the
would-have-been-length value if the buffer is too small (currently this
means Win32 or perhaps it is just VC++'s lib) we need a large-ish static
buffer that is protected by a mutex that we can use instead. See
VString::determineSprintfLength().
*/
#ifndef V_EFFICIENT_SPRINTF
        static const int kSprintfBufferSize = 32768;            ///< Size of the static printf buffer used when efficient sprintf is not available.
        static char        gSprintfBuffer[kSprintfBufferSize];     ///< A static buffer used when efficient sprintf is not available.
        static VMutex*    gSprintfBufferMutex;                    ///< Mutex to protect the static buffer from multiple threads.
#endif
    };

/**
VStringVector is simply a vector of VString objects. Note that the vector
elements are objects, not pointers to objects.
*/
typedef std::vector<VString> VStringVector;
/**
VStringPtrVector is a vector of pointers to VString objects.
*/
typedef std::vector<VString*> VStringPtrVector;

inline bool operator==(const VString& s1, const VString& s2) { return ::strcmp(s1, s2) == 0; }    ///< Compares s1 and s2 for equality. @param    s1    a string @param    s2    a string @return true if s1 and s2 are equal according to strcmp()
inline bool operator==(const VString& s1, const char* s2) { return ::strcmp(s1, s2) == 0; }        ///< Compares s1 and s2 for equality. @param    s1    a string @param    s2    a C string @return true if s1 and s2 are equal according to strcmp()
inline bool operator==(const char* s1, const VString& s2) { return ::strcmp(s1, s2) == 0; }        ///< Compares s1 and s2 for equality. @param    s1    a C string @param    s2    a string @return true if s1 and s2 are equal according to strcmp()
inline bool operator==(const VString& s, char c) { return (s.length() == 1) && (s.charAt(0) == c); }    ///< Compares s and c for equality. @param    s    a string @param    c    a char @return true if s is one character in length, the character being c

inline bool operator!=(const VString& s1, const VString& s2) { return ::strcmp(s1, s2) != 0; }    ///< Compares s1 and s2 for inequality. @param    s1    a string @param    s2    a string @return true if s1 and s2 are not equal according to strcmp()
inline bool operator!=(const VString& s1, const char* s2) { return ::strcmp(s1, s2) != 0; }        ///< Compares s1 and s2 for inequality. @param    s1    a string @param    s2    a C string @return true if s1 and s2 are not equal according to strcmp()
inline bool operator!=(const char* s1, const VString& s2) { return ::strcmp(s1, s2) != 0; }        ///< Compares s1 and s2 for inequality. @param    s1    a C string @param    s2    a string @return true if s1 and s2 are not equal according to strcmp()
inline bool operator!=(const VString& s, char c) { return (s.length() != 1) || (s.charAt(0) != c); }    ///< Compares s and c for inequality. @param    s    a string @param    c    a char @return true if s is NOT one character in length, or if it's sole character is not c

inline bool operator>=(const VString& s1, const VString& s2) { return ::strcmp(s1, s2) >= 0; }    ///< Compares s1 and s2. @param    s1    a string @param    s2    a string @return true if s1 >= s2 according to strcmp()
inline bool operator>=(const VString& s1, const char* s2) { return ::strcmp(s1, s2) >= 0; }        ///< Compares s1 and s2. @param    s1    a string @param    s2    a C string @return true if s1 >= s2 according to strcmp()
inline bool operator>=(const char* s1, const VString& s2) { return ::strcmp(s1, s2) >= 0; }        ///< Compares s1 and s2. @param    s1    a C string @param    s2    a string @return true if s1 >= s2 according to strcmp()

inline bool operator<=(const VString& s1, const VString& s2) { return ::strcmp(s1, s2) <= 0; }    ///< Compares s1 and s2. @param    s1    a string @param    s2    a string @return true if s1 <= s2 according to strcmp()
inline bool operator<=(const VString& s1, const char* s2) { return ::strcmp(s1, s2) <= 0; }        ///< Compares s1 and s2. @param    s1    a string @param    s2    a C string @return true if s1 <= s2 according to strcmp()
inline bool operator<=(const char* s1, const VString& s2) { return ::strcmp(s1, s2) <= 0; }        ///< Compares s1 and s2. @param    s1    a C string @param    s2    a string @return true if s1 <= s2 according to strcmp()

inline bool operator>(const VString& s1, const VString& s2) { return ::strcmp(s1, s2) > 0; }    ///< Compares s1 and s2. @param    s1    a string @param    s2    a string @return true if s1 > s2 according to strcmp()
inline bool operator>(const VString& s1, const char* s2) { return ::strcmp(s1, s2) > 0; }        ///< Compares s1 and s2. @param    s1    a string @param    s2    a C string @return true if s1 > s2 according to strcmp()
inline bool operator>(const char* s1, const VString& s2) { return ::strcmp(s1, s2) > 0; }        ///< Compares s1 and s2. @param    s1    a C string @param    s2    a string @return true if s1 > s2 according to strcmp()

inline bool operator<(const VString& s1, const VString& s2) { return ::strcmp(s1, s2) < 0; }    ///< Compares s1 and s2. @param    s1    a string @param    s2    a string @return true if s1 < s2 according to strcmp()
inline bool operator<(const VString& s1, const char* s2) { return ::strcmp(s1, s2) < 0; }        ///< Compares s1 and s2. @param    s1    a string @param    s2    a C string @return true if s1 < s2 according to strcmp()
inline bool operator<(const char* s1, const VString& s2) { return ::strcmp(s1, s2) < 0; }        ///< Compares s1 and s2. @param    s1    a C string @param    s2    a string @return true if s1 < s2 according to strcmp()

std::istream& operator>>(std::istream& in, VString& s);                                            ///< Creates the string by reading an istream. @param    in    the input stream @param    s    the string @return the input stream
inline std::ostream& operator<<(std::ostream& out, const VString& s) { return out << s.chars(); }    ///< Writes the string to an ostream. @param    out    the output stream @param s    the string @return the output stream
inline VString& operator<<(VString& s, std::istream& in) { s.appendFromIStream(in); return s; }    ///< Appends to the string by reading an istream. @param    s    the string @param    in    the input stream @return the string

inline VString& operator<<(VString& s, const char* in) { s += in; return s; }    ///< Appends to the string by copying a C string. @param    s    the string @param    in    the input C string buffer @return the string
inline VString& operator<<(VString& s, int i) { s += i; return s; }        ///< Appends to the string by copying an int as string. @param    s    the string @param    i    the int to append @return the string
inline VString& operator<<(VString& s, Vu8 i) { s += i; return s; }        ///< Appends to the string by copying a Vu8 as string. @param    s    the string @param    i    the Vu8 to append @return the string
inline VString& operator<<(VString& s, Vs8 i) { s += i; return s; }        ///< Appends to the string by copying a Vs8 as string. @param    s    the string @param    i    the Vs8 to append @return the string
inline VString& operator<<(VString& s, Vu16 i) { s += i; return s; }    ///< Appends to the string by copying a Vu16 as string. @param    s    the string @param    i    the Vu16 to append @return the string
inline VString& operator<<(VString& s, Vs16 i) { s += i; return s; }    ///< Appends to the string by copying a Vs16 as string. @param    s    the string @param    i    the Vs16 to append @return the string
inline VString& operator<<(VString& s, Vu32 i) { s += i; return s; }    ///< Appends to the string by copying a Vu32 as string. @param    s    the string @param    i    the Vu32 to append @return the string
inline VString& operator<<(VString& s, Vs32 i) { s += i; return s; }    ///< Appends to the string by copying a Vs32 as string. @param    s    the string @param    i    the Vs32 to append @return the string
inline VString& operator<<(VString& s, Vu64 i) { s += i; return s; }    ///< Appends to the string by copying a Vu64 as string. @param    s    the string @param    i    the Vu64 to append @return the string
inline VString& operator<<(VString& s, Vs64 i) { s += i; return s; }    ///< Appends to the string by copying a Vs64 as string. @param    s    the string @param    i    the Vs64 to append @return the string
inline VString& operator<<(VString& s, VDouble f) { s += f; return s; }    ///< Appends to the string by copying a VDouble as string. @param    s    the string @param    f    the VDouble to append @return the string

#endif /* vstring_h */
