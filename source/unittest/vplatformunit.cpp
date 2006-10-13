/*
Copyright c1997-2006 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.5
http://www.bombaydigital.com/
*/

/** @file */

#include "vplatformunit.h"

VPlatformUnit::VPlatformUnit(bool logOnSuccess, bool throwOnError)
: VUnit("VPlatformUnit", logOnSuccess, throwOnError)
    {
    }

void VPlatformUnit::run()
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

// If you find that the call to _wrap_vsnprintf() below crashes on some
// new platform, you can comment out this #define statement, and the
// platform checks will proceed, presuming that the feature is unavailable
// rather than actually testing for it. All platforms tested so far
// survive it as a valid way to test the V_EFFICIENT_SPRINTF setting.
#define PERFORM_VSNPRINTF_NULL_FEATURE_CHECK

void VPlatformUnit::_runEfficientSprintfCheck()
    {
#ifdef PERFORM_VSNPRINTF_NULL_FEATURE_CHECK
    int theLength = _wrap_vsnprintf(NULL, 0, "%s%s%s%s%s", "a", "b", "c", "d", "e");
    
    if (theLength == 5)
        {
#ifdef V_EFFICIENT_SPRINTF
        this->test(true, "V_EFFICIENT_SPRINTF should be defined.");
#else
        this->test(false, "V_EFFICIENT_SPRINTF should be defined.");
#endif
        }
    else
        {
#ifdef V_EFFICIENT_SPRINTF
        this->test(false, "V_EFFICIENT_SPRINTF should not be defined.");
#else
        this->test(true, "V_EFFICIENT_SPRINTF should not be defined.");
#endif
        }
#else /* PERFORM_VSNPRINTF_NULL_FEATURE_CHECK not in effect follows */

#ifdef V_EFFICIENT_SPRINTF
        this->test(false, "V_EFFICIENT_SPRINTF should not be defined.");
#else
        this->test(true, "V_EFFICIENT_SPRINTF should not be defined.");
#endif

#endif /* PERFORM_VSNPRINTF_NULL_FEATURE_CHECK */
    }

void VPlatformUnit::_runByteswapCheck()
    {
    const Vs16    kS16Value = 0x0123;
    Vs16    s16 = kS16Value;
    
    Vu8* s16p1 = (Vu8*) &s16; // point to "highest" byte (assuming big endian)
    Vu8* s16p2 = s16p1 + 1;   // point to "lowest" byte (assuming big endian)

    const Vs32    kS32Value = 0x01234567;
    Vs32    s32 = kS32Value;

    Vu8* s32p1 = (Vu8*) &s32; // point to "highest" byte (assuming big endian)
    Vu8* s32p2 = s32p1 + 1;
    Vu8* s32p3 = s32p1 + 2;
    Vu8* s32p4 = s32p1 + 3;   // point to "lowest" byte (assuming big endian)

    const Vs64    kS64Value = CONST_S64(0x0123456789ABCDEF);
    Vs64    s64 = kS64Value;

    Vu8* s64p1 = (Vu8*) &s64; // point to "highest" byte (assuming big endian)
    Vu8* s64p2 = s64p1 + 1;
    Vu8* s64p3 = s64p1 + 2;
    Vu8* s64p4 = s64p1 + 3;
    Vu8* s64p5 = s64p1 + 4;
    Vu8* s64p6 = s64p1 + 5;
    Vu8* s64p7 = s64p1 + 6;
    Vu8* s64p8 = s64p1 + 7;   // point to "lowest" byte (assuming big endian)

    if (*s16p1 == 0x01 && *s16p2 == 0x23 &&
        *s32p1 == 0x01 && *s32p2 == 0x23 && *s32p3 == 0x45 && *s32p4 == 0x67 &&
        *s64p1 == 0x01 && *s64p2 == 0x23 && *s64p3 == 0x45 && *s64p4 == 0x67 && *s64p5 == 0x89 && *s64p6 == 0xAB && *s64p7 == 0xCD && *s64p8 == 0xEF
        )
        {
        // We're big endian.
#ifdef VBYTESWAP_NEEDED
        this->test(false, "VBYTESWAP_NEEDED should not be defined on a big-endian system.");
#else
        this->test(true, "VBYTESWAP_NEEDED should not be defined on a big-endian system.");
#endif
        }
    else if (*s16p1 == 0x23 && *s16p2 == 0x01 &&
        *s32p1 == 0x67 && *s32p2 == 0x45 && *s32p3 == 0x23 && *s32p4 == 0x01 &&
        *s64p1 == 0xEF && *s64p2 == 0xCD && *s64p3 == 0xAB && *s64p4 == 0x89 && *s64p5 == 0x67 && *s64p6 == 0x45 && *s64p7 == 0x23 && *s64p8 == 0x01
        )
        {
        // We're little endian.
#ifdef VBYTESWAP_NEEDED
        this->test(true, "VBYTESWAP_NEEDED should be defined on a little-endian system.");
#else
        this->test(false, "VBYTESWAP_NEEDED should be defined on a little-endian system.");
#endif
        }
    else
        {
        // We're something weirder.
#ifdef VBYTESWAP_NEEDED
        this->test(false, "VBYTESWAP_NEEDED is correctly defined on an other-endian system, but the particular swapping code is not implemented.");
#else
        this->test(false, "VBYTESWAP_NEEDED should be defined on an other-endian system.");
#endif
        }

    }

typedef int IntTypedef;
typedef Vs32 Vs32Typedef;

void VPlatformUnit::_runMinMaxAbsCheck()
    {
    // This function is mainly to detect compiler conflicts. If it compiles it
    // will probably work. However, it also verifies that the macros do what
    // they claim.

    Vs8         s8_low = -5;
    Vs8         s8_high = 5;
    Vs16        s16_low = -5;
    Vs16        s16_high = 5;
    Vs32        s32_low = -5;
    Vs32        s32_high = 5;
    Vs64        s64_low = CONST_S64(-5);
    Vs64        s64_high = CONST_S64(5);
    int         int_low = -5;
    int         int_high = 5;
    VFloat      float_low = -5.0f;
    VFloat      float_high = 5.0f;
    VDouble     double_low = -5.0;
    VDouble     double_high = 5.0;
    IntTypedef  intT_low = -5;
    IntTypedef  intT_high = 5;
    Vs32Typedef Vs32T_low = -5;
    Vs32Typedef Vs32T_high = 5;
    
    this->test(V_ABS(s8_low) == s8_high, "V_ABS for Vs8");
    this->test(V_ABS(s16_low) == s16_high, "V_ABS for Vs16");
    this->test(V_ABS(s32_low) == s32_high, "V_ABS for Vs32");
    this->test(V_ABS(s64_low) == s64_high, "V_ABS for Vs64");
    this->test(V_ABS(int_low) == int_high, "V_ABS for int");
    this->test(V_FABS(float_low) == float_high, "V_FABS for float");
    this->test(V_FABS(double_low) == double_high, "V_FABS for double");
    this->test(V_ABS(intT_low) == intT_high, "V_ABS for int typedef");
    this->test(V_ABS(Vs32T_low) == Vs32T_high, "V_ABS for Vs32 typedef");
    
    Vs8         s8_min = V_MIN(s8_low, s8_high);
    Vs8         s8_max = V_MAX(s8_low, s8_high);
    Vs16        s16_min = V_MIN(s16_low, s16_high);
    Vs16        s16_max = V_MAX(s16_low, s16_high);
    Vs32        s32_min = V_MIN(s32_low, s32_high);
    Vs32        s32_max = V_MAX(s32_low, s32_high);
    Vs64        s64_min = V_MIN(s64_low, s64_high);
    Vs64        s64_max = V_MAX(s64_low, s64_high);
    int         int_min = V_MIN(int_low, int_high);
    int         int_max = V_MAX(int_low, int_high);
    VFloat      float_min = V_MIN(float_low, float_high);
    VFloat      float_max = V_MAX(float_low, float_high);
    VDouble     double_min = V_MIN(double_low, double_high);
    VDouble     double_max = V_MAX(double_low, double_high);
    IntTypedef  intT_min = V_MIN(intT_low, intT_high);
    IntTypedef  intT_max = V_MAX(intT_low, intT_high);
    Vs32Typedef Vs32T_min = V_MIN(Vs32T_low, Vs32T_high);
    Vs32Typedef Vs32T_max = V_MAX(Vs32T_low, Vs32T_high);
    
    this->test(s8_min == s8_low, "V_MIN for Vs8");
    this->test(s8_max == s8_high, "V_MAX for Vs8");
    this->test(s16_min == s16_low, "V_MIN for Vs16");
    this->test(s16_max == s16_high, "V_MAX for Vs16");
    this->test(s32_min == s32_low, "V_MIN for Vs32");
    this->test(s32_max == s32_high, "V_MAX for Vs32");
    this->test(s64_min == s64_low, "V_MIN for Vs64");
    this->test(s64_max == s64_high, "V_MAX for Vs64");
    this->test(int_min == int_low, "V_MIN for int");
    this->test(int_max == int_high, "V_MAX for int");
    this->test(float_min == float_low, "V_MIN for float");
    this->test(float_max == float_high, "V_MAX for float");
    this->test(double_min == double_low, "V_MIN for double");
    this->test(double_max == double_high, "V_MAX for double");
    this->test(intT_min == intT_low, "V_MIN for int typedef");
    this->test(intT_max == intT_high, "V_MAX for int typedef");
    this->test(Vs32T_min == Vs32T_low, "V_MIN for Vs32 typedef");
    this->test(Vs32T_max == Vs32T_high, "V_MAX for Vs32 typedef");
    }

void VPlatformUnit::_runTimeCheck()
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
        this->test(true, "V_INSTANT_SNAPSHOT_IS_UTC should be defined for high-resolution times.");
#else
        this->test(false, "V_INSTANT_SNAPSHOT_IS_UTC should be defined for high-resolution times.");
#endif /* V_INSTANT_SNAPSHOT_IS_UTC */
        }
    else
        {
#ifdef V_INSTANT_SNAPSHOT_IS_UTC
        this->test(false, "V_INSTANT_SNAPSHOT_IS_UTC should be not be defined. This platform does not have high-resolution times.");
#else
        this->test(true, "V_INSTANT_SNAPSHOT_IS_UTC should be not be defined. This platform does not have high-resolution times.");
#endif /* V_INSTANT_SNAPSHOT_IS_UTC */
        }

    }
