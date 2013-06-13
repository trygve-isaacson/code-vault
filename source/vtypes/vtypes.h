/*
Copyright c1997-2011 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.3
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
    provide them for all data types; so here we define a complete set of macros
    for all fundamental data types.

    In general, you should use VBinaryIOStream to read and write data in a
    byte-order-neutral fashion. In rare cases you may want to make sure your data
    is in host or network byte order, in which case you would use one of the
    V_BYTESWAP_* macros, all of which do the right thing -- which is "nothing" if
    the host order is network order. Finally, if you really need to do byte swapping
    explicitly, you can call one of the byte-swapping functions VbyteSwap16(),
    VbyteSwap32(), VbyteSwap64(), VbyteSwapFloat(), or VbyteSwapDouble() which
    swap bytes between network and host order (the two directions happen to be
    symmetric for big- and little-endian). These functions always swap the bytes,
    in contrast to the macros which are no-ops when running on a big-endian system.

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
Next we include the few platform-specific definitions we've defined.
Which actual file this refers to will depend on the include path
set up for this platform build.
*/
#include "vtypes_platform.h"

#include <vector>
#include <stdarg.h>
#include <vector>
#include <iostream>
#include <deque>
#include <map>
#include <limits>

/*
We choose to define just the basic specific-sized data types. Most
code should just use 'int' wherever a 32-bit signed value is sufficient.
But if you need a specific size (such as when doing stream i/o or
talking to an external API that uses a specific sized type, these
are our official definitions.
*/
typedef int8_t              Vs8;    ///< Signed 8-bit integer.
typedef uint8_t             Vu8;    ///< Unsigned 8-bit integer.

typedef int16_t             Vs16;   ///< Signed 16-bit integer.
typedef uint16_t            Vu16;   ///< Unsigned 16-bit integer.

typedef int32_t             Vs32;   ///< Signed 32-bit integer.
typedef uint32_t            Vu32;   ///< Unsigned 32-bit integer.

#ifndef VCOMPILER_MSVC_6_CRIPPLED /* MSVC++ 6 hacks for this are defined in the _win platform header */
typedef int64_t             Vs64;   ///< Signed 64-bit integer.
typedef uint64_t            Vu64;   ///< Unsigned 64-bit integer.
#endif

typedef float               VFloat;     ///< Single-precision floating-point number.
typedef double              VDouble;    ///< Double-precision floating-point number.
typedef Vs64                VFSize;     ///< Container for file or stream sizes. The purpose is to prevent 32-bit limits from creeping into APIs and source code.
typedef size_t              VSizeType;  ///< loop index variable of correct type for STL iteration

/*
These three defines are to future-proof against compile errors when overloading functions
that take various integer types. Which overloads are actually identical (and thus duplicates)
depends on the compile flags. Specifically, and in most 32- and 64-bit environments,
Vs32 = int32_t = int, so you can't have an overloaded function with both int and Vs32 versions.
But if you use an ILP64 model, then Vs64 = int64_t = int, in which case the error would be if
you overload a function with both int and Vs64 versions. I don't yet have an example to know
how to detect ILP64, and all others (e.g., 32-bit and LP64) have int32_t = int, so for now we
just define the first one all the time (Vs32 / Vu32 are int / unsigned int), and therefore
where we have int overloads (VString assignment and increment, VAssert variants) we don't define
the 32-bit overloads. If you use ILP64, make this define conditional on something appropriate,
and we'll avoid defining the 64-bit overloads. The definition of size_t can also conflict with
unsigned int if, as Microsoft does, the headers declare it as such a typedef.
An enhancement would be to auto-detect or more conditionally define these; for now all supported
environments use the Vx32_IS_xINT model, and V_SIZE_T_IS_UNSIGNED_INT is handled in the platform
header for Windows, so that's not necessary.
@see http://en.wikipedia.org/wiki/LP64#64-bit_data_models
*/
#define Vx32_IS_xINT /* Vs32 is int32_t is typedef of int. Correct for 32-bit, as well as 64-bit when using LLP64/IL32P64 (e.g. Microsoft X64/IA-64) or LP64/I32LP64 (e.g. Unix; everyone but Microsoft). The norm. */
//#define Vx64_IS_xINT /* Vs64 is int64_t is typedef of int. Correct for 64-bit when using ILP64 or SILP64. The oddball case. */
//#define V_SIZE_T_IS_UNSIGNED_INT /* size_t is typedef of unsigned int. This is defined in _win/vtypes_platform.h because in MS defines size_t this way, so overloading a function signature both ways won't work. */

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
static const size_t V_MAX_SIZE = std::numeric_limits<size_t>::max(); ///< Largest size_t value

namespace vault {

/**
Byte-swaps a 16-bit (2-byte) integer either host-to-network order, or
network-to-host order (it's the same shuffling either way).

This function always swaps the byte order of the value, regardless of
whether native order matches network order. Its purpose is to perform
a swap. The caller must have already decided that a swap is desirable.

@param  a16BitValue the original value
@return the swapped value
*/
extern Vu16 VbyteSwap16(Vu16 a16BitValue);

/**
Byte-swaps a 32-bit (4-byte) integer either host-to-network order, or
network-to-host order (it's the same shuffling either way).

This function always swaps the byte order of the value, regardless of
whether native order matches network order. Its purpose is to perform
a swap. The caller must have already decided that a swap is desirable.

@param  a32BitValue the original value
@return the swapped value
*/
extern Vu32 VbyteSwap32(Vu32 a32BitValue);

/**
Byte-swaps a 64-bit (8-byte) integer either host-to-network order, or
network-to-host order (it's the same shuffling either way).

This function always swaps the byte order of the value, regardless of
whether native order matches network order. Its purpose is to perform
a swap. The caller must have already decided that a swap is desirable.

@param  a64BitValue the original value
@return the swapped value
*/
extern Vu64 VbyteSwap64(Vu64 a64BitValue);

/**
Byte-swaps a 32-bit (4-byte) float either host-to-network order, or
network-to-host order (it's the same shuffling either way).

This function always swaps the byte order of the value, regardless of
whether native order matches network order. Its purpose is to perform
a swap. The caller must have already decided that a swap is desirable.

@param  a32BitValue the original value
@return the swapped value
*/
extern VFloat VbyteSwapFloat(VFloat a32BitValue);

/**
Byte-swaps a 64-bit (8-byte) double either host-to-network order, or
network-to-host order (it's the same shuffling either way).

This function always swaps the byte order of the value, regardless of
whether native order matches network order. Its purpose is to perform
a swap. The caller must have already decided that a swap is desirable.

@param  a64BitValue the original value
@return the swapped value
*/
extern VDouble VbyteSwapDouble(VDouble a64BitValue);

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
inline void Vmemcpy(Vu8* to, Vu8* from, int length) {
    ::memcpy(to, (char*) from, (VSizeType) length);
}

/**
Returns a pointer to a const static containing this platform's native
text file line ending bytes, and also sets the supplied byte count so the
caller (@see VTextIOStream) does not have the scan the string to determine
the length. The caller will use this information to write line endings
to text streams when asked to use the native form.
@param  numBytes    to be filled in, will be the number of bytes of line ending chars
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
void vectorDeleteAll(std::vector<T*>& v) {
    for (typename std::vector<T*>::iterator i = v.begin(); i != v.end(); ++i) {
        delete(*i);
    }

    v.clear();
}

/**
A helper template function that both clears a map and deletes each
value object it holds a pointer to. This is the simplest way to delete all
contained value objects held in a map. The map values must be "delete"-able
pointers, and there must not be multiple keys using the same value (which is
a perfectly legitimate way to use a map, so if you use the map that way, you
must manage that aspect of deletion; otherwise, a crash may occur when an
object is deleted twice). The keys are not deleted (presumed to not be pointers).
@param m a map<k,v*> where the map holds values that are pointers to deletable
    objects; on return, the map will have no entries, and delete will have been
    called on each entry's v pointer.
*/
template <class KEY_TYPE, class VALUE_TYPE>
void mapDeleteAllValues(std::map<KEY_TYPE, VALUE_TYPE*>& m) {
    for (typename std::map<KEY_TYPE, VALUE_TYPE*>::const_iterator i = m.begin(); i != m.end(); ++i) {
        delete(*i).second;
    }

    m.clear();
}

/**
A helper template function that both erases a map entry for a value, and also
deletes the value object it holds a pointer to. This is the simplest way to remove
one contained value object held in a map and delete the underlying heap object.
The map values must be "delete"-able pointers, and there must not be multiple keys
using the same value (which is a perfectly legitimate way to use a map, so if you
use the map that way, you must manage that aspect of deletion; otherwise, a crash
may occur when an object is deleted twice). The keys are not deleted (presumed to
not be pointers).
@param m a map<k,v*> where the map holds values that are pointers to deletable
    objects
@param key the key for the entry to be removed from the map; delete will be called
    on the value pointer in the entry if found.
*/
template <class KEY_TYPE, class VALUE_TYPE>
void mapDeleteOneValue(std::map<KEY_TYPE, VALUE_TYPE*>& m, KEY_TYPE key) {
    typename std::map<KEY_TYPE, VALUE_TYPE*>::iterator position = m.find(key);
    if (position != m.end()) {
        delete position->second; // Delete heap object pointed to by the value.
        (void) m.erase(position); // Remove the dead entry from the map.
    }
}

} // namespace vault

// The vtypes_platform.h header decided whether VBYTESWAP_NEEDED is defined.
#ifdef VBYTESWAP_NEEDED

    // Signed 16-bit actual swapping:
    #define V_BYTESWAP_HTON_S16_GET(x)      vault::VbyteSwap16((Vu16) x)
    #define V_BYTESWAP_NTOH_S16_GET(x)      vault::VbyteSwap16((Vu16) x)
    #define V_BYTESWAP_HTON_S16_IN_PLACE(x) ((x) = (vault::VbyteSwap16((Vu16) x)))
    #define V_BYTESWAP_NTOH_S16_IN_PLACE(x) ((x) = (vault::VbyteSwap16((Vu16) x)))
    // Unsigned 16-bit actual swapping:
    #define V_BYTESWAP_HTON_U16_GET(x)      vault::VbyteSwap16(x)
    #define V_BYTESWAP_NTOH_U16_GET(x)      vault::VbyteSwap16(x)
    #define V_BYTESWAP_HTON_U16_IN_PLACE(x) ((x) = (vault::VbyteSwap16(x)))
    #define V_BYTESWAP_NTOH_U16_IN_PLACE(x) ((x) = (vault::VbyteSwap16(x)))

    // Signed 32-bit actual swapping:
    #define V_BYTESWAP_HTON_S32_GET(x)      vault::VbyteSwap32((Vu32) x)
    #define V_BYTESWAP_NTOH_S32_GET(x)      vault::VbyteSwap32((Vu32) x)
    #define V_BYTESWAP_HTON_S32_IN_PLACE(x) ((x) = (Vs32)(vault::VbyteSwap32((Vu32) x)))
    #define V_BYTESWAP_NTOH_S32_IN_PLACE(x) ((x) = (Vs32)(vault::VbyteSwap32((Vu32) x)))
    // Unsigned 32-bit actual swapping:
    #define V_BYTESWAP_HTON_U32_GET(x)      vault::VbyteSwap32(x)
    #define V_BYTESWAP_NTOH_U32_GET(x)      vault::VbyteSwap32(x)
    #define V_BYTESWAP_HTON_U32_IN_PLACE(x) ((x) = (vault::VbyteSwap32(x)))
    #define V_BYTESWAP_NTOH_U32_IN_PLACE(x) ((x) = (vault::VbyteSwap32(x)))

    // Signed 64-bit actual swapping:
    #define V_BYTESWAP_HTON_S64_GET(x)      vault::VbyteSwap64((Vu64) x)
    #define V_BYTESWAP_NTOH_S64_GET(x)      vault::VbyteSwap64((Vu64) x)
    #define V_BYTESWAP_HTON_S64_IN_PLACE(x) ((x) = (vault::VbyteSwap64((Vu64) x)))
    #define V_BYTESWAP_NTOH_S64_IN_PLACE(x) ((x) = (vault::VbyteSwap64((Vu64) x)))
    // Unsigned 64-bit actual swapping:
    #define V_BYTESWAP_HTON_U64_GET(x)      vault::VbyteSwap64(x)
    #define V_BYTESWAP_NTOH_U64_GET(x)      vault::VbyteSwap64(x)
    #define V_BYTESWAP_HTON_U64_IN_PLACE(x) ((x) = (vault::VbyteSwap64(x)))
    #define V_BYTESWAP_NTOH_U64_IN_PLACE(x) ((x) = (vault::VbyteSwap64(x)))

    // VFloat (float) actual swapping:
    #define V_BYTESWAP_HTON_F_GET(x)        vault::VbyteSwapFloat(x)
    #define V_BYTESWAP_NTOH_F_GET(x)        vault::VbyteSwapFloat(x)
    #define V_BYTESWAP_HTON_F_IN_PLACE(x)   ((x) = (vault::VbyteSwapFloat(x)))
    #define V_BYTESWAP_NTOH_F_IN_PLACE(x)   ((x) = (vault::VbyteSwapFloat(x)))

    // VDouble (double) actual swapping:
    #define V_BYTESWAP_HTON_D_GET(x)        vault::VbyteSwapDouble(x)
    #define V_BYTESWAP_NTOH_D_GET(x)        vault::VbyteSwapDouble(x)
    #define V_BYTESWAP_HTON_D_IN_PLACE(x)   ((x) = (vault::VbyteSwapDouble(x)))
    #define V_BYTESWAP_NTOH_D_IN_PLACE(x)   ((x) = (vault::VbyteSwapDouble(x)))

#else

    // Signed 16-bit no-op non-swapping:
    #define V_BYTESWAP_HTON_S16_GET(x)      (x)
    #define V_BYTESWAP_NTOH_S16_GET(x)      (x)
    #define V_BYTESWAP_HTON_S16_IN_PLACE(x) ((void)0)
    #define V_BYTESWAP_NTOH_S16_IN_PLACE(x) ((void)0)
    // Unsigned 16-bit no-op non-swapping:
    #define V_BYTESWAP_HTON_U16_GET(x)      (x)
    #define V_BYTESWAP_NTOH_U16_GET(x)      (x)
    #define V_BYTESWAP_HTON_U16_IN_PLACE(x) ((void)0)
    #define V_BYTESWAP_NTOH_U16_IN_PLACE(x) ((void)0)

    // Signed 32-bit no-op non-swapping:
    #define V_BYTESWAP_HTON_S32_GET(x)      (x)
    #define V_BYTESWAP_NTOH_S32_GET(x)      (x)
    #define V_BYTESWAP_HTON_S32_IN_PLACE(x) ((void)0)
    #define V_BYTESWAP_NTOH_S32_IN_PLACE(x) ((void)0)
    // Unsigned 32-bit no-op non-swapping:
    #define V_BYTESWAP_HTON_U32_GET(x)      (x)
    #define V_BYTESWAP_NTOH_U32_GET(x)      (x)
    #define V_BYTESWAP_HTON_U32_IN_PLACE(x) ((void)0)
    #define V_BYTESWAP_NTOH_U32_IN_PLACE(x) ((void)0)

    // Signed 64-bit no-op non-swapping:
    #define V_BYTESWAP_HTON_S64_GET(x)      (x)
    #define V_BYTESWAP_NTOH_S64_GET(x)      (x)
    #define V_BYTESWAP_HTON_S64_IN_PLACE(x) ((void)0)
    #define V_BYTESWAP_NTOH_S64_IN_PLACE(x) ((void)0)
    // Unsigned 64-bit no-op non-swapping:
    #define V_BYTESWAP_HTON_U64_GET(x)      (x)
    #define V_BYTESWAP_NTOH_U64_GET(x)      (x)
    #define V_BYTESWAP_HTON_U64_IN_PLACE(x) ((void)0)
    #define V_BYTESWAP_NTOH_U64_IN_PLACE(x) ((void)0)

    // VFloat (float) no-op non-swapping:
    #define V_BYTESWAP_HTON_F_GET(x)        (x)
    #define V_BYTESWAP_NTOH_F_GET(x)        (x)
    #define V_BYTESWAP_HTON_F_IN_PLACE(x)   ((void)0)
    #define V_BYTESWAP_NTOH_F_IN_PLACE(x)   ((void)0)

    // VDouble (double) no-op non-swapping:
    #define V_BYTESWAP_HTON_D_GET(x)        (x)
    #define V_BYTESWAP_NTOH_D_GET(x)        (x)
    #define V_BYTESWAP_HTON_D_IN_PLACE(x)   ((void)0)
    #define V_BYTESWAP_NTOH_D_IN_PLACE(x)   ((void)0)

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

/*
Here is where we use a hierarchy of preprocessor symbols to control assertion behavior.
The hierarchy of user-definable symbols is:
  VAULT_ASSERTIONS_ENABLED
    V_ASSERT_INVARIANT_ENABLED
      V_ASSERT_INVARIANT_classname_ENABLED (for a given classname)

In vconfigure.h, you may define any or all of these symbols to 0 or 1 to force them off or on;
by default, V_ASSERT_ENABLED is on unless VAULT_ASSERTIONS_NOOP is defined; and the others inherit their state from
the level above.

Internally, we use the above symbols to define or leave undefined the following symbols:
  V_ASSERT_ACTIVE
    V_ASSERT_INVARIANT_ACTIVE
      V_ASSERT_INVARIANT_classname_ACTIVE (for a given classname)

This allows to the Vault code to simply #ifdef the existence of those symbols.

The following values for classname are implemented:
    VEXCEPTION
    VSTRING
    VDATE_AND_TIME
    VMESSAGE
    VMEMORYSTREAM
    VTEXTIOSTREAM
*/

// Do the VASSERT macros do anything if called?
#ifndef VAULT_ASSERTIONS_ENABLED
    #define V_ASSERT_ACTIVE
#else
    #if VAULT_ASSERTIONS_ENABLED == 0
        // don't define V_ASSERT_ACTIVE, thereby disabling the VASSERT macros
    #else
        #define V_ASSERT_ACTIVE
    #endif
#endif

// Does a "failed assertion" (e.g., VASSERT(false)) throws a VStackTraceException in addition to logging an error?
// Turned on by default here; define to 0 in vconfigure.h to turn it off.
#ifndef VAULT_ASSERTIONS_THROW
    #define V_ASSERT_THROWS_EXCEPTION
#else
    #if VAULT_ASSERTIONS_THROW == 0
        // don't define V_ASSERT_THROWS_EXCEPTION, thereby disabling throwing upon assertion failure
    #else
        #define V_ASSERT_THROWS_EXCEPTION
    #endif
#endif

// Does ASSERT_INVARIANT actually call this->_assertInvariant?
#ifdef V_ASSERT_INVARIANT_ENABLED
    #if V_ASSERT_INVARIANT_ENABLED == 1
        #define V_ASSERT_INVARIANT_ACTIVE
    #endif
#else
    #ifdef V_ASSERT_ACTIVE
        #define V_ASSERT_INVARIANT_ACTIVE
    #endif
#endif

/*
Now we can actually decide on the definition of ASSERT_INVARIANT().
It either calls this->_assertInvariant() or does nothing.
*/
#ifdef V_ASSERT_INVARIANT_ACTIVE
    #define ASSERT_INVARIANT() this->_assertInvariant() ///< Macro to call this->_assertInvariant().
#else
    #define ASSERT_INVARIANT() ((void) 0) ///< No-op.
#endif

// This is useful in some _assertInvariant() methods, to detect when there is a
// pointer that was already deallocated. In debug mode, VC++ generally sets pointers to
// this value upon deleting the pointer.
extern const void* const VCPP_DEBUG_BAD_POINTER_VALUE;

// Uncomment this define, or define it in your vconfigure.h to get a trace of static
// initialization through the macro below:
//#define V_DEBUG_STATIC_INITIALIZATION_TRACE 1

extern int Vtrace(const char* fileName, int lineNumber);
#ifdef V_DEBUG_STATIC_INITIALIZATION_TRACE
    #define V_STATIC_INIT_TRACE static int staticVtrace = Vtrace(__FILE__, __LINE__);
#else
    #define V_STATIC_INIT_TRACE
#endif

#define V_CONSTRAIN_MINMAX(n, minValue, maxValue) V_MAX(minValue, V_MIN(maxValue, n))

/**
VAutoreleasePool currently is defined to support memory management in Cocoa applications.
It is a no-op on other platforms. We declare an autorelease pool in VThread mains
and in unit tests to provide a proper autorelease environment for objects allocated
in those contexts. The constructor creates the pool and the destructor drains it.
If appropriate, you can drain the pool each time through a loop, while keeping the
pool itself around for a longer lifecycle.
*/
class VAutoreleasePool {
    public:
        VAutoreleasePool();
        ~VAutoreleasePool();
        void drain();
    private:
        void* mPool; // "void*" so as not to add a type dependency on includers.
};

/*
Memory leak tracking facility. See implementation in vmemorytracker.cpp.
The feature is compiled in, or not, based on this defined preprocessor symbol.
*/
#ifndef __OBJC__ /* Redefining new is not compatible with NSObject use of new keyword. So OC code will not see this feature. */
#ifdef VAULT_MEMORY_ALLOCATION_TRACKING_SUPPORT

void* operator new(size_t size, const char* file, int line);
void operator delete(void* p, const char* file, int line);
void operator delete(void* p) throw();
void* operator new[](size_t size, const char* file, int line);
void operator delete[](void* p, const char* file, int line);
void operator delete[](void* p) throw();
#define V_NEW new(__FILE__, __LINE__)
#define new V_NEW

class VTextIOStream;
class VString;
class VDuration;

class VMemoryTracker {
    public:

        /**
        You must disable memory tracking before main() ends; the easiest way is to
        declare one of these as a local variable in your main. The destructor disables
        memory tracking. You choose whether it's enabled at start as a constructor
        parameter. Tracking must not be enabled during static initialization.
        */
        VMemoryTracker(bool enableAtStart = false);
        ~VMemoryTracker();

        static void enable();                           ///< Turns on tracking. Subsequent allocations will be tracked.
        static void disable();                          ///< Turns off tracking. Subsequent allocations will not be tracked.
        static void reset();                            ///< Resets tracking. Tracked items are discarded. Enable state is not changed.
        static bool isEnabled();                        ///< Returns true if tracking is turned on.
        static void setOver(size_t newOver);            ///< Sets the size, above which each allocation is logged.
        static size_t getOver();                        ///< Returns the size, above which each allocation is logged.
        static void setUnder(size_t newOver);           ///< Sets the size, below which each allocation is logged.
        static size_t getUnder();                       ///< Returns the size, below which each allocation is logged.
        static void setLimit(int maxNumAllocations);    ///< Sets the max number of allocations tracked. 0 means no limit.
        static int  getLimit();                         ///< Returns the max number of allocations that will be tracked.
        static void setExpiration(const VDuration& d);  ///< Sets the duration from now when tracking will turn off. 0 means never.
        static Vs64 getExpirationTime();                ///< Returns the VInstant "raw value" when tracking will turn off. May be INFINITE_FUTURE. (Can't expose VInstant directly in this API.)
        static Vs64 getExpirationMilliseconds();        ///< Returns the number of ms tracking auto-disables after being enabled. (Can't expose VDuration directly in this API.)
        static void omitPointer(const void* p);         ///< Tells the tracker to forget a pointer, if it's currently tracked.
        static void enableCodeLocationCrawl(const VString& file, int line);
        static void disableCodeLocationCrawl(const VString& file, int line);
        static Vs64 getAllocationNumber();
        /**
        Prints a memory tracking report. Lots of options:
        @param label an optional string to bracket the report; if empty, boilerplate is used
        @param toLogger if true, the output will (also) be sent to VLOGGER_INFO
        @param toConsole if true, the output will (also) be sent to std::cout
        @param toStream if not NULL, the output will (also) be sent to this text output stream
        @param bufferLengthLimit if non-zero, dumped data buffers will have limited length
        @param showDetails if true, each items data is displayed in a full hex dump (bufferLength limit is ignored)
        @param performAnalysis if true, extra analysis is performed to collect related objects and thus better show
                                the "root" of a leaked graph of related objects
        */
        static void reportMemoryTracking(const VString& label, bool toLogger, bool toConsole, VTextIOStream* toStream, Vs64 bufferLengthLimit, bool showDetails, bool performAnalysis);
};

#endif /* VAULT_MEMORY_ALLOCATION_TRACKING_SUPPORT */
#endif /* __OBJC__ */

// Give app a chance to configure after us as well as before.
#define VCONFIGURE_AFTER_VTYPES
#include "vconfigure.h"
#undef VCONFIGURE_AFTER_VTYPES

#endif /* vtypes_h */
