/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.0
http://www.bombaydigital.com/
*/

#ifndef vstring_h
#define vstring_h

/** @file */

#include "vtypes.h"

#include "vcodepoint.h"
#include "vstringiterator.h"

class VChar;

/**
VStringVector is simply a vector of VString objects. Note that the vector
elements are objects, not pointers to objects.
*/
typedef std::vector<VString> VStringVector;
/**
VStringPtrVector is a vector of pointers to VString objects.
*/
typedef std::vector<VString*> VStringPtrVector;

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

    A related class, VCodePoint, can be used to represent a single Unicode
    character and the operations that are useful for characters.
    
    There is another class, VChar, that is a wrapper around the old-fashioned
    single-bye C char type, which should usually be avoided because it is not
    generally compatible with Unicode data. Using VString and VCodePoint
    exclusively will lead to good Unicode behavior.

*/

/**
    @ingroup vstring
*/

/*
Best practices for VString creation:

- To pass a temporary VString constructed with vararg formatting, use VSTRING_FORMAT:
  functionThatTakesAString(VSTRING_FORMAT("my %s format %d string", "hey!", 42));

- To construct a local or instance variable with vararg formatting, use VSTRING_ARGS:
  VString s(VSTRING_ARGS("my %s format %d string", "hey!", 42));
  ... mMyInstanceVar(VSTRING_ARGS("my %s format %d string", "hey!", 42)) ...

- To simply construct a VString from a literal, you can just supply the literal, but
  if you care about optimizing compatibility "non-strict" formatting, you may prefer
  to use VSTRING_COPY. It depends whether you are assigning or initializing. Initializing
  does not benefit from this.
  VString x("my string literal");
  VString y = VSTRING_COPY("my literal");

- Where you may want to future-proof to distinguish between making a copy of a temporary
  buffer vs. simply referring to a literal (a feature I have been expirimenting with to
  determine feasibility), use VSTRING_COPY to indicate that a copy must be made. Notice
  the difference between this and the simple literal constructor above; in theory, the
  literal does not need to be copied, but here we do need to copy because the temp buffer
  may live a shorter life than the string.
  VString s = VSTRING_COPY(some_temp_buffer_returned_by_a_system_api);

- So in fact the above is a good distinction:
  If constructing with a literal, just pass it in unadorned.
  If constructing with a temporary char* buffer, wrap it with VSTRING_COPY to ensure
  copying if I later optimize away literal copying. (This is the dilemma; how do I know
  what the pointer points to?)
*/

#ifdef VAULT_VSTRING_STRICT_FORMATTING
    #define VSTRING_FORMAT(format_string, ...) VString(0, format_string, __VA_ARGS__)
    #define VSTRING_ARGS(format_string, ...) true, format_string, __VA_ARGS__
    #define VSTRING_COPY(literal_string) VString(literal_string)
#else
    #define VSTRING_FORMAT(format_string, ...) VString(format_string, __VA_ARGS__)
    #define VSTRING_ARGS(format_string, ...) format_string, __VA_ARGS__
    #define VSTRING_COPY(literal_string) VString((char*)literal_string) // call non-formatting ctor
#endif

// The following macros define the proper formatting directives for the basic POD types,
// and VString constructors for converting those types to strings. For convenience, we
// also define a boolean string constructor.
// My advice and practice in Vault code:
// - %s (char*), and %c (char) are pervasively well-understood and should be used as is. Often %d as well.
// - The other formatters are provided here and should usually be used by name for clarity, except where
//   their presence makes code less understandable because of verbosity.
// For example:
// VSTRING_FORMAT("Hello, %s.", p->getName().chars());                          <-- No need to avoid using %s for (char*) values.
// VSTRING_FORMAT("Answer is %d.", 42);                                         <-- No need to avoid using %d for int values.
// VSTRING_FORMAT("Vector size is " VSTRING_FORMATTER_SIZE, vec.size());        <-- Useful because size_t formatting is sketchy.
// VSTRING_FORMAT("My 32-bit value is " VSTRING_FORMATTER_S32, myVs32Value);    <-- Useful because 32-bit int formatter may depend on compiler mode.
// VSTRING_FORMAT("Jumbo 64-bit size is " VSTRING_FORMATTER_S64, myVs64Value);  <-- Useful because 64-bit int formatter may depend on compiler mode.
// VSTRING_FORMAT("Result dimensions: %lld*%lld*%lld", x, y, z);                <-- Code would be less readable if VSTRING_FORMATTER_S64 were used 3x here, but this is not totally correct on unusual 64-bit platforms.

#define VSTRING_FORMATTER_INT       "%d"
#define VSTRING_FORMATTER_UINT      "%u"
#define VSTRING_FORMATTER_LONG      "%ld"
#define VSTRING_FORMATTER_ULONG     "%lu"
#ifdef VCOMPILER_MSVC
    #define VSTRING_FORMATTER_SIZE  "%Iu"   // VC++ libraries do not conform to IEEE1003.1 here.
#else
    #define VSTRING_FORMATTER_SIZE  "%zu"
#endif
#define VSTRING_FORMATTER_S8        "%hhd"  // Note: %hhd is not universally supported; converting value to other type may be better.
#define VSTRING_FORMATTER_U8        "%hhu"  // Note: %hhu is not universally supported; converting value to other type may be better.
#define VSTRING_FORMATTER_S16       "%hd"
#define VSTRING_FORMATTER_U16       "%hu"

#ifdef Vx32_IS_xINT /* Don't redefine if types are same; else form is untested environment. */
#define VSTRING_FORMATTER_S32       VSTRING_FORMATTER_INT
#define VSTRING_FORMATTER_U32       VSTRING_FORMATTER_UINT
#else
#define VSTRING_FORMATTER_S32       VSTRING_FORMATTER_LONG
#define VSTRING_FORMATTER_U32       VSTRING_FORMATTER_ULONG
#endif

#ifdef Vx64_IS_xINT /* Don't redefine if types are same; else form is normal environment. */
#define VSTRING_FORMATTER_S64       VSTRING_FORMATTER_INT
#define VSTRING_FORMATTER_U64       VSTRING_FORMATTER_UINT
#else
#define VSTRING_FORMATTER_S64       "%lld"
#define VSTRING_FORMATTER_U64       "%llu"
#endif

#define VSTRING_FORMATTER_FLOAT     "%f"
#define VSTRING_FORMATTER_DOUBLE    "%lf"
#define VSTRING_FORMATTER_PTR       "%p"

// Consider these macros a public API for declaring strings that are built by converting integers
// of various sizes, etc. This is preferable to specifying the proper formatting directives manually.
#define VSTRING_INT(n)      VSTRING_FORMAT(VSTRING_FORMATTER_INT, n)    ///< Creates a string by formatting an int value.
#define VSTRING_UINT(n)     VSTRING_FORMAT(VSTRING_FORMATTER_UINT, n)   ///< Creates a string by formatting an unsigned int value.
#define VSTRING_LONG(n)     VSTRING_FORMAT(VSTRING_FORMATTER_LONG, n)   ///< Creates a string by formatting a long value.
#define VSTRING_ULONG(n)    VSTRING_FORMAT(VSTRING_FORMATTER_ULONG, n)  ///< Creates a string by formatting an unsigned long value.
#define VSTRING_SIZE(z)     VSTRING_FORMAT(VSTRING_FORMATTER_SIZE, z)   ///< Creates a string by formatting a size_t value.
#define VSTRING_S8(n)       VSTRING_FORMAT(VSTRING_FORMATTER_S8, n)     ///< Creates a string by formatting an 8-bit int value.
#define VSTRING_U8(n)       VSTRING_FORMAT(VSTRING_FORMATTER_U8, n)     ///< Creates a string by formatting an unsigned 8-bit int value.
#define VSTRING_S16(n)      VSTRING_FORMAT(VSTRING_FORMATTER_S16, n)    ///< Creates a string by formatting a 16-bit int value.
#define VSTRING_U16(n)      VSTRING_FORMAT(VSTRING_FORMATTER_U16, n)    ///< Creates a string by formatting an unsigned 16-bit int value.
#define VSTRING_S32(n)      VSTRING_FORMAT(VSTRING_FORMATTER_S32, n)    ///< Creates a string by formatting a 32-bit int value.
#define VSTRING_U32(n)      VSTRING_FORMAT(VSTRING_FORMATTER_U32, n)    ///< Creates a string by formatting an unsigned 32-bit int value.
#define VSTRING_S64(n)      VSTRING_FORMAT(VSTRING_FORMATTER_S64, n)    ///< Creates a string by formatting a 64-bit int value.
#define VSTRING_U64(n)      VSTRING_FORMAT(VSTRING_FORMATTER_U64, n)    ///< Creates a string by formatting an unsigned 64-bit int value.
#define VSTRING_FLOAT(n)    VSTRING_FORMAT(VSTRING_FORMATTER_FLOAT, n)  ///< Creates a string by formatting a float value.
#define VSTRING_DOUBLE(n)   VSTRING_FORMAT(VSTRING_FORMATTER_DOUBLE, n) ///< Creates a string by formatting a double value.
#define VSTRING_PTR(p)      VSTRING_FORMAT(VSTRING_FORMATTER_PTR, p)    ///< Creates a string by formatting a pointer as hexadecimal preceded by 0x.
#define VSTRING_BOOL(b)     VString(b ? "true" : "false")               ///< Creates a string from a boolean as the text "true" or "false".

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
class VString {
    public:

        /**
        Returns a reference to the read-only empty VString constant.
        */
        static const VString& EMPTY();
        /**
        Returns a reference to the read-only VString constant holding the platform-native line ending character(s).
        If we are running on DOS/Windows it will be 0x0D0A. If we are running on Unix (including Mac OS X) it will be 0x0A.
        This is normally what you should use if you need to explicitly reference the preferred text file line ending
        for the current platform. In the rare case where you need to use a particular format (for example, if writing
        a file with a platform- or format-specific style, then you could use one of the platform specific accessors
        that are defined below.
        */
        static const VString& NATIVE_LINE_ENDING();
        /**
        Returns a reference to the read-only VString constant holding the Unix-style 0x0A line ending character.
        This is also what we consider the "native" line ending on Mac OS X.
        */
        static const VString& UNIX_LINE_ENDING();
        /**
        Returns a reference to the read-only VString constant holding the classic Mac Classic-style 0x0D line ending character.
        Note that this is different from our "native" Unix-style line ending on Mac OS X.
        */
        static const VString& MAC_CLASSIC_LINE_ENDING();
        /**
        Returns a reference to the read-only VString constant holding the DOS-style 0x0D0A line ending characters.
        */
        static const VString& DOS_LINE_ENDING();

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
        "Copy Constructor" -- constructs from another VString.
        @param    s    the string to copy
        */
        VString(const VString& s);
        /**
        Constructs a string from a char. The explicit keyword is to
        prevent the previous VString s(n) meaning of preflight string
        to size "n" from compiling.
        @param    c    the char
        */
        explicit VString(char c);
        /**
        Constructs a string from a C string.
        Note that if strict formatting is off, the param is not marked const so that
        we avoid ambiguous linkage vs. the const param in the vararg ctor below.
        @param    s    pointer to C string to copy
        */
#ifdef VAULT_VSTRING_STRICT_FORMATTING
        VString(const char* s);
#else /* non-strict formatting */
        //lint -e1776 Converting a string literal to char * is not const safe [OK: Intentionally non-const to disambiguate API; function treats parameter as const.]
        VString(char* s);
#endif /* VAULT_VSTRING_STRICT_FORMATTING */

#ifdef VAULT_VARARG_STRING_FORMATTING_SUPPORT
        /**
        Constructs a string by sprintf-like formatting.
        @param    dummy         not used, but distinguishes this fn signature from the one above;
                                important to use a type that literal strings can't auto convert to,
                                so compiler will always detect lack of dummy parameter
        @param    formatText    the format text
        @param    ...           varargs to be formatted
        */
#ifdef VAULT_VSTRING_STRICT_FORMATTING
        VString(Vs8 dummy, const char* formatText, ...);
#else /* non-strict formatting */
        VString(const char* formatText, ...);
#endif /* VAULT_VSTRING_STRICT_FORMATTING */

#endif /* VAULT_VARARG_STRING_FORMATTING_SUPPORT */

        /**
        Constructs from a "wide" string, converting from UTF-16/32 to our internal UTF-8.
        @param    ws    the wide string to copy
        */
        VString(const std::wstring& ws);

#ifdef VAULT_QT_SUPPORT
        /**
        Constructs a string from a QString.
        @param    s    the Qstring to copy
        */
        VString(const QString& s);
#endif

#ifdef VAULT_BOOST_STRING_FORMATTING_SUPPORT
        /**
        Constructs a string from a boost::format.
        @param fmt the formatter
        */
        VString(const boost::format& fmt);
#endif

#ifdef VAULT_CORE_FOUNDATION_SUPPORT
        /**
        Constructs a string from a CFStringRef.
        @param    s    the CFStringRef to copy
        */
        VString(const CFStringRef& s);
#endif
        /**
        Constructs a string from a Unicode code point (a single character value).
        @param    cp    the code point to use
        */
        VString(const VCodePoint& cp);

        /**
        Destructor.
        */
        ~VString();

        /**
        Basic assignment operator.
        @param    s    the string to copy
        */
        VString& operator=(const VString& s);
        /**
        Assign from a pointer to VString.
        @param    s    the string pointer to copy
        */
        VString& operator=(const VString* s);

#ifdef VAULT_QT_SUPPORT
        /**
        Assign from a QString.
        @param    s    the QString to copy
        */
        VString& operator=(const QString& s);
#endif

#ifdef VAULT_BOOST_STRING_FORMATTING_SUPPORT
        /**
        Assign from a boost::format.
        @param fmt the formatter
        */
        VString& operator=(const boost::format& fmt);
#endif

#ifdef VAULT_CORE_FOUNDATION_SUPPORT
        /**
        Assign from CFStringRef.
        @param    s    the CFStringRef to copy
        */
        VString& operator=(const CFStringRef& s);
#endif
        /**
        Assigns from a Unicode code point (a single character value).
        @param    cp    the code point to use
        */
        VString& operator=(const VCodePoint& cp);

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
        Assigns the string from a "wide" string, converting from UTF-16/32 to our internal UTF-8.
        @param    ws    the wide string to copy
        */
        VString& operator=(const std::wstring& ws);
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

#ifndef Vx32_IS_xINT /* don't redefine if types are same */
        /**
        Assigns the string from a Vs32.
        @param    i    the integer value to be formatted
        */
        VString& operator=(Vs32 i);
#endif /* not Vx32_IS_xINT */

        /**
        Assigns the string from a Vu64.
        @param    i    the integer value to be formatted
        */
        VString& operator=(Vu64 i);

#ifndef Vx64_IS_xINT /* don't redefine if types are same */
        /**
        Assigns the string from a Vs64.
        @param    i    the integer value to be formatted
        */
        VString& operator=(Vs64 i);
#endif /* not Vx64_IS_xINT */

        /**
        Assigns the string from a VDouble.
        @param    d    the double value to be formatted
        */
        VString& operator=(VDouble d);


        /**
        Creates a string from this string + (appending) a char.
        @param    c     the char to append
        */
        VString operator+(const char c) const;
        /**
        Creates a string from this string + (appending) a (const char*) buffer.
        @param    s     the string to append
        */
        VString operator+(const char* s) const;
        /**
        Creates a string from this string + (appending) "wide" string, converting it from UTF-16/32 to our internal UTF-8.
        @param    ws    the wide string to append
        */
        VString operator+(const std::wstring& ws) const;
        /**
        Creates a string from this string + (appending) another string.
        @param    s     the string to append
        */
        VString operator+(const VString& s) const;

#ifdef VAULT_BOOST_STRING_FORMATTING_SUPPORT
        /**
        Creates a string from this string + (appending) a boost format string.
        @param fmt the formatter to append
        */
        VString operator+(const boost::format& fmt) const;
#endif
        /**
        Creates a string from this string + (appending) a unicode code point.
        @param    cp    the code point to append
        */
        VString operator+(const VCodePoint& cp) const;

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
        Appends a "wide" string to the string, converting it from UTF-16/32 to our internal UTF-8..
        @param    ws   pointer to the wide string to copy
        */
        VString& operator+=(const std::wstring& ws);

#ifdef VAULT_BOOST_STRING_FORMATTING_SUPPORT
        /**
        Appends to the string from a boost::format.
        @param fmt the formatter to append
        */
        VString& operator+=(const boost::format& fmt);
#endif
        /**
        Appends to the string from a Unicode code point (a single character value).
        @param    cp    the code point to use
        */
        VString& operator+=(const VCodePoint& cp);

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
#ifndef Vx32_IS_xINT /* don't redefine if types are same */
        /**
        Appends to the string from a Vs32.
        @param    i    the integer value to be formatted
        */
        VString& operator+=(Vs32 i);
#endif /* not Vx32_IS_xINT */
        /**
        Appends to the string from a Vu64.
        @param    i    the integer value to be formatted
        */
        VString& operator+=(Vu64 i);
#ifndef Vx64_IS_xINT /* don't redefine if types are same */
        /**
        Appends to the string from a Vs64.
        @param    i    the integer value to be formatted
        */
        VString& operator+=(Vs64 i);
#endif /* not Vx64_IS_xINT */
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
        
        typedef VStringIterator<VString&> iterator;
        typedef VStringIterator<const VString&> const_iterator;
        typedef VStringIterator<VString&> reverse_iterator;
        typedef VStringIterator<const VString&> const_reverse_iterator;
        
        iterator begin();
        const_iterator begin() const;
        iterator end();
        const_iterator end() const;

        reverse_iterator rbegin();
        const_reverse_iterator rbegin() const;
        reverse_iterator rend();
        const_reverse_iterator rend() const;

#ifdef VAULT_VARARG_STRING_FORMATTING_SUPPORT
        /**
        Formats the string by sprintf-like formatting.
        @param    formatText    the format text
        @param    ...            varargs to be formatted
        */
        void format(const char* formatText, ...);
#endif

        /**
        Inserts the specified code point into the string at the
        specified iterator position, moving the remaining part of
        the string later in the buffer.
        Thus the length of the string increases by the length of
        the code point.
        To append, you are better off using operator+=.
        Note that the iterator position does not change during
        this insert.
        @param  cp          the code point to insert
        @param  position    the position to insert at
        */
        void insert(const VCodePoint& cp, const VString::iterator& position);
        /**
        Inserts the specified string into the string at the
        specified iterator position, moving the remaining part of
        the string later in the buffer.
        Thus the length of the string increases by the length of
        the string being inserted
        To append, you are better off using operator+=.
        Note that the iterator position does not change during
        this insert.
        @param  s           the string to insert
        @param  position    the position to insert at
        */
        void insert(const VString& s, const VString::iterator& position);
        /**
        Inserts the specified character into the string at the
        specified iterator position, moving the remaining part of
        the string later in the buffer.
        Thus the length of the string increases by 1.
        To append, you are better off using operator+=.
        Note that the iterator position does not change during
        this insert.
        @param  c           the character to insert
        @param  position    the position to insert at
        */
        void insert(char c, const VString::iterator& position);

        /**
        Inserts the specified code point into the string at the
        specified offset (default is at the front of the string),
        moving the remaining part of the string later in the buffer.
        Thus the length of the string increases by 1 character.
        To append, you are better off using operator+=.
        @param  cp      the code point to insert
        @param  offset  the buffer location to insert in front of
        */
        void insert(const VCodePoint& cp, int offset = 0);
        /**
        Inserts the specified string into the string at the
        specified offset (default is at the front of the string),
        moving the remaining part of the string later in the buffer.
        Thus the length of the string increases by the length of
        the string being inserted.
        To append, you are better off using operator+=.
        @param  s       the string to insert
        @param  offset  the location to insert in front of
        */
        void insert(const VString& s, int offset = 0);
        /**
        Inserts the specified character into the string at the
        specified offset (default is at the front of the string),
        moving the remaining part of the string later in the buffer.
        Thus the length of the string increases by 1 character.
        To append, you are better off using operator+=.
        @param  c       the char to insert
        @param  offset  the location to insert in front of
        */
        void insert(char c, int offset = 0);

        /**
        Returns the number of code points in the string. Note that this is
        not the same as the number of bytes.
        @return the number of code points in the string
        */
        int getNumCodePoints() const;
        /**
        Returns the string length in bytes. Note that this is not the same
        as the number of code points.
        @return the string length in bytes
        */
        int length() const;
        /**
        Truncates the string to specified number of code points; if the string
        is already that length or less, nothing happens.
        @param  maxNumCodePoints  the number of code points to truncate to
        */
        void truncateCodePoints(int maxNumCodePoints);
        /**
        Truncates the string to specified length; if the string is already
        that length or less, nothing happens.
        @param  maxLength  the length to truncate to
        */
        void truncateLength(int maxLength);
        /**
        Returns true if the string length is zero.
        @return true if the string length is zero
        */
        bool isEmpty() const;
        /**
        Returns true if the string length is non-zero.
        @return true if the string length is non-zero
        */
        bool isNotEmpty() const;
        /**
        Returns the character at the specified index.
        If the index is out of range, a VException is thrown.
        @param  i   the index (0 to length-1)
        @return the character
        */
        VChar at(int i) const;
        /**
        Returns the character at the specified index.
        If the index is out of range, a VException is thrown.
        @param  i   the index (0 to length-1)
        @return the character
        */
        VChar operator[](int i) const;
        /**
        Returns a reference to the char at the specified index, which
        can be assigned to.
        If the index is out of range, a VException is thrown.
        @param  i   the index (0 to length-1)
        @return the character
        */
        char& operator[](int i);
        /**
        Returns the char at the specified index.
        If the index is out of range, a VException is thrown.
        @param  i   the index (0 to length-1)
        @return the char
        */
        char charAt(int i) const;
        /**
        Coerces the string to a C string, for use with APIs that take
        a const char* parameter.
        In fact, this coercion returns the string buffer pointer.
        @return the char buffer pointer
        */
        operator const char*() const;
        /**
        Returns the string as a C string, for use with APIs that take
        a const char* parameter. The result is a const pointer, so the
        caller cannot modify the string. In fact, this coercion returns
        the string buffer pointer. This is the same as operator const char*,
        but is explicit. You need an explicit method like this when passing
        to an API that does not have sufficient type information, such as
        vararg calls like printf and sprintf.

        Example:    \c    printf("string='%s'\n", myString->chars());

        In the example above, simply passing myString would result in
        garbage output, because printf would not know how to treat
        myString other than as a pointer value.
        @return the char buffer pointer
        */
        const char* chars() const;
        
        /**
        Returns a "wide" string built from the VString, converting from our
        internal UTF-8 to UTF-16 code units. One code unit per wchar_t element.
        @return the wstring formed as UTF16 code units
        */
        std::wstring toUTF16() const;

#ifdef VAULT_QT_SUPPORT
        /**
        Returns a QString built from the VString.
        @return the QString
        */
        QString qstring() const;
#endif

#ifdef VAULT_CORE_FOUNDATION_SUPPORT
        /**
        Returns a CFStringRef built from the VString. The returned
        CFStringRef must be released at some point by the caller since it is a
        new object.
        @return the CFStringRef
        */
        CFStringRef cfstring() const;
#endif

        /**
        Returns true if this string is equal to the specified string,
        ignoring case, using strcmp semantics.
        @param  s   the string to compare with
        @return true if the strings are equal, case-insensitive
        */
        bool equalsIgnoreCase(const VString& s) const;
        /**
        Returns true if this string is equal to the specified C string,
        ignoring case, using strcmp semantics.
        @param  s   the C string to compare with
        @return true if the strings are equal, case-insensitive
        */
        bool equalsIgnoreCase(const char* s) const;
        /**
        Returns the comparison value of this string and the supplied
        string, using strcmp semantics.
        @param  s   the string to compare with
        @return <0, 0, or >0, depending on how the strings compare
        */
        int compare(const VString& s) const;
        /**
        Returns the comparison value of this string and the supplied
        C string, using strcmp semantics.
        @param  s   the C string to compare with
        @return <0, 0, or >0, depending on how the strings compare
        */
        int compare(const char* s) const;
        /**
        Returns the comparison value of this string and the supplied
        string, ignoring case, using strcmp semantics.
        @param  s   the string to compare with
        @return <0, 0, or >0, depending on how the strings compare
        */
        int compareIgnoreCase(const VString& s) const;
        /**
        Returns the comparison value of this string and the supplied
        C string, ignoring case, using strcmp semantics.
        @param  s   the C string to compare with
        @return <0, 0, or >0, depending on how the strings compare
        */
        int compareIgnoreCase(const char* s) const;
        /**
        Returns true if this string starts with the specified string.
        @param  s   the string to compare with
        @return true if this string starts with the specified string
        */
        bool startsWith(const VString& s) const;
        /**
        Returns true if this string starts with the specified string (ignoring case).
        @param  s   the string to compare with
        @return true if this string starts with the specified string (ignoring case)
        */
        bool startsWithIgnoreCase(const VString& s) const;
        /**
        Returns true if this string starts with the specified code point.
        @param  cp   the code point to compare with
        @return true if this string starts with the specified code point
        */
        bool startsWith(const VCodePoint& cp) const;
        /**
        Returns true if this string starts with the specified char.
        @param  c   the char to compare with
        @return true if this string starts with the specified char
        */
        bool startsWith(char c) const;
        /**
        Returns true if this string ends with the specified string.
        @param  s   the string to compare with
        @return true if this string ends with the specified string
        */
        bool endsWith(const VString& s) const;
        /**
        Returns true if this string ends with the specified string (ignoring case).
        @param  s   the string to compare with
        @return true if this string ends with the specified string (ignoring case)
        */
        bool endsWithIgnoreCase(const VString& s) const;
        /**
        Returns true if this string ends with the specified code point.
        @param  c   the char to compare with
        @return true if this string ends with the specified code point
        */
        bool endsWith(const VCodePoint& cp) const;
        /**
        Returns true if this string ends with the specified char.
        @param  c   the char to compare with
        @return true if this string ends with the specified char
        */
        bool endsWith(char c) const;
        /**
        Returns an iterator pointing to the first occurrence of the specified
        code point within the string.
        @param  cp  the code point to search for
        @return an iterator pointing to the found code point, or end() if not found
        */
        VString::const_iterator find(const VCodePoint& cp) const;
        VString::iterator find(const VCodePoint& cp);
        /**
        Returns an iterator pointing to the first occurrence of the specified
        code point within the string, bounded by the specified start and end
        iterator positions.
        @param  cp              the code point to search for
        @param  startPosition   the start iterator position to search;
                                    begin() means search from the start of the string
        @param  endPosition     the end iterator position to search;
                                    end() means search to the end of the string
        @return an iterator pointing to the found code point, or end() if not found
        */
        VString::const_iterator find(const VCodePoint& cp, const VString::const_iterator& startPosition, const VString::const_iterator& endPosition) const;
        VString::iterator find(const VCodePoint& cp, const VString::iterator& startPosition, const VString::iterator& endPosition);
        /**
        Returns the index of the first occurrence of the specified character.
        @param  c           the character to search for
        @param  fromIndex   index in this string to start the search from
        @return the index where the character was found, or -1 if not found
        */
        int indexOf(char c, int fromIndex = 0) const;
        /**
        Returns the index of the first occurrence of the specified character,
        using a case-insensitive comparison.
        @param  c           the character to search for
        @param  fromIndex   index in this string to start the search from
        @return the index where the character was found, or -1 if not found
        */
        int indexOfIgnoreCase(char c, int fromIndex = 0) const;
        /**
        Returns the index of the first occurrence of the specified string.
        @param  s           the string to look for
        @param  fromIndex   index in this string to start the search from
        @return the index where the string was found, or -1 if not found
        */
        int indexOf(const VString& s, int fromIndex = 0) const;
        /**
        Returns the index of the first occurrence of the specified string,
        using a case-insensitive comparison.
        @param    s            the string to look for
        @param    fromIndex    index in this string to start the search from
        @return the index where the string was found, or -1 if not found
        */
        int indexOfIgnoreCase(const VString& s, int fromIndex = 0) const;
        /**
        Returns the index of the last occurrence of the specified character.
        @param  c           the character to search for
        @param  fromIndex   index in this string to start the backward search from,
                            with -1 indicating a search from the end
        @return the index where the character was found, or -1 if not found
        */
        int lastIndexOf(char c, int fromIndex = -1) const;
        /**
        Returns the index of the last occurrence of the specified character,
        using a case-insensitive comparison.
        @param  c           the character to search for
        @param  fromIndex   index in this string to start the backward search from,
                            with -1 indicating a search from the end
        @return the index where the character was found, or -1 if not found
        */
        int lastIndexOfIgnoreCase(char c, int fromIndex = -1) const;
        /**
        Returns the index of the last occurrence of the specified string.
        @param  s           the string to look for
        @param  fromIndex   index in this string to start the backward search from,
                            with -1 indicating a search from the end
        @return the index where the string was found, or -1 if not found
        */
        int lastIndexOf(const VString& s, int fromIndex = -1) const;
        /**
        Returns the index of the last occurrence of the specified string,
        using a case-insensitive comparison.
        @param  s           the string to look for
        @param  fromIndex   index in this string to start the backward search from,
                            with -1 indicating a search from the end
        @return the index where the string was found, or -1 if not found
        */
        int lastIndexOfIgnoreCase(const VString& s, int fromIndex = -1) const;
        /**
        Returns true if the specified range of this string matches the
        specified range of the specified string. If the characters in
        the ranges do not match, or if either range goes past the end
        of either string, then the result is false.

        @param  thisIndex       the start index (0 to this this->length()-1) in this string to start the match
        @param  otherString     the string to match
        @param  otherIndex      the start index (0 to other otherString.length()-1) in the other string to start the match
        @param  regionLength    the number of characters to compare
        @param  caseSensitive   true if the comparison should be case-sensitive
        @return true if the ranges match
        */
        bool regionMatches(int thisIndex, const VString& otherString, int otherIndex, int regionLength, bool caseSensitive = true) const;
        /**
        Returns true if the specified character exists in this string.
        @param  c           the character to search for
        @param  fromIndex   index in this string to start the search from
        @return true if the character was found
        */
        bool contains(char c, int fromIndex = 0) const;
        /**
        Returns true if the specified character exists in this string,
        using a case-insensitive match.
        @param  c           the character to search for
        @param  fromIndex   index in this string to start the search from
        @return true if the character was found
        */
        bool containsIgnoreCase(char c, int fromIndex = 0) const;
        /**
        Returns true if the specified string exists in this string.
        @param  s           the string to look for
        @param  fromIndex   index in this string to start the search from
        @return true if the string was found
        */
        bool contains(const VString& s, int fromIndex = 0) const;
        /**
        Returns true if the specified string exists in this string,
        using a case-insensitive match.
        @param  s           the string to look for
        @param  fromIndex   index in this string to start the search from
        @return true if the string was found
        */
        bool containsIgnoreCase(const VString& s, int fromIndex = 0) const;
        /**
        Replaces every occurrence of the specified search string with the supplied
        replacement string. Returns the number of replacements performed, which may
        be zero.
        @param  searchString        the string to search for
        @param  replacementString   the string to replace the search string with
        @param  caseSensitiveSearch true if the search match should be case-sensitive
        @return the number of replaced occurrences
        */
        int replace(const VString& searchString, const VString& replacementString, bool caseSensitiveSearch = true);
        /**
        Replaces every occurrence of the specified search string with the supplied
        replacement string. Returns the number of replacements performed, which may
        be zero.
        @param  searchChar          the character to search for
        @param  replacementChar     the character to replace the search character with
        @param  caseSensitiveSearch true if the search match should be case-sensitive
        @return the number of replaced occurrences
        */
        int replace(const VCodePoint& searchChar, const VCodePoint& replacementChar, bool caseSensitiveSearch = true);

        /**
        Folds the string to lower case using tolower().
        */
        void toLowerCase();
        /**
        Folds the string to upper case using toupper().
        */
        void toUpperCase();
        /**
        Parses the string as an integer. The range must fall within the signed
        integer range (typically 32 bits, but possibly more, and theoretically
        less). The allowed format is ([+|-][0*][0-9]). That is, a leading
        plus or minus sign is allowed, as are leading zeroes, followed by digits.
        If the string is empty, the value is zero. If the string contains an invalid
        character sequence or evaluates to an out-of-range integer, VRangeException
        is thrown. This function is similar in purpose to atoi().
        */
        int parseInt() const;
        /**
        Parses the string as a Vs64 integer. The range must fall within the 64-bit signed
        integer range. The allowed format is ([+|-][0*][0-9]). That is, a leading
        plus or minus sign is allowed, as are leading zeroes, followed by digits.
        If the string is empty, the value is zero. If the string contains an invalid
        character sequence, VRangeException
        is thrown. This function is similar in purpose to atoi().
        */
        Vs64 parseS64() const;
        /**
        Parses the string as a Vu64 integer. The range must fall within the 64-bit unsigned
        integer range. The allowed format is ([+][0*][0-9]). That is, a leading
        plus sign is allowed, as are leading zeroes, followed by digits.
        If the string is empty, the value is zero. If the string contains an invalid
        character sequence, VRangeException
        is thrown. This function is similar in purpose to atoi().
        */
        Vu64 parseU64() const;
        /**
        Parses the string as a VDouble integer. The string must conform to the specification
        of strtod() given by ISO C and POSIX IEEE 1003.1. In the case of an illegal string,
        VRangeException is thrown. This function is similar in purpose to (and my rely on)
        sscanf() using the "%lf" format. An empty string is deemed to have the value 0.0.
        */
        VDouble parseDouble() const;
        /**
        Sets the character at the specified index to the specified value.
        If the index is out of range, a VException is thrown.
        @param  i   the index (0 to length-1)
        @param  c   the new character value
        */
        void set(int i, const VChar& c);
        /**
        Copies a substring of this string to a target string. The start index
        is inclusive, that is it is the index of the first character copied;
        zero means to copy from the start of the string. The stop index is
        exclusive, that is it is one past the index of the last character
        copied; the default value of -1 means to copy all the way to the end
        of the string. Thus with the defaults you get the whole string.
        @param  toString    the string to copy the specified substring into
        @param  startIndex  index of the first char to copy, inclusive
        @param  endIndex    index of the last char to copy, exclusive (end-start is the length)
        */
        void getSubstring(VString& toString, int startIndex/* = 0*/, int endIndex = -1) const;
        void getSubstring(VString& toString, VString::const_iterator rangeStart, VString::const_iterator rangeEnd) const;
        /**
        Makes a substring of this string in place (contrast with getSubstring(),
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
        @param  startIndex  index of the first char to copy, inclusive
        @param  endIndex    index of the last char to copy, exclusive (end-start is the length)
        */
        void substringInPlace(int startIndex/* = 0*/, int endIndex = -1);
        /**
        Splits the string into pieces using a specified delimiter character.
        Trailing empty strings are omitted in the output.
        The intent is to behave similarly to Java String.split().
        @param  result                  this string vector is cleared and then fill with the result
        @param  delimiter               the character that delimits the split points
        @param  limit                   if non-zero, the max number of result items; if the string
                                            has more elements than that, the trailing part of the
                                            string is collapsed into one element (including delimiters)
        @param  stripTrailingEmpties    if true, any empty strings at the end of the resulting
                                            list are discarded (this is the Java String.split() behavior)
        */
        void split(VStringVector& result, const VCodePoint& delimiter, int limit = 0, bool stripTrailingEmpties = true) const;
        /**
        Convenience version of split() that returns the vector. Will likely incur copy overhead
        compared to the non-returning version, so use the other version in time-critical code.
        @param  delimiter               the character that delimits the split points
        @param  limit                   if non-zero, the max number of result items; if the string
                                            has more elements than that, the trailing part of the
                                            string is collapsed into one element (including delimiters)
        @param  stripTrailingEmpties    if true, any empty strings at the end of the resulting
                                            list are discarded (this is the Java String.split() behavior)
        @return a vector of split result strings
        */
        VStringVector split(const VCodePoint& delimiter, int limit = 0, bool stripTrailingEmpties = true) const;
        /**
        Strips leading and trailing whitespace from the string.
        Whitespace as implemented here is defined as ASCII byte
        values <= 0x20 as well as 0x7F.
        */
        void trim();
        /**
        Copies the string's chars to the specified buffer, plus a null terminator byte; the
        number of bytes copied (including null terminator) is limited to the specified
        buffer size. That is, if bufferSize is N, and this string's length is greater than
        (N-1), some characters will not be copied. The target buffer always gets null terminated.
        @param  toBuffer    the char buffer to copy into
        @param  bufferSize  the size of the target buffer; limits size of resulting copied string
        */
        void copyToBuffer(char* toBuffer, int bufferSize) const;
        /**
        Sets the string by copying a number of characters from the source buffer.
        @param  fromBuffer  the char buffer to copy from
        @param  startIndex  the offset in the buffer to start from, inclusive (0 starts at first character)
        @param  endIndex    the offset in the buffer to stop at, exclusive (end-start is the length,
                                thus strlen(fromBuffer) will copy to the last character).
        */
        void copyFromBuffer(const char* fromBuffer, int startIndex, int endIndex);
        /**
        Sets the string by copying a C string from the source buffer. Same as copyFromBuffer(b, 0, strlen(b));
        @param  fromBuffer  the char buffer to copy from
        */
        void copyFromCString(const char* fromBuffer);
        /**
        Copies the string's chars to the specified Pascal string buffer, using
        the Pascal string format (length byte plus data). The caller is responsible
        for making sure that the buffer is big enough to hold 1 + this->length()
        bytes, up to a maximum of 256 total bytes.
        @param  pascalBuffer    the Pascal string buffer to copy into
        */
        void copyToPascalString(char* pascalBuffer) const;
        /**
        Sets the string by copying a Pascal string from the source buffer.
        @param  pascalBuffer    the Pascal string buffer to copy from
        */
        void copyFromPascalString(const char* pascalBuffer);
        /**
        Sets the string from a "four character code". The input is simply a 32-bit integer
        whose 4 bytes are treated as ASCII characters (most significant byte becomes the
        first character of the string, etc.). An exception is thrown if any of the
        input bytes is zero, since that would cause a null terminator byte to be part of the
        string's character data.
        */
        void setFourCharacterCode(Vu32 fourCharacterCode);
        /**
        Returns the "four character code" represented by the string. The first four characters
        of the string are used to compose a 32-bit integer (the first character of the string
        becomes the most significant byte, etc.). If the string has fewer than 4 characters,
        the result is padded out to 4 characters with spaces (0x20).
        */
        Vu32 getFourCharacterCode() const;

#ifdef VAULT_VARARG_STRING_FORMATTING_SUPPORT
        /**
        Vararg format method used by the sprintf method, and also available
        if you have a vararg API that needs to then format a string.
        @param  formatText  the format text
        @param  args        the argument list
        */
        void vaFormat(const char* formatText, va_list args);
#endif

        /**
        Ensures that the buffer is big enough to hold the specified string length.
        If preflight needs to reallocate the buffer and is unable to do so, a
        VException is thrown.
        @param  stringLength    the length of the string that will need to fit in the buffer
        */
        void preflight(int stringLength);
        /**
        Simulates what happens if preflight() fails to allocate the buffer
        due to lack of memory. This is used by the unit test code for platforms
        that we have not been able to get to fail by passing a huge buffer size
        to preflight().
        */
        void preflightWithSimulatedFailure();
        /**
        Returns the string's char buffer pointer. This method should only be used
        in special circumstances by code that is also calling preflight() and
        postflight() to manage the buffer. The pointer is non-const, for the
        purpose of allowing the caller to manipulate the buffer under the
        preflight/postflight rules.
        @return the buffer pointer
        */
        char* buffer();
        /**
        Returns the string's buffer pointer as a Vu8 pointer, which is a type
        directly compatible with the various stream data reading APIs. So this
        method lets the caller avoid a reinterpret_cast that is often otherwise
        needed when doing stream reads into the string data buffer. This method
        should only be used in special circumstances by code that is also calling
        preflight() and postflight() to manage the buffer. The returned pointer
        is non-const, for the purpose of allowing the caller to manipulate the
        buffer under the preflight/postflight rules.
        @return the buffer pointer
        */
        Vu8* getDataBuffer();
        /**
        Returns the string's buffer pointer as a const Vu8 pointer, which is a type
        directly compatible with the various stream data writing APIs. So this
        method lets the caller avoid a reinterpret_cast that is often otherwise
        needed when doing stream writes from the string data buffer.
        @return the buffer pointer
        */
        const Vu8* getDataBufferConst() const;
        /**
        Transfers ownership of the string's buffer to the caller and sets the string
        to empty (with no buffer). This is a way of extracting a char buffer from a
        VString such that the VString can be destructed and the caller retains the
        buffer. If the VString is an empty string such that it doesn't have a heap
        buffer, this function will create and return a buffer containing a null-terminated
        C string, so you do not need to check for a NULL function result; however,
        it is more efficient if you avoid calling this function when the VString is
        an empty string.
        @return a buffer pointer, which is now owned by the caller and no longer
                    referenced by the VString object (which is now an "empty" string)
        */
        char* orphanDataBuffer();
        /**
        Syncs the internal data to the specified length, setting the length
        and writing a null terminator into the buffer. Again, this should only
        be used in special circumstances by callers who are using preflight()
        and buffer() to do something with the buffer that cannot be done via
        the normal public API.
        */
        void postflight(int stringLength);

        friend inline bool operator==(const VString& s1, const VString& s2);
        friend inline bool operator==(const VString& s1, const char* s2);
        friend inline bool operator==(const char* s1, const VString& s2);
        friend inline bool operator==(const VString& s, char c);

        friend inline bool operator!=(const VString& s1, const VString& s2);
        friend inline bool operator!=(const VString& s1, const char* s2);
        friend inline bool operator!=(const char* s1, const VString& s2);
        friend inline bool operator!=(const VString& s, char c);

        friend inline bool operator<(const VString& s1, const VString& s2);
        friend inline bool operator<(const VString& s1, const char* s2);
        friend inline bool operator<(const char* s1, const VString& s2);

        friend inline bool operator<=(const VString& s1, const VString& s2);
        friend inline bool operator<=(const VString& s1, const char* s2);
        friend inline bool operator<=(const char* s1, const VString& s2);

        friend inline bool operator>=(const VString& s1, const VString& s2);
        friend inline bool operator>=(const VString& s1, const char* s2);
        friend inline bool operator>=(const char* s1, const VString& s2);

        friend inline bool operator>(const VString& s1, const VString& s2);
        friend inline bool operator>(const VString& s1, const char* s2);
        friend inline bool operator>(const char* s1, const VString& s2);

    private:

        void _setLength(int stringLength);
        Vs64 _parseSignedInteger() const;
        Vu64 _parseUnsignedInteger() const;

        /** Asserts if any invariant is broken. */
        void _assertInvariant() const;

#ifdef VAULT_VARARG_STRING_FORMATTING_SUPPORT
        /**
        Determines the length of a string if it were to be sprintf'd into
        a buffer. Used by VString to preflight buffers that will be used
        with sprintf.
        @param  formatText  the format text
        @param  args        the argument list
        */
        static int _determineSprintfLength(const char* formatText, va_list args);
#endif

        /**
        This is where we do the conversion and assignment for all APIs that
        create a VString from a "wide" string, which is in UTF-16 form. We
        convert to our internal UTF-8.
        */
        void _assignFromUTF16WideString(const std::wstring& utf16WideString);

#ifdef VAULT_CORE_FOUNDATION_SUPPORT
        void _assignFromCFString(const CFStringRef& s);
#endif

        /*
        New Vault 4.0 feature: SSO (small string optimization).
        Prior to 4.0, VString already optimized empty strings, avoiding heap allocation for that case, and dealing
        with a few special cases around not having a buffer. It also took care to allocate needed heap space in
        chunks so as to avoid excessive re-allocation for strings that grew repeatedly.
        With 4.0, the SSO feature means that for small strings ( <= 6 characters in a 32-bit build, <= 14 characters
        in a 64-bit build) it can store the string data internally inside the VString without allocating a heap buffer,
        and without adding any new size overhead to VString itself. In short, the "buffer size" and "buffer pointer"
        instance variables used to manage the heap buffer have their space inside VString re-purposed for storing
        the characters of the short string. If the string gets too large, then heap allocation is required as before.
        
        Internally, we use a union to overlay the data, and have an internal flag in the union, mUsingInternalBuffer,
        that tells us which buffer (and which part of the union) is in effect. To make most efficient use of space
        and avoid adding overhead, I have carefully examined how the compilers lay out and align the data, and I have
        ordered the union data to avoid bloat. If not for that concern, I would have put mStringLength, mNumCodePoints,
        and mUsingInternalBuffer outside the union since they apply to both parts of the union; however, that would have
        an the effect on union alignment on 64-bit of wasting several bytes. So instead, those instance variables
        are overlaid in both parts of the union, or put in the first part, but only the ones named in the first ("mI")
        part are referenced in the code.
        */
        
        // The internal buffer lengths are carefully chosen to fit within the existing object footprint's unused space.
        // 32-bit and 64-bit builds have different alignment/padding, so they have different amounts of unused space.
        // For testing, you can set the size to 1 to prevent use of the internal buffer other than for empty strings.
        #ifdef VCOMPILER_64BIT
            #define VSTRING_INTERNAL_BUFFER_SIZE 15
        #else
            #define VSTRING_INTERNAL_BUFFER_SIZE 7
        #endif

        // Internal low-level utility functions for bookkeeping the union data.
        
        /**
        Performs constructor-time initialization of the internal union data. This must be called by every constructor.
        We can't use initializer list syntax in the constructors because we are using structs and a union. It might
        be possible to use C++11 struct initializer syntax but that would not work in many or most compilers today.
        */
        void _construct();
        /**
        Returns a pointer to the (read-only) buffer that is in use (internal or external). It will never be null,
        even for an empty string which is usually in the internal buffer space. The buffer is immutable and so this
        API should be used in any const function that needs to read the buffer, or a non-const function when it is
        only reading the buffer. This function is named _get() for conciseness of code in the internal implementation.
        It means to obtain "getter" (read-only) access to the buffer.
        @return a valid pointer to an immutable null-terminated C string buffer, which may be the internal buffer or an external buffer
        */
        const char* _get() const { return mU.mI.mUsingInternalBuffer ? mU.mI.mInternalBuffer : mU.mX.mHeapBufferPtr; }
        /**
        Returns a pointer to the (writeable) buffer that is in use (internal or external). It will never be null,
        even for an empty string which is usually in the internal buffer space. The buffer is mutable and so this
        API should be used in any non-const function where it needs to write to the buffer. This function is
        named _set() for conciseness of code in the internal implementation. It means to obtain "setter" (read-write)
        access to the buffer.
        @return a valid pointer to a mutable null-terminated C string buffer, which may be the internal buffer or an external buffer
        */
        char* _set() { return mU.mI.mUsingInternalBuffer ? mU.mI.mInternalBuffer : mU.mX.mHeapBufferPtr; }
        /**
        Returns the buffer length of the buffer that is in use (internal or external). The length is the capacity of
        the buffer including a null terminator; another way of saying this that the length is one greater than the
        maximum "string length" the buffer can hold. The actual string length of the chars in the buffer at the time
        of this call cannot be inferred; length() or mI.mStringLength tells you that. This function is used internally
        primarily just to supply a limit on the vsnprintf() function as well as a couple of other error checks. The
        preflight() code is more explicit about the difference in internal and external buffers.
        @return the string buffer's total length
        */
        int _getBufferLength() const { return mU.mI.mUsingInternalBuffer ? VSTRING_INTERNAL_BUFFER_SIZE : mU.mX.mHeapBufferLength; }
        /**
        Computes the number of code points in the string and stores it in mI.mNumCodePoints; meant to be called internally, lazily,
        by getNumCodePoints() if mI.mNumCodePoints is -1. If the string is empty, the answer is 0; otherwise, the function iterates
        over the string buffer and counts the code points found.
        */
        void _determineNumCodePoints() const;

        // Finally, the union that defines our internal structure.
        union {

            // When mI.mUsingInternalBuffer == true, we use mI.mInternalBuffer to store the string data.
            struct {
                int         mStringLength;                                  ///< The length of the string; this is always valid.
                mutable int mNumCodePoints;                                 ///< The number of UTF-8 code points in the string; this is lazily calculated and a value of -1 means we must scan to calculate it. It gets set to 0 whenever mStringLength is set to 0, and set to -1 whenever mStringLength is set to something else.
                bool        mUsingInternalBuffer : 1;                       ///< True if mI.mInternalBuffer is valid, vs. mX.mHeapBufferPtr; this is always valid. It is a bitfield so that we don't have to explicitly do bit masking, and the debugger will display it correctly.
                int         mPadBits : 7;                                   ///< Unused bits in mI, overlaps with mX.mHeapBufferLength which is valid when mI.mUsingInternalBuffer is false.
                char        mInternalBuffer[VSTRING_INTERNAL_BUFFER_SIZE];  ///< The embedded character buffer, when mI.mUsingInternalBuffer is true; when mI.mUsingInternalBuffer is false, it is n/a and may appear to contain garbage.
            } mI; ///< Union part for overall bookkeeping, and internal SSO buffer space.
            
            // When mI.mUsingInternalBuffer == false, we use mX.mHeapBufferLength and mX.mHeapBufferPtr to store the string data.
            struct {
                int         mStringLength_Alias;                            ///< Do not use. Occupies same memory as mI.mStringLength, which should be used instead.
                int         mNumCodePoints_Alias;                           ///< Do not use. Occupies same memory as mI.mNumCodePoints, which should be used instead.
                int         mHeapBufferLength;                              ///< The size of the mHeapBufferPtr new[] allocated memory; when mI.mUsingInternalBuffer is true, it is n/a and may appear to contain garbage.
                char*       mHeapBufferPtr;                                 ///< Pointer to our new[] allocated memory; when mI.mUsingInternalBuffer is true, it is n/a and may appear to contain garbage.
            } mX; ///< Union part for heap-allocated buffer space.

        } mU; ///< Union for overlaying mI internal and mX external views of string buffer storage. mI.mStringLength and mI.mUsingInternalBuffer are always valid and authoritative.
        
        friend class VStringUnit; ///< Let it examine our internals under test.
};

inline bool operator==(const VString& lhs, const VString& rhs) { return ::strcmp(lhs, rhs) == 0; }      ///< Compares lhs and rhs for equality. @param    lhs    a string @param    rhs    a string @return true if lhs and rhs are equal according to strcmp()
inline bool operator==(const VString& lhs, const char* rhs) { return ::strcmp(lhs, rhs) == 0; }         ///< Compares lhs and rhs for equality. @param    lhs    a string @param    rhs    a C string @return true if lhs and rhs are equal according to strcmp()
inline bool operator==(const char* lhs, const VString& rhs) { return ::strcmp(lhs, rhs) == 0; }         ///< Compares lhs and rhs for equality. @param    lhs    a C string @param    rhs    a string @return true if lhs and rhs are equal according to strcmp()
inline bool operator==(const VString& s, char c) { return (s.length() == 1) && (s.charAt(0) == c); }    ///< Compares s and c for equality. @param    s    a string @param    c    a char @return true if s is one character in length, the character being c

inline bool operator!=(const VString& lhs, const VString& rhs) { return !operator==(lhs, rhs); }    ///< Compares lhs and rhs for inequality. @param    lhs    a string @param    rhs    a string @return true if lhs and rhs are not equal according to strcmp()
inline bool operator!=(const VString& lhs, const char* rhs) { return !operator==(lhs, rhs); }       ///< Compares lhs and rhs for inequality. @param    lhs    a string @param    rhs    a C string @return true if lhs and rhs are not equal according to strcmp()
inline bool operator!=(const char* lhs, const VString& rhs) { return !operator==(lhs, rhs); }       ///< Compares lhs and rhs for inequality. @param    lhs    a C string @param    rhs    a string @return true if lhs and rhs are not equal according to strcmp()
inline bool operator!=(const VString& lhs, char rhs) { return !operator==(lhs, rhs); }              ///< Compares lhs and rhs for inequality. @param    lhs    a string @param    rhs    a char @return true if lhs is NOT one character in length, or if its sole character is not rhs

inline bool operator<(const VString& lhs, const VString& rhs) { return ::strcmp(lhs, rhs) < 0; }    ///< Compares lhs and rhs. @param    lhs    a string @param    rhs    a string @return true if lhs < rhs according to strcmp()
inline bool operator<(const VString& lhs, const char* rhs) { return ::strcmp(lhs, rhs) < 0; }       ///< Compares lhs and rhs. @param    lhs    a string @param    rhs    a C string @return true if lhs < rhs according to strcmp()
inline bool operator<(const char* lhs, const VString& rhs) { return ::strcmp(lhs, rhs) < 0; }       ///< Compares lhs and rhs. @param    lhs    a C string @param    rhs    a string @return true if lhs < rhs according to strcmp()

inline bool operator<=(const VString& lhs, const VString& rhs) { return !operator>(lhs, rhs); } ///< Compares lhs and rhs. @param    lhs    a string @param    rhs    a string @return true if lhs <= rhs according to strcmp()
inline bool operator<=(const VString& lhs, const char* rhs) { return !operator>(lhs, rhs); }    ///< Compares lhs and rhs. @param    lhs    a string @param    rhs    a C string @return true if lhs <= rhs according to strcmp()
inline bool operator<=(const char* lhs, const VString& rhs) { return !operator>(lhs, rhs); }    ///< Compares lhs and rhs. @param    lhs    a C string @param    rhs    a string @return true if lhs <= rhs according to strcmp()

inline bool operator>=(const VString& lhs, const VString& rhs) { return !operator<(lhs, rhs); } ///< Compares lhs and rhs. @param    lhs    a string @param    rhs    a string @return true if lhs >= rhs according to strcmp()
inline bool operator>=(const VString& lhs, const char* rhs) { return !operator<(lhs, rhs); }    ///< Compares lhs and rhs. @param    lhs    a string @param    rhs    a C string @return true if lhs >= rhs according to strcmp()
inline bool operator>=(const char* lhs, const VString& rhs) { return !operator<(lhs, rhs); }    ///< Compares lhs and rhs. @param    lhs    a C string @param    rhs    a string @return true if lhs >= rhs according to strcmp()

inline bool operator>(const VString& lhs, const VString& rhs) { return operator<(rhs, lhs); }   ///< Compares lhs and rhs. @param    lhs    a string @param    rhs    a string @return true if lhs > rhs according to strcmp()
inline bool operator>(const VString& lhs, const char* rhs) { return operator<(rhs, lhs); }      ///< Compares lhs and rhs. @param    lhs    a string @param    rhs    a C string @return true if lhs > rhs according to strcmp()
inline bool operator>(const char* lhs, const VString& rhs) { return operator<(rhs, lhs); }      ///< Compares lhs and rhs. @param    lhs    a C string @param    rhs    a string @return true if lhs > rhs according to strcmp()

inline std::istream& operator>>(std::istream& in, VString& s) { s.readFromIStream(in); return in; }        ///< Creates the string by reading an istream. @param    in    the input stream @param    s    the string @return the input stream
inline std::ostream& operator<<(std::ostream& out, const VString& s) { return out << s.chars(); }   ///< Writes the string to an ostream. @param    out    the output stream @param s    the string @return the output stream
inline VString& operator<<(VString& s, std::istream& in) { s.appendFromIStream(in); return s; }     ///< Appends to the string by reading an istream. @param    s    the string @param    in    the input stream @return the string

inline VString& operator<<(VString& s, const char* in) { s += in; return s; }   ///< Appends to the string by copying a C string. @param    s    the string @param    in    the input C string buffer @return the string
inline VString& operator<<(VString& s, int i) { s += i; return s; }     ///< Appends to the string by copying an int as string. @param    s    the string @param    i    the int to append @return the string
inline VString& operator<<(VString& s, Vu8 i) { s += i; return s; }     ///< Appends to the string by copying a Vu8 as string. @param    s    the string @param    i    the Vu8 to append @return the string
inline VString& operator<<(VString& s, Vs8 i) { s += i; return s; }     ///< Appends to the string by copying a Vs8 as string. @param    s    the string @param    i    the Vs8 to append @return the string
inline VString& operator<<(VString& s, Vu16 i) { s += i; return s; }    ///< Appends to the string by copying a Vu16 as string. @param    s    the string @param    i    the Vu16 to append @return the string
inline VString& operator<<(VString& s, Vs16 i) { s += i; return s; }    ///< Appends to the string by copying a Vs16 as string. @param    s    the string @param    i    the Vs16 to append @return the string
inline VString& operator<<(VString& s, Vu32 i) { s += i; return s; }    ///< Appends to the string by copying a Vu32 as string. @param    s    the string @param    i    the Vu32 to append @return the string

#ifndef Vx32_IS_xINT /* don't redefine if types are same */
inline VString& operator<<(VString& s, Vs32 i) { s += i; return s; }    ///< Appends to the string by copying a Vs32 as string. @param    s    the string @param    i    the Vs32 to append @return the string
#endif /* not Vx32_IS_xINT */

inline VString& operator<<(VString& s, Vu64 i) { s += i; return s; }    ///< Appends to the string by copying a Vu64 as string. @param    s    the string @param    i    the Vu64 to append @return the string

#ifndef Vx64_IS_xINT /* don't redefine if types are same */
inline VString& operator<<(VString& s, Vs64 i) { s += i; return s; }    ///< Appends to the string by copying a Vs64 as string. @param    s    the string @param    i    the Vs64 to append @return the string
#endif /* not Vx64_IS_xINT */

inline VString& operator<<(VString& s, VDouble f) { s += f; return s; } ///< Appends to the string by copying a VDouble as string. @param    s    the string @param    f    the VDouble to append @return the string

#endif /* vstring_h */
