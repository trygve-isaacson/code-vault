/*
Copyright c1997-2005 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.3.2
http://www.bombaydigital.com/
*/

#include "vault.h"

#include "vunitrunall.h"

class App
    {
    public:
    
        App(int argc, char** argv);
        ~App();
        void run();
        
        int getResult() { return mResult; }

    private:
    
        void _runPlatformCheck();

        void _runEfficientSprintfCheck();
        void _runByteswapCheck();
        void _runMinMaxAbsCheck();
        void _runTimeCheck();
    
        int        mArgc;
        char**    mArgv;
        int        mResult;
    };

int main(int argc, char** argv)
    {
    int    result = -1;
    App    app(argc, argv);
    
    try
        {
        app.run();
        result = app.getResult();
        }
    catch (const VException& ex)
        {
        std::cout << "ERROR: Caught VException (" << ex.getError() << "): '" << ex.what() << "'\n";
        }
    catch(const std::exception &ex)
        {
        std::cout << "ERROR: Caught STL exception: '" << ex.what() << "'\n";
        }

    if (result == 0)
        std::cout << "SUCCESS: Platform check completed with result 0." << std::endl;
    else
        std::cout << "ERROR: Platform check completed with result " << result << "." << std::endl;
    
    return result;
    }

App::App(int argc, char** argv) :
mArgc(argc),
mArgv(argv),
mResult(0)
    {
    }

App::~App()
    {
    }

void App::run()
    {
    this->_runPlatformCheck();
    
    bool success;
    int numSuccessfulTests;
    int numFailedTests;
    runAllVUnitTests(false, true, false, success, numSuccessfulTests, numFailedTests, NULL);
    
    if (!success)
        mResult = -1;
    }

void App::_runPlatformCheck()
    {
    this->_runEfficientSprintfCheck();
    this->_runByteswapCheck();
    this->_runMinMaxAbsCheck();
    this->_runTimeCheck();
    }

static int _wrap_vsnprintf(char* dest, size_t count, const char* formatText, ...)
    {
 	va_list	args;
	va_start(args, formatText);

	int result = vault::vsnprintf(dest, count, formatText, args);

	va_end(args);
	
	return result;
    }

void App::_runEfficientSprintfCheck()
    {
    char    oneByteBuffer = 0;
    char    stackCheck1 = 0;
    char    stackCheck2 = 0;
    char    stackCheck3 = 0;
    char    stackCheck4 = 0;
    int        theLength = _wrap_vsnprintf(&oneByteBuffer, 1, "%s%s%s%s%s", "a", "b", "c", "d", "e");
    
    if (theLength == 5 && stackCheck1 == 0 && stackCheck2 == 0 && stackCheck3 == 0 && stackCheck4 == 0)
        {
#ifdef V_EFFICIENT_SPRINTF
        std::cout << "SUCCESS: Efficient snprintf is available on this platform, and V_EFFICIENT_SPRINTF is correctly defined. The vtypes_platform.h for this platform is OK." << std::endl;
#else
        std::cout << "WARNING: Efficient snprintf is available on this platform, but V_EFFICIENT_SPRINTF is undefined. You should fix the vtypes_platform.h for this platform, so that VString formatting is more efficient." << std::endl;
#endif
        }
    else
        {
#ifdef V_EFFICIENT_SPRINTF
        std::cout << "ERROR: Efficient snprintf is not available on this platform, but V_EFFICIENT_SPRINTF is defined. You need to fix the vtypes_platform.h for this platform; otherwise, VString formatting may crash." << std::endl;
        mResult = -1;
#else
        std::cout << "SUCCESS: Efficient snprintf is not available on this platform, and V_EFFICIENT_SPRINTF is correctly undefined. The vtypes_platform.h for this platform is OK." << std::endl;
#endif
        }
    }

void App::_runByteswapCheck()
    {
    const Vs16    kS16Value = 0x0123;
    Vs16    s16 = kS16Value;
    
    Vu8* s16p1 = (Vu8*) &s16;    // point to "highest" byte (assuming big endian)
    Vu8* s16p2 = s16p1 + 1;        // point to "lowest" byte (assuming big endian)

    const Vs32    kS32Value = 0x01234567;
    Vs32    s32 = kS32Value;

    Vu8* s32p1 = (Vu8*) &s32;    // point to "highest" byte (assuming big endian)
    Vu8* s32p2 = s32p1 + 1;
    Vu8* s32p3 = s32p1 + 2;
    Vu8* s32p4 = s32p1 + 3;        // point to "lowest" byte (assuming big endian)

    const Vs64    kS64Value = CONST_S64(0x0123456789ABCDEF);
    Vs64    s64 = kS64Value;

    Vu8* s64p1 = (Vu8*) &s64;    // point to "highest" byte (assuming big endian)
    Vu8* s64p2 = s64p1 + 1;
    Vu8* s64p3 = s64p1 + 2;
    Vu8* s64p4 = s64p1 + 3;
    Vu8* s64p5 = s64p1 + 4;
    Vu8* s64p6 = s64p1 + 5;
    Vu8* s64p7 = s64p1 + 6;
    Vu8* s64p8 = s64p1 + 7;        // point to "lowest" byte (assuming big endian)

    if (*s16p1 == 0x01 && *s16p2 == 0x23 &&
        *s32p1 == 0x01 && *s32p2 == 0x23 && *s32p3 == 0x45 && *s32p4 == 0x67 &&
        *s64p1 == 0x01 && *s64p2 == 0x23 && *s64p3 == 0x45 && *s64p4 == 0x67 && *s64p5 == 0x89 && *s64p6 == 0xAB && *s64p7 == 0xCD && *s64p8 == 0xEF
        )
        {
        // We're big endian.
#ifdef VBYTESWAP_NEEDED
        std::cout << "ERROR: This platform is big-endian, but VBYTESWAP_NEEDED is defined. You need to fix the vtypes_platform.h for this platform, or stream byte ordering will be wrong." << std::endl;
        mResult = -1;
#else
        std::cout << "SUCCESS: This platform is big-endian, and VBYTESWAP_NEEDED is correctly undefined. The vtypes_platform.h for this platform is OK." << std::endl;
#endif
        }
    else if (*s16p1 == 0x23 && *s16p2 == 0x01 &&
        *s32p1 == 0x67 && *s32p2 == 0x45 && *s32p3 == 0x23 && *s32p4 == 0x01 &&
        *s64p1 == 0xEF && *s64p2 == 0xCD && *s64p3 == 0xAB && *s64p4 == 0x89 && *s64p5 == 0x67 && *s64p6 == 0x45 && *s64p7 == 0x23 && *s64p8 == 0x01
        )
        {
        // We're little endian.
#ifdef VBYTESWAP_NEEDED
        std::cout << "SUCCESS: This platform is little-endian, and VBYTESWAP_NEEDED is correctly defined. The vtypes_platform.h for this platform is OK." << std::endl;
#else
        std::cout << "ERROR: This platform is little-endian, but VBYTESWAP_NEEDED is undefined. You need to fix the vtypes_platform.h for this platform, or stream byte ordering will be wrong." << std::endl;
        mResult = -1;
#endif
        }
    else
        {
        // We're something weirder.
#ifdef VBYTESWAP_NEEDED
        std::cout << "WARNING: This platform is neither big- nor little-endian, and VBYTESWAP_NEEDED is correctly defined. You need to verify that vtypes_platform.h for this platform does swapping correctly." << std::endl;
#else
        std::cout << "ERROR: This platform is neither big- nor little-endian, but VBYTESWAP_NEEDED is undefined. You need to fix the vtypes_platform.h for this platform, or stream byte ordering will be wrong." << std::endl;
        mResult = -1;
#endif
        }

    }

typedef int IntTypedef;
typedef Vs32 Vs32Typedef;

void App::_runMinMaxAbsCheck()
    {
    // This function is mainly to detect compiler conflicts. If it compiles it
    // will probably work.

    Vs8        s8_low = -5;
    Vs8     s8_high = 5;
    Vs16    s16_low = -5;
    Vs16    s16_high = 5;
    Vs32    s32_low = -5;
    Vs32    s32_high = 5;
    Vs64    s64_low = CONST_S64(-5);
    Vs64    s64_high = CONST_S64(5);
    int        int_low = -5;
    int        int_high = 5;
    VFloat    float_low = -5.0f;
    VFloat    float_high = 5.0f;
    VDouble    double_low = -5.0;
    VDouble    double_high = 5.0;
    IntTypedef    intT_low = -5;
    IntTypedef    intT_high = 5;
    Vs32Typedef    Vs32T_low = -5;
    Vs32Typedef    Vs32T_high = 5;
    
    if (V_ABS(s8_low) == s8_high)
        std::cout << "SUCCESS: V_ABS for Vs8 works." << std::endl;
    else
        {
        std::cout << "ERROR: V_ABS for Vs8 does not work." << std::endl;
        mResult = -1;
        }
    
    if (V_ABS(s16_low) == s16_high)
        std::cout << "SUCCESS: V_ABS for Vs16 works." << std::endl;
    else
        {
        std::cout << "ERROR: V_ABS for Vs16 does not work." << std::endl;
        mResult = -1;
        }
    
    if (V_ABS(s32_low) == s32_high)
        std::cout << "SUCCESS: V_ABS for Vs32 works." << std::endl;
    else
        {
        std::cout << "ERROR: V_ABS for Vs32 does not work." << std::endl;
        mResult = -1;
        }
    
    if (V_ABS(s64_low) == s64_high)
        std::cout << "SUCCESS: V_ABS for Vs64 works." << std::endl;
    else
        {
        std::cout << "ERROR: V_ABS for Vs64 does not work." << std::endl;
        mResult = -1;
        }
    
    if (V_ABS(int_low) == int_high)
        std::cout << "SUCCESS: V_ABS for int works." << std::endl;
    else
        {
        std::cout << "ERROR: V_ABS for int does not work." << std::endl;
        mResult = -1;
        }
    
    if (V_FABS(float_low) == float_high)
        std::cout << "SUCCESS: V_FABS for float works." << std::endl;
    else
        {
        std::cout << "ERROR: V_FABS for float does not work." << std::endl;
        mResult = -1;
        }
    
    if (V_FABS(double_low) == double_high)
        std::cout << "SUCCESS: V_FABS for double works." << std::endl;
    else
        {
        std::cout << "ERROR: V_FABS for double does not work." << std::endl;
        mResult = -1;
        }

    if (V_ABS(intT_low) == intT_high)
        std::cout << "SUCCESS: V_ABS for int typedef works." << std::endl;
    else
        {
        std::cout << "ERROR: V_ABS for int typedef does not work." << std::endl;
        mResult = -1;
        }
    
    if (V_ABS(Vs32T_low) == Vs32T_high)
        std::cout << "SUCCESS: V_ABS for Vs32 typedef works." << std::endl;
    else
        {
        std::cout << "ERROR: V_ABS for Vs32 typedef does not work." << std::endl;
        mResult = -1;
        }
    
    Vs8        s8_min = V_MIN(s8_low, s8_high);
    Vs8        s8_max = V_MAX(s8_low, s8_high);
    Vs16    s16_min = V_MIN(s16_low, s16_high);
    Vs16    s16_max = V_MAX(s16_low, s16_high);
    Vs32    s32_min = V_MIN(s32_low, s32_high);
    Vs32    s32_max = V_MAX(s32_low, s32_high);
    Vs64    s64_min = V_MIN(s64_low, s64_high);
    Vs64    s64_max = V_MAX(s64_low, s64_high);
    int        int_min = V_MIN(int_low, int_high);
    int        int_max = V_MAX(int_low, int_high);
    VFloat    float_min = V_MIN(float_low, float_high);
    VFloat    float_max = V_MAX(float_low, float_high);
    VDouble    double_min = V_MIN(double_low, double_high);
    VDouble    double_max = V_MAX(double_low, double_high);
    IntTypedef    intT_min = V_MIN(intT_low, intT_high);
    IntTypedef    intT_max = V_MAX(intT_low, intT_high);
    Vs32Typedef    Vs32T_min = V_MIN(Vs32T_low, Vs32T_high);
    Vs32Typedef    Vs32T_max = V_MAX(Vs32T_low, Vs32T_high);
    
    if (s8_min == s8_low)
        std::cout << "SUCCESS: V_MIN for Vs8 works." << std::endl;
    else
        {
        std::cout << "ERROR: V_MIN for Vs8 does not work." << std::endl;
        mResult = -1;
        }
    
    if (s8_max == s8_high)
        std::cout << "SUCCESS: V_MAX for Vs8 works." << std::endl;
    else
        {
        std::cout << "ERROR: V_MAX for Vs8 does not work." << std::endl;
        mResult = -1;
        }
    
    if (s16_min == s16_low)
        std::cout << "SUCCESS: V_MIN for Vs16 works." << std::endl;
    else
        {
        std::cout << "ERROR: V_MIN for Vs16 does not work." << std::endl;
        mResult = -1;
        }
    
    if (s16_max == s16_high)
        std::cout << "SUCCESS: V_MAX for Vs16 works." << std::endl;
    else
        {
        std::cout << "ERROR: V_MAX for Vs16 does not work." << std::endl;
        mResult = -1;
        }
    
    if (s32_min == s32_low)
        std::cout << "SUCCESS: V_MIN for Vs32 works." << std::endl;
    else
        {
        std::cout << "ERROR: V_MIN for Vs32 does not work." << std::endl;
        mResult = -1;
        }
    
    if (s32_max == s32_high)
        std::cout << "SUCCESS: V_MAX for Vs32 works." << std::endl;
    else
        {
        std::cout << "ERROR: V_MAX for Vs32 does not work." << std::endl;
        mResult = -1;
        }
    
    if (s64_min == s64_low)
        std::cout << "SUCCESS: V_MIN for Vs64 works." << std::endl;
    else
        {
        std::cout << "ERROR: V_MIN for Vs64 does not work." << std::endl;
        mResult = -1;
        }
    
    if (s64_max == s64_high)
        std::cout << "SUCCESS: V_MAX for Vs64 works." << std::endl;
    else
        {
        std::cout << "ERROR: V_MAX for Vs64 does not work." << std::endl;
        mResult = -1;
        }
    
    if (int_min == int_low)
        std::cout << "SUCCESS: V_MIN for int works." << std::endl;
    else
        {
        std::cout << "ERROR: V_MIN for int does not work." << std::endl;
        mResult = -1;
        }
    
    if (int_max == int_high)
        std::cout << "SUCCESS: V_MAX for int works." << std::endl;
    else
        {
        std::cout << "ERROR: V_MAX for int does not work." << std::endl;
        mResult = -1;
        }
    
    if (float_min == float_low)
        std::cout << "SUCCESS: V_MIN for float works." << std::endl;
    else
        {
        std::cout << "ERROR: V_MIN for float does not work." << std::endl;
        mResult = -1;
        }
    
    if (float_max == float_high)
        std::cout << "SUCCESS: V_MAX for float works." << std::endl;
    else
        {
        std::cout << "ERROR: V_MAX for float does not work." << std::endl;
        mResult = -1;
        }
    
    if (double_min == double_low)
        std::cout << "SUCCESS: V_MIN for double works." << std::endl;
    else
        {
        std::cout << "ERROR: V_MIN for double does not work." << std::endl;
        mResult = -1;
        }
    
    if (double_max == double_high)
        std::cout << "SUCCESS: V_MAX for double works." << std::endl;
    else
        {
        std::cout << "ERROR: V_MAX for double does not work." << std::endl;
        mResult = -1;
        }

    if (intT_min == intT_low)
        std::cout << "SUCCESS: V_MIN for int typedef works." << std::endl;
    else
        {
        std::cout << "ERROR: V_MIN for int typedef does not work." << std::endl;
        mResult = -1;
        }
    
    if (intT_max == intT_high)
        std::cout << "SUCCESS: V_MAX for int typedef works." << std::endl;
    else
        {
        std::cout << "ERROR: V_MAX for int typedef does not work." << std::endl;
        mResult = -1;
        }
    
    if (Vs32T_min == Vs32T_low)
        std::cout << "SUCCESS: V_MIN for Vs32 typedef works." << std::endl;
    else
        {
        std::cout << "ERROR: V_MIN for Vs32 typedef does not work." << std::endl;
        mResult = -1;
        }
    
    if (Vs32T_max == Vs32T_high)
        std::cout << "SUCCESS: V_MAX for Vs32 typedef works." << std::endl;
    else
        {
        std::cout << "ERROR: V_MAX for Vs32 typedef does not work." << std::endl;
        mResult = -1;
        }
    
    }

void App::_runTimeCheck()
    {
    Vs64 timeValue = CONST_S64(1000) * static_cast<Vs64>(::time(NULL));
    Vs64 snapshotValue = VInstant::snapshot();
    
    // Truncate the milliseconds from the snapshot value (it's got ms resolution).
    snapshotValue -= (snapshotValue % CONST_S64(1000));
    
    // We need to have a little bit of slop in our test, because at millisecond
    // resolution, those two values may have been calculated a few milliseconds
    // apart. So when we truncate the milliseconds off the snapshot value, we
    // may even have a wrapped value at seconds resolution.
    
    // So, if the snapshot is a UTC-based time, the two values should now
    // be "equal", which with slop means within 1 second of each other.
    
    Vs64 delta = V_ABS(snapshotValue - timeValue);
    
    if (delta <= 1000)
        {
#ifdef V_INSTANT_SNAPSHOT_IS_UTC
        std::cout << "SUCCESS: This platform does UTC-based time snapshots, and V_INSTANT_SNAPSHOT_IS_UTC is correctly defined. The vtypes_platform.h for this platform is OK." << std::endl;
#else
        std::cout << "WARNING: This platform does UTC-based time snapshots, but V_INSTANT_SNAPSHOT_IS_UTC is not defined. You should fix the vtypes_platform.h for this platform to get higher resolution VInstant values (milliseconds rather than seconds)." << std::endl;
#endif /* V_INSTANT_SNAPSHOT_IS_UTC */
        }
    else
        {
#ifdef V_INSTANT_SNAPSHOT_IS_UTC
        std::cout << "ERROR: This platform does NOT do UTC-based time snapshots, but V_INSTANT_SNAPSHOT_IS_UTC is defined. You need to fix the vtypes_platform.h for this platform; otherwise, VInstant values will be incorrect." << std::endl;
        mResult = -1;
#else
        std::cout << "SUCCESS: This platform does NOT do UTC-based time snapshots, and V_INSTANT_SNAPSHOT_IS_UTC is correctly undefined. The vtypes_platform.h for this platform is OK." << std::endl;
#endif /* V_INSTANT_SNAPSHOT_IS_UTC */
        }

    }
