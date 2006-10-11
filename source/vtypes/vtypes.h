/*
Copyright c1997-2006 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.5
http://www.bombaydigital.com/
*/

#ifndef vtypes_h
#define vtypes_h

/** @file */

/**

    @defgroup vtypes Vault Data Types

    The file vtypes.h and the platform-specific version of the file it
    includes, vtypes_platform.h, define a core set of basic data types and
    preprocessor macros that facilitate platform-neutral code. Without these
    definitions, you would have to cope with the various quirks and
    behaviors of different compilers, libraries, and processors.
    
    <h2>Data Types</h2>
    
    <h3>Integers</h3>
    
    Whenever possible, you should just use int. If you are willing to assume
    that the world is now 32-bit or better, then you can treat int as
    "a signed integer that holds about +/- 2 billion", and if your data fits
    into that description, int is good because it is signed and general.
    However, there are cases where you need to use specific sized integer,
    perhaps unsigned, and the types Vs8, Vu8, Vs16, Vu16, Vs32, Vu32, Vs64,
    and Vu64 provide signed and unsigned definitions for 8, 16, 32, and 64
    bit integers. For example, if you are doing binary stream i/o, you'd better
    be certain of the exact number of bytes used in the stream to represent each
    particular integer value; thus, VBinaryIOStream speaks in these types and
    does not let you read and write the more ambiguous "int". The definition
    of a 64-bit integer is different across compilers, hence it is unsafe for you
    to use "long long" and expect it to work; thus the existence of Vs64 and Vu64,
    which work correctly even with MSVC++ 6.
    
    <h3>Floating-Point</h3>
    
    Because the single-precision "float" type in the language is ambigous and
    essentially dangerous, vtypes defines VDouble, which uses double precision.
    There is also a VFloat type, but in general you should avoid it because of
    loss of precision and the aforementioned dangers.
    
    <h3>File Sizes</h3>
    
    If you want to avoid writing code that breaks when encountering large files,
    it is best to use the type VFSize to represent file and stream sizes. Even in
    today's systems, some users work with files whose size exceeds what can be
    represented in 32 bits. A video file of 3GB is not that uncommon. You need
    64 bits, and that's how VFSize is defined. Alternatively, you could just use
    Vs64 (or even Vu64), and that would work just as well, although it reads as
    less obvious what its purpose is. The somewhat standard type size_t is
    useful, but it will not be 64 bits on most systems today, so the Vault does
    not use it to define stream-related sizes, in favor of a uniform use of
    VFSize.
    
    <h3>Collection Sizes</h3>
    
    The type VSizeType is defined as a uniform way of defining a type for
    the size of an STL collection. This is often a good clean way of defining
    the loop control variable for an STL iteration for-loop. You cannot just
    use "int" for those situations because it's the wrong type and strong compiler
    type-checking will complain; and it seems awkward to use a specific STL
    template-defined size_type because if you decide to use a different
    STL collection type, your control variable type should change, too. I'm not
    totally satisfied with this, but so far this seems like the best option.
    
    <h3>Strings</h3>
    
    See VString for the string class used everywhere in the Vault.
    
    <h3>Date and Time</h3>
    
    See VInstant and its relatives VDate and VTimeOfDay for the date and time
    representations used everywhere in the Vault.
    
    <h3>Byte Order</h3>
    
    Some compilers' libraries provide macros for invoking host-to-network order
    and network-to-host order byte swapping, such that they do nothing on "big
    endian" machines ("network byte order" means big endian in this terminology).
    However, not all such libraries provide these macros and functions, or don't
    provide them for 64-bit values, so they are defined here when necessary,
    along with the raw byte swapping functions VbyteSwap16(), VbyteSwap32(), and
    VbyteSwap64() that can be called directly when the host order is not relevant.
    In general, you should use VBinaryIOStream to read and write data in a
    byte-order-neutral fashion. In rare cases you may want to make sure your data
    is in host or network byte order, in which case you would use one of the
    macros htons, ntohs, htonl, ntohl, hton64, ntoh64, or their upper case
    relatives, all of which do the right thing -- which is "nothing" if the host
    order is network order. Finally, if you really need to do byte swapping
    explicitly, you can call VbyteSwap16(), VbyteSwap32(), or VbyteSwap64(), which
    swap bytes between network and host order (the two directions happen to be
    symmetric for big- and little-endian).
    
    <h3>Constants</h3>
    
    There are a couple of things to note about defining and using constants.
    
    Because of certain compilers' limitations (namely MSVC++ 6), you cannot
    declare a 64-bit constant in the normal way if you want it to compile
    cross-platform. That is, "1LL" is unfortunately not going to work. Provided
    here is a macro CONST_S64 that will work with all compilers. So use
    "CONST_S64(1)" instead, to declare a 64-bit constant whose value is 1.
    Use CONST_U64 for unsigned values.
    
    MSVC++ 6 also is incompatible with the normal way of declaring class static
    constants (e.g., "const int kMyConstant = 42;"). The Vault provides a macro
    called "CLASS_CONST" that allows you to declare class static int constants
    in a way that will compile with MSVC++ 6: "CLASS_CONST(int, kMyConstant, 42);".
    As of Code Vault 2.5, I am no longer using this macro, but it's still defined
    in case you need it (please just dump the piece of junk MSVC++ 6 compiler and
    get the freely downloadable and reasonably decent VC++ 8 instead).
    
    There are constants defined for the miniumum and maximum possible values of each
    integer data type: V_MIN_S8, V_MAX_S8, V_MAX_U8, V_MIN_S16, V_MAX_S16, V_MAX_U16,
    V_MIN_32, V_MAX_S32, V_MAX_U32, V_MIN_64, V_MAX_S64, V_MAX_U64.
    These are defined as Vs64 values so that they are always valid and interchangeable.
    You should use these in place of the typical POSIX macros because not only are
    the POSIX macros not available on all platforms, but they aren't necessarily
    provided for 64-bit values; and you should use them instead of the standard C++
    function templates may not be available at all.
    
    <h3>Miscellaneous</h3>
    
    There are macros defined to wrap min(), max(), abs(), and fabs(), such that they
    will still work if the standard C++ function templates are not available: V_MIN,
    V_MAX, V_ABS, V_FABS. You should use these if you want to be sure your code compiles
    cross-platform.
    
    A macro ASSERT_INVARIANT is defined that calls "this->assertInvariant()" if you
    are building in debug mode. This allows you to define a method assertInvariant()
    in any class, and call this macro to test your invariants. The rule of thumb is
    that you should test the invariants at the end of every (public) constructor,
    test the invariants on entry to each method, test the invariants on exit of any
    non-const method. See "C++ FAQs" item 224. The Vault uses this mechanism itself in
    several of its classes that are suited to invariant testing.
    
*/
    
/**
    @addtogroup vtypes
    @{
*/

/*
First we include the user-editable header file that configures
the desired Vault features/behavior.
*/
#include "vconfigure.h"

/*
Next we include the few platform-specific definitions we've defined.
Which actual file this refers to will depend on the include path
set up for this platform build.
*/
#include "vtypes_platform.h"

#include <vector>

/*
We choose to define just the basic specific-sized data types. Most
code should just use 'int' wherever a 32-bit signed value is sufficient.
But if you need a specific size (such as when doing stream i/o or
talking to an external API that uses a specific sized type, these
are our official definitions.
*/
typedef signed char         Vs8;    ///< Signed 8-bit integer.
typedef unsigned char       Vu8;    ///< Unsigned 8-bit integer.

typedef signed short        Vs16;   ///< Signed 16-bit integer.
typedef unsigned short      Vu16;   ///< Unsigned 16-bit integer.

typedef signed long         Vs32;   ///< Signed 32-bit integer.
typedef unsigned long       Vu32;   ///< Unsigned 32-bit integer.

#ifndef VCOMPILER_MSVC_6_CRIPPLED /* MSVC++ 6 hacks for this are defined in Win32 platform header */
typedef signed long long    Vs64;   ///< Signed 64-bit integer.
typedef unsigned long long  Vu64;   ///< Unsigned 64-bit integer.
#endif

typedef float               VFloat;     ///< Single-precision floating-point number.
typedef double              VDouble;    ///< Double-precision floating-point number.
typedef Vs64                VFSize;     ///< Container for file or stream sizes. The purpose is to prevent 32-bit limits from creeping into APIs and source code.
typedef size_t              VSizeType;  ///< loop index variable of correct type for STL iteration

#ifndef NULL
    #define NULL 0  ///< Definition of NULL in compiler environments that don't already define it.
#endif

/*
Because MSVC++ 6 uses nonstandard notation, we use a macro whenever we need
to define a 64-bit constant value.

For signed, instead of:
12345678901234567890LL or 0x0123456789ABCDEFLL
we must use:
CONST_S64(12345678901234567890) or CONST_S64(0x0123456789ABCDEF)

For unsigned, instead of:
12345678901234567890ULL or 0x0123456789ABCDEFULL
we must use:
CONST_U64(12345678901234567890) or CONST_U64(0x0123456789ABCDEF)
*/
#ifndef VCOMPILER_MSVC_6_CRIPPLED /* MSVC++ 6 hacks for this are defined in Win32 platform header */
    #define CONST_S64(s) /*lint -save -e961*/ s##LL    /*lint -restore*/ ///< Macro to declare a Vs64 constant in a way that works even in VC++ 6.
    #define CONST_U64(s) s##ULL    ///< Macro to declare a Vu64 constant in a way that works even in VC++ 6.
#endif

// Limit constants.
static const Vs64 V_MIN_S8  = CONST_S64(0xFFFFFFFFFFFFFF80); ///< Smallest signed 8-bit value
static const Vs64 V_MAX_S8  = CONST_S64(0x000000000000007F); ///< Largest signed 8-bit value
static const Vs64 V_MAX_U8  = CONST_S64(0x00000000000000FF); ///< Largest unsigned 8-bit value
static const Vs64 V_MIN_S16 = CONST_S64(0xFFFFFFFFFFFF8000); ///< Smallest signed 16-bit value
static const Vs64 V_MAX_S16 = CONST_S64(0x0000000000007FFF); ///< Largest signed 16-bit value
static const Vs64 V_MAX_U16 = CONST_S64(0x000000000000FFFF); ///< Largest unsigned 16-bit value
static const Vs64 V_MIN_S32 = CONST_S64(0xFFFFFFFF80000000); ///< Smallest signed 32-bit value
static const Vs64 V_MAX_S32 = CONST_S64(0x000000007FFFFFFF); ///< Largest signed 32-bit value
static const Vs64 V_MAX_U32 = CONST_S64(0x00000000FFFFFFFF); ///< Largest unsigned 32-bit value
static const Vs64 V_MIN_S64 = CONST_S64(0x8000000000000000); ///< Smallest signed 64-bit value
static const Vs64 V_MAX_S64 = CONST_S64(0x7FFFFFFFFFFFFFFF); ///< Largest signed 64-bit value
static const Vs64 V_MAX_U64 = CONST_S64(0xFFFFFFFFFFFFFFFF); ///< Largest unsigned 64-bit value

namespace vault {

/**
Byte-swaps a 16-bit (2-byte) integer either host-to-network order, or
network-to-host order (it's the same shuffling either way).

This function always swaps the byte order of the value, regardless of
whether native order matches network order. Its purpose is to perform
a swap. The caller must have already decided that a swap is desirable.

@param    a16BitValue    the original value
@return    the swapped value
*/
extern Vu16 VbyteSwap16(Vu16 a16BitValue);

/**
Byte-swaps a 32-bit (4-byte) integer either host-to-network order, or
network-to-host order (it's the same shuffling either way).

This function always swaps the byte order of the value, regardless of
whether native order matches network order. Its purpose is to perform
a swap. The caller must have already decided that a swap is desirable.

@param    a32BitValue    the original value
@return    the swapped value
*/
extern Vu32 VbyteSwap32(Vu32 a32BitValue);

/**
Byte-swaps a 64-bit (8-byte) integer either host-to-network order, or
network-to-host order (it's the same shuffling either way).

This function always swaps the byte order of the value, regardless of
whether native order matches network order. Its purpose is to perform
a swap. The caller must have already decided that a swap is desirable.

@param    a64BitValue    the original value
@return    the swapped value
*/
extern Vu64 VbyteSwap64(Vu64 a64BitValue);

/**
Byte-swaps a 32-bit (4-byte) float either host-to-network order, or
network-to-host order (it's the same shuffling either way).

This function always swaps the byte order of the value, regardless of
whether native order matches network order. Its purpose is to perform
a swap. The caller must have already decided that a swap is desirable.

@param    a32BitValue    the original value
@return    the swapped value
*/
extern VFloat VbyteSwapFloat(VFloat a32BitValue);

/**
Returns the amount of memory used by the process as reported by
some appropriate platform API. Note that this value may not necessarily
grow and shrink directly by the amount of each heap malloc/new operation,
because the underlying memory management library might keep free blocks
around for future use without returning them to the operating system.
This value is useful for general memory usage trend tracking, not
necessarily for detailed memory leak detection.
@return the number of bytes of memory used by the process, or zero if
        this API is not implemented on this platform
*/
extern Vs64 VgetMemoryUsage();

/**
Wrapper for memcpy using more precise and convenient types used within
Vault classes.
*/
inline void Vmemcpy(Vu8* to, Vu8* from, int length) { ::memcpy(to, (char*) from, (VSizeType) length); }

/**
Returns a pointer to a const static containing this platform's native
text file line ending bytes, and also sets the supplied byte count so the
caller (@see VTextIOStream) does not have the scan the string to determine
the length. The caller will use this information to write line endings
to text streams when asked to use the native form.
@param  numBytes to be filled in, will be the number of bytes of line
                ending chars
@return a pointer to the line ending string
*/
extern const Vu8* VgetNativeLineEnding(int& numBytes);

/**
A helper template function that both clears a vector and deletes each
object it holds a pointer to. This is the simplest way to delete all
contained objects held in a vector. The vector must hold pointers to
objects. See VLogger::shutdown for an example use.
*/
template <class T>
void vectorDeleteAll(std::vector<T*>& v)
    {
    for (std::vector<T*>::iterator i = v.begin(); i != v.end(); ++i)
        delete (*i);

    v.clear();
    }

} // namespace vault

#ifdef V_DEBUG
    #define ASSERT_INVARIANT() this->assertInvariant() ///< Macro to call this->assertInvariant() in debug mode only.
#else
    #define ASSERT_INVARIANT() ((void) 0) ///< Macro to call this->assertInvariant() in debug mode only.
#endif

/*
Because MSVC++ 6 does not support the C++ standard we must define a macro
for defining static class constants such as "const static int kFoo = 1;"
and doing something different in that compiler.
*/
#ifndef VCOMPILER_MSVC_6_CRIPPLED /* MSVC++ 6 hacks for this are defined in Win32 platform header */
    #define CLASS_CONST(type, name, init) static const type name = (init)    ///< Macro to declare a class static constant in a way that works even in VC++ 6.
#endif

/** @} */

/**
The V_ASSERT macro and Vassert function provide assertion and assertion breakpoint
support. Normally you should use the V_ASSERT macro instead of the compiler-specific
assert macro, and put a breakpoint in Vassert if you are in debug mode and want to
hit a breakpoint upon assertion failure. The macro is a no-op in a release build, so
it has no runtime cost in a release build.
@param    expression    the expression that if false will cause an assertion failure
@param    file        the name of the source file that is asserting
@param    line        the line number in the source file that is asserting
*/
extern void Vassert(bool expression, const char* file, int line);

#ifdef V_DEBUG
    // This inline wrapper eliminates function call overhead for successful assertions.
    inline void VassertWrapper(bool expression, const char* file, int line) { if (!expression) Vassert(expression, file, line); }
    #define V_ASSERT(expression) VassertWrapper(expression, __FILE__, __LINE__)
#else
    #define V_ASSERT(expression) ((void) 0)
#endif

// Uncomment this define to get a trace of static initialization through the macro below.
//#define V_DEBUG_STATIC_INITIALIZATION_TRACE 1

extern int Vtrace(const char* fileName, int lineNumber);
#ifdef V_DEBUG_STATIC_INITIALIZATION_TRACE
    #define V_STATIC_INIT_TRACE static int staticVtrace = Vtrace(__FILE__, __LINE__);
#else
    #define V_STATIC_INIT_TRACE
#endif

#endif /* vtypes_h */

