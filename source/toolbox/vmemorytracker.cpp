/*
Copyright c1997-2011 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.3
http://www.bombaydigital.com/
*/

/** @file */

#include "vtypes.h"

#ifdef VAULT_MEMORY_ALLOCATION_TRACKING_SUPPORT

/*
Memory leak tracking facility.
THEORY OF OPERATION:

This entire facility is enabled by defining VAULT_MEMORY_ALLOCATION_TRACKING_SUPPORT in vconfigure.h.

In vtypes.h, we redefine "new" with a C preprocessor macro. Our version performs this replacement:

  new Foo  --->  new(__FILE__, __LINE__) foo

This means that code exposed to our macro (that is, code that includes vtypes.h) will call our
versions of operator new defined here:
  void* operator new(size_t size, const char* file, int line)
  void* operator new[](size_t size, const char* file, int line)

Any code not exposed to our macro will use the standard operator new, and those memory allocations
we will not see here.

We also define a replacement for the global operator delete and delete[].

Our new uses malloc(). Our delete uses free().

Our implementations keep track of each malloc'ed allocation, by storing an AllocationRecord in a map.
The map maps the (const void*) pointer value returned by malloc() to the (const AllocationRecord*)
holding our extra information.
In the delete, we remove the AllocationRecord from the map and delete it.

We protect our data structures with a mutex. However, for efficiency, we don't lock the mutex just
to check our enable flag. This flag is only changed by user's debugging command, and if it's off
we don't want to have any extra overhead. So our check of the flag is non-atomic, but we don't care.

There are several important things we do here to protect against infinite recursion and other
problems:
1. When locking our mutex, we must pass the global empty string, so that it does not
   itself allocate memory for the string buffer.
2. When reporting, we locally turn off allocation tracking, so that the reporting code
   itself does not mutate our data structures.
3. We must anticipate our delete being called for pointers we did not new. These might be pointers
   allocated when tracking was disabled, or by system libraries.
4. Here in the implementation file, we have to #undef new and V_NEW so that when we do memory allocation
   here we aren't recursing into our tracking code. We get the original global new.
5. Any code here that executes while holding our mutex, must be careful to guarantee that it does not
   invoke other code here that acquires the mutex (such as allocating via our new), because that will
   create a recursion deadlock (except on Windows where mutex locks allow recursion).
6. So far, it seems that we have to make sure we don't get involved at static initialization time or
   static termination time. Avoiding use at static init is easily achieved by requiring our feature to be
   explicitly turned on at runtime. Avoiding use at static termination is not so automatic. To make this
   easy, we define our interface class VMemoryTracker to disable tracking on destruction. The recommended
   procedure is to simply declare a VMemoryTracker on the stack in the application main() function. If
   it wants, it can enable tracking there, or let some later runtime command control it. When the main
   function ends, the destructor will turn off tracking before static termination time.

*/

#include "vmutex.h"
#include "vmutexlocker.h"
#include "vhex.h"
#include "vtextiostream.h"
#include "vlogger.h"
#include "vthread.h"

#undef V_NEW
#undef new

/**
This class defines the information we keep about each allocation we track.
In addition to the pointer, size, file, and line number supplied by our new macro, we also
assign an allocation number, which is incremented for each allocation. This allows us to
sort by allocation order when we report, and to distinguish unique allocations at the same
address.
*/
class AllocationRecord
    {
    public:
        AllocationRecord(void* pointer, bool isArray, size_t size, const char* file, int line, const char* stackCrawlInfo) :
            fAllocationNumber(0),
            fWhen(),
            fPointer(pointer),
            fIsArray(isArray),
            fSize(size),
            fFile(file),
            fLine(line),
            fStackCrawlInfo(stackCrawlInfo) {}
        ~AllocationRecord() // Note that we don't own fPointer or fFile.
            {
            delete [] fStackCrawlInfo;
            }

        mutable Vs64    fAllocationNumber;  ///< The unique sequential number of this allocation.
        VInstant        fWhen;              ///< Time of the allocation.
        void*           fPointer;           ///< The allocated pointer.
        bool            fIsArray;           ///< True if the pointer was allocated through the array operator new[]().
        size_t          fSize;              ///< The number of bytes allocated by the caller.
        const char*     fFile;              ///< The file name supplied by the __FILE__ macro.
        int             fLine;              ///< The line number supplied by the __LINE__ macro.
        const char*     fStackCrawlInfo;    ///< If not null, a char* (to be delete[]'ed) with stack crawl text.

        // Need operator< to suport STL sort(). We sort by fAllocationNumber.
        friend inline bool operator<(const AllocationRecord& r1, const AllocationRecord& r2);
    };

inline bool operator<(const AllocationRecord& r1, const AllocationRecord& r2) { return r1.fAllocationNumber < r2.fAllocationNumber; }

typedef std::map<const void*, const AllocationRecord*> AllocationMap;

/**
This class exists purely to allow a std::sort() on a vector of allocation records. If we just put pointers
to AllocationRecord in a vector, sort() will sort on the raw pointer values. So instead, we wrap the pointers
in this class, which can implement operator< correctly for our desired sort order.
*/
class AllocationRecordPtr
    {
    public:
        AllocationRecordPtr(const AllocationRecord* r) :
            fPtr(r) {}
        ~AllocationRecordPtr() {} // Note that we don't own fPtr.

        const AllocationRecord* fPtr; ///< Pointer to the allocation record we wrap.

        // Need operator< to suport STL sort(). We sort by fAllocationNumber.
        friend inline bool operator<(const AllocationRecordPtr& r1, const AllocationRecordPtr& r2);
    };

inline bool operator<(const AllocationRecordPtr& r1, const AllocationRecordPtr& r2) { return r1.fPtr->fAllocationNumber < r2.fPtr->fAllocationNumber; }

typedef std::vector<AllocationRecordPtr> AllocationPtrVector;

/**
We allow the user to specify certain lines of code that record a stack crawl along
with the allocation record; to manage this we just keep a vector of file+line
values for which this feature is turned on. Should be a very small set.
*/
class CodeLocation
    {
    public:
        CodeLocation(const VString& file, int line) :
            mFile(file),
            mLine(line) {}
        ~CodeLocation() {}

        VString mFile;
        int mLine;
    };

typedef std::vector<CodeLocation> CodeLocationVector;

// Private global variables.
static AllocationMap gAllocationMap; ///< The allocations we are currently tracking.
static CodeLocationVector gStackCrawlCodeLocations; ///< Code locations whose allocations record a stack crawl.
static bool   gTrackMemory = false; ///< The flag to enable or disable tracking new allocations.
static bool   gInsideLockedMutex = false; ///< True only when inside locker during reset or report code; prevents deletes there from recursive deadlock.
static Vs64   gNextAllocationNumber = 1; ///< The fAllocationNumber value of the next allocation.
static int    gMaxNumAllocations = 50000; ///< The max number of allocations we'll hold in our map. Desire: 5s max to produce report. Seems like it does about 10 per ms.
static int    gCurrentNumAllocations = 0; ///< The current number of allocations in our map.
static VDuration gExpirationDuration = 15 * VDuration::MINUTE(); ///< Whenever enabled(), we apply this to gExpirationTime.
static VInstant gExpirationTime; ///< The time at which when we will disable() tracking.
static VMutex gAllocationMapMutex("gAllocationMapMutex", true); // 2nd param MUST be true to prevent this mutex from logging (which would allocate memory).
static VString REPORT_LABEL("MEMORY REPORT");
// Track large memory allocations
static size_t gTrackAllocationssOver = 0;
static size_t gTrackAllocationssUnder = V_MAX_S32;

/**
Puts an item to the map. The allocation record must not be null.
*/
static void _putToMap(const void* p, const AllocationRecord* r)
    {
    VMutexLocker locker(&gAllocationMapMutex, VString::EMPTY());
    r->fAllocationNumber = gNextAllocationNumber++; // We do this manually so that AllocationRecord ctor doesn't also need to lock
    ++gCurrentNumAllocations;
    gAllocationMap[p] = r;
    }

/**
Removes an item from the map. It's presumed that it's in the map, but
it is harmless if not.
*/
static void _removeFromMap(const void* p)
    {
    VMutexLocker locker(&gAllocationMapMutex, VString::EMPTY());
    --gCurrentNumAllocations;
    gAllocationMap[p] = NULL;
    }

/**
Strips off everything before the file name in the supplied path. For example,
a string like "C:\path\to\a\file.cpp" becomes "file.cpp". Basically we just
look for the last slash or backslash and keep only what follows.
*/
static void _stripFileName(VString& path)
    {
    int lastSlash = path.lastIndexOf('\\');
    if (lastSlash == -1)
        lastSlash = path.lastIndexOf('/');
    if (lastSlash != -1)
        path.substringInPlace(lastSlash+1);
    }

/**
Walks backward from the end of the path and returns a pointer to the
part that is a file name. That is, the first char after the last
slash or backslash if there is one.
*/
static const char* _getFileNamePtr(const char* path)
    {
    int length = (int) ::strlen(path);
    // Walk backwards until we find a slash or backslash.
    int offset = length-1;
    while ((offset >= 0) && (path[offset] != '/') && (path[offset] != '\\'))
        --offset;

    return path + offset + 1;
    }

/**
Returns true if the specified file+line are enabled for stack crawl tracking.
*/
static bool _isCodeLocationCrawlEnabled(const char* file, int line)
    {
    // Fast exit if no code locations are crawl-enabled.
    if (gStackCrawlCodeLocations.size() == 0)
        return false;

    // Note that we can't use VString here because we are being called during memory allocation, and
    // a VString used here will itself allocate memory and we'll have infinite recursion. Use low-level
    // C char operations.
    const char* fileName = _getFileNamePtr(file);
    for (CodeLocationVector::const_iterator i = gStackCrawlCodeLocations.begin(); i != gStackCrawlCodeLocations.end(); ++i)
        {
        if (((*i).mFile == fileName) && ((*i).mLine == line))
            return true;
        }

    return false;
    }

/**
Returns an item from the map. The result might be null.
*/
static const AllocationRecord* _getFromMap(const void* p)
    {
    VMutexLocker locker(&gAllocationMapMutex, VString::EMPTY());
    return gAllocationMap[p];
    }

VMemoryTracker::VMemoryTracker(bool enableAtStart)
    {
    if (enableAtStart)
        VMemoryTracker::enable();
    }

VMemoryTracker::~VMemoryTracker()
    {
    VMemoryTracker::disable();
    VMemoryTracker::reset();
    }

// static
void VMemoryTracker::enable()
    {
    // Set the expiration first to avoid a race.
    if (gExpirationDuration == VDuration::ZERO())
        gExpirationTime = VInstant::INFINITE_FUTURE();
    else
        gExpirationTime = VInstant(/*now*/) + gExpirationDuration;

    gTrackMemory = true;
    }

// static
void VMemoryTracker::disable()
    {
    gTrackMemory = false;
    }

// static
void VMemoryTracker::reset()
    {
    VMutexLocker locker(&gAllocationMapMutex, VString::EMPTY());
    bool wasTracking = gTrackMemory;
    gTrackMemory = false;
    gInsideLockedMutex = true; // prevents our deletes from triggering delete processing while we hold the mutex

    for (AllocationMap::const_iterator i = gAllocationMap.begin(); i != gAllocationMap.end(); ++i)
        {
        const void* p = i->first;
        const AllocationRecord* r = i->second;
        if (r != NULL)
            {
            gAllocationMap[p] = NULL;
            delete r;
            }
        }

    gAllocationMap.clear();
    gCurrentNumAllocations = 0;
    gInsideLockedMutex = false;
    gTrackMemory = wasTracking;
    }

// static
bool VMemoryTracker::isEnabled()
    {
    return gTrackMemory;
    }

// static
void VMemoryTracker::setLimit(int maxNumAllocations)
    {
    gMaxNumAllocations = maxNumAllocations;
    }

// static
int VMemoryTracker::getLimit()
    {
    return gMaxNumAllocations;
    }

// static
void VMemoryTracker::setOver(size_t newOver)
    {
    gTrackAllocationssOver = newOver;
    }

// static
size_t VMemoryTracker::getOver()
    {
    return gTrackAllocationssOver;
    }

// static
void VMemoryTracker::setUnder(size_t newUnder)
    {
    gTrackAllocationssUnder = newUnder == 0 ? V_MAX_S32 : newUnder;
    }

// static
size_t VMemoryTracker::getUnder()
    {
    return gTrackAllocationssUnder;
    }

Vs64 VMemoryTracker::getAllocationNumber()
    {
    return gNextAllocationNumber;
    }

// static
void VMemoryTracker::setExpiration(const VDuration& d)
    {
    gExpirationDuration = d;

    if (gExpirationDuration == VDuration::ZERO())
        gExpirationTime = VInstant::INFINITE_FUTURE();
    else
        gExpirationTime = VInstant(/*now*/) + gExpirationDuration;
    }

// static
Vs64 VMemoryTracker::getExpirationTime()
    {
    return gExpirationTime.getValue();
    }

// static
Vs64 VMemoryTracker::getExpirationMilliseconds()
    {
    return gExpirationDuration.getDurationMilliseconds();
    }

// static
void VMemoryTracker::omitPointer(const void* p)
    {
    const AllocationRecord* r = _getFromMap(p);
    if (r != NULL)
        {
        _removeFromMap(p);
        delete r;
        }
    }

// static
void VMemoryTracker::enableCodeLocationCrawl(const VString& file, int line)
    {
    for (CodeLocationVector::const_iterator i = gStackCrawlCodeLocations.begin(); i != gStackCrawlCodeLocations.end(); ++i)
        {
        if (((*i).mFile == file) && ((*i).mLine == line))
            return;
        }

    // Not already enabled -- add it.
    gStackCrawlCodeLocations.push_back(CodeLocation(file, line));
    }

// static
void VMemoryTracker::disableCodeLocationCrawl(const VString& file, int line)
    {
    for (CodeLocationVector::iterator i = gStackCrawlCodeLocations.begin(); i != gStackCrawlCodeLocations.end(); ++i)
        {
        if (((*i).mFile == file) && ((*i).mLine == line))
            {
            gStackCrawlCodeLocations.erase(i);
            return;
            }
        }
    }

static void* _allocateMemory(size_t size, const char* file, int line, bool isArray)
    {
    void* p = ::malloc(size); // Would prefer to call through to global new: ::operator new(size) or new[](size)

    // When malloc fails, it returns null. But new should throw std::bad_alloc().
    if (p == NULL)
        throw std::bad_alloc();

    if (gTrackMemory)
        {
        if (size <= gTrackAllocationssOver || size >= gTrackAllocationssUnder)
            {
            return p;
            }
        // The most efficient way to check expiration is to go ahead and create the allocation record,
        // and use its timestamp. This way we don't also read the clock a 2nd time.
        if ((gMaxNumAllocations < 1) || (gCurrentNumAllocations < gMaxNumAllocations))
            {
            const char* stackCrawlInfo = NULL;
            if (_isCodeLocationCrawlEnabled(file, line))
                {
                VStringLogger logger(VString::EMPTY(), VLoggerLevel::TRACE);
                VThread::logStackCrawl(VString::EMPTY(), VNamedLoggerPtr(&logger), false);
                stackCrawlInfo = logger.orphanLines();
                }
            const AllocationRecord* r = new AllocationRecord(p, isArray, size, file, line, stackCrawlInfo);
            _putToMap(p, r);

            if ((gExpirationDuration != VDuration::ZERO()) && (r->fWhen > gExpirationTime))
                {
                gTrackMemory = false;
                }
            }
        else
            {
            gTrackMemory = false;
            }
        }
    return p;
    }

static void _freeMemory(void* p, bool /*isArray*/)
    {
    if (gTrackMemory || (!gInsideLockedMutex && (gCurrentNumAllocations > 0))) // We must honor deletions that might be in our map, even if tracking is disabled. Unless we're inside our own mutex lock block.
        {
        const AllocationRecord* r = _getFromMap(p);
        if (r != NULL)
            {
            _removeFromMap(p);
            delete r;
            }
        }

    ::free(p); // Would prefer to call through to global delete: ::operator delete(p) or delete[](p)
    }

void* operator new(size_t size, const char* file, int line)
    {
    return _allocateMemory(size, file, line, false);
    }

void operator delete(void* p, const char* /*file*/, int /*line*/)
    {
    if (p == NULL)
        return;

    _freeMemory(p, false);
    }

void operator delete(void* p) throw()
    {
    if (p == NULL)
        return;

    _freeMemory(p, false);
    }

void* operator new[](size_t size, const char* file, int line)
    {
    return _allocateMemory(size, file, line, true);
    }

void operator delete[](void* p, const char* /*file*/, int /*line*/)
    {
    if (p == NULL)
        return;

    _freeMemory(p, true);
    }

void operator delete[](void* p) throw()
    {
    if (p == NULL)
        return;

    _freeMemory(p, true);
    }

static void _reportText(const VString& s, bool toLogger, bool toConsole, VTextIOStream* toStream)
    {
    if (toLogger)
        VLOGGER_INFO(s);

    if (toConsole)
        std::cout << s << std::endl;

    if (toStream != NULL)
        toStream->writeLine(s);
    }

void VMemoryTracker::reportMemoryTracking(const VString& label, bool toLogger, bool toConsole, VTextIOStream* toStream, Vs64 bufferLengthLimit, bool showDetails, bool /*performAnalysis*/)
    {
    VMemoryTracker::omitPointer(label.getDataBufferConst()); // don't include the label in the report

    VMutexLocker locker(&gAllocationMapMutex, VString::EMPTY());
    gInsideLockedMutex = true; // prevents our deletes from triggering delete processing while we hold the mutex
    Vs64 numObjects = 0;
    size_t numBytes = 0;
    bool wasTracking = gTrackMemory;
    gTrackMemory = false;
    VDuration duration;

        { // scope for "records" to guarantee STL deletes stuff before we re-enable gTrackMemory at end of function
        VInstant start;
        AllocationPtrVector records;

        // First pass is to gather the objects into a vector we can sort.
        for (AllocationMap::const_iterator i = gAllocationMap.begin(); i != gAllocationMap.end(); ++i)
            {
            // void* p = i->first;
            const AllocationRecord* r = i->second;
            if (r != NULL)
                records.push_back(AllocationRecordPtr(r));
            }

        std::sort(records.begin(), records.end());

        // Second pass uses the vector and prints info about each record.
        _reportText(VSTRING_FORMAT("----- START %s", (label.isEmpty() ? REPORT_LABEL.chars():label.chars())), toLogger, toConsole, toStream);
        _reportText(VSTRING_FORMAT(" Tracked object limit=%d, tracked object count=%d.", gMaxNumAllocations, gCurrentNumAllocations), toLogger, toConsole, toStream);
        for (AllocationPtrVector::const_iterator i = records.begin(); i != records.end(); ++i)
            {
            const AllocationRecord* r = (*i).fPtr;
            if (r != NULL)
                {
                ++numObjects;
                numBytes += r->fSize;
                const Vu8* dataPtr = static_cast<const Vu8*>(r->fPointer);
                Vs64 hexDumpLength = static_cast<Vs64>(r->fSize);
                hexDumpLength = V_MIN(bufferLengthLimit, hexDumpLength);
                
                // Strip off the front of the full file path, leaving just the file name. Could be DOS or Unix separators present.
                VString fileName(r->fFile);
                _stripFileName(fileName);

                try 
                    {
                    if (showDetails)
                        {
                        VString timeString;
                        r->fWhen.getLocalString(timeString);
                        VString summary(VSTRING_ARGS(" [" VSTRING_FORMATTER_S64 "] [%s] 0x%08X " VSTRING_FORMATTER_SIZE " bytes @%s:%d", r->fAllocationNumber, timeString.chars(), r->fPointer, r->fSize, fileName.chars(), r->fLine));

                        if (r->fStackCrawlInfo != NULL)
                            {
                            summary += VString::NATIVE_LINE_ENDING();
                            summary += r->fStackCrawlInfo;
                            }

                        if (toLogger)
                            {
                            VLOGGER_HEXDUMP(VLoggerLevel::INFO, summary, dataPtr, hexDumpLength);
                            }

                        if (toConsole)
                            {
                            std::cout << summary.chars() << std::endl;
                            VHex hexDump(NULL);
                            hexDump.printHex(dataPtr, hexDumpLength);
                            }

                        if (toStream)
                            {
                            toStream->writeLine(summary);
                            VHex hexDump(toStream);
                            hexDump.printHex(dataPtr, hexDumpLength);
                            }
                        }
                    else
                        {
                        VString hexString;
                        VString asciiChars;
                        if (dataPtr != NULL && hexDumpLength > 0) {
                            VHex::bufferToHexString(dataPtr, hexDumpLength, hexString, false);
                            VHex::bufferToPrintableASCIIString(dataPtr, hexDumpLength, asciiChars);
                        }
                        VString summary(VSTRING_ARGS(" [" VSTRING_FORMATTER_S64 "] 0x%08X " VSTRING_FORMATTER_SIZE " bytes @%s:%d %s %s", r->fAllocationNumber, r->fPointer, r->fSize, fileName.chars(), r->fLine, asciiChars.chars(), hexString.chars()));

                        if (r->fStackCrawlInfo != NULL)
                            {
                            summary += " ... was allocated by:";
                            summary += VString::NATIVE_LINE_ENDING();
                            summary += r->fStackCrawlInfo;
                            }

                        _reportText(summary, toLogger, toConsole, toStream);
                        }
                    }
                catch (...) 
                    {
                    VString summary(VSTRING_ARGS(" [" VSTRING_FORMATTER_S64 "] 0x%08X " VSTRING_FORMATTER_SIZE " bytes @%s:%d **EXCEPTION GETTING DETAILS**", r->fAllocationNumber, r->fPointer, r->fSize, fileName.chars(), r->fLine));
                    _reportText(summary, toLogger, toConsole, toStream);
                    }
                }
            }

        VInstant end;
        duration = end - start;
        } // end of artificial scope ensuring "records" vector is cleaned up early

    _reportText(VSTRING_FORMAT(" Total objects found: " VSTRING_FORMATTER_S64 " objects, " VSTRING_FORMATTER_SIZE " bytes. %s", numObjects, numBytes, duration.getDurationString().chars()), toLogger, toConsole, toStream);

    if (!wasTracking && (numObjects > 0)) // Remind user that we still need to monitor deletes while our map has records.
        _reportText("WARNING: There is still some performance overhead until you 'reset' the tracked memory.", false/*only scare interactive user, not log readers*/, toConsole, toStream);

    _reportText(VSTRING_FORMAT("----- END %s", (label.isEmpty() ? REPORT_LABEL.chars():label.chars())), toLogger, toConsole, toStream);

    gInsideLockedMutex = false;
    gTrackMemory = wasTracking;
    }

#endif /* VAULT_MEMORY_ALLOCATION_TRACKING_SUPPORT */

