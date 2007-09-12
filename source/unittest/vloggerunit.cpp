/*
Copyright c1997-2006 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.5
http://www.bombaydigital.com/
*/

/** @file */

#include "vloggerunit.h"
#include "vlogger.h"

/**
V2xInterceptLogger is a VInterceptLogger that captures a 2nd previous message for
our VLoggerRepetitionFilter testing.
*/
class V2xInterceptLogger : public VInterceptLogger
    {
    public:

        V2xInterceptLogger(int logLevel, const VString& name) : VInterceptLogger(logLevel, name, VString::EMPTY()), mNumRawLinesEmitted(0) {}
        virtual ~V2xInterceptLogger() {}

        const VString& get2ndLastMessage() const { return m2ndLastLoggedMessage; }
        virtual void reset() { VInterceptLogger::reset(); m2ndLastLoggedMessage = VString::EMPTY(); mNumRawLinesEmitted = 0; }
        int getNumRawLinesEmitted() const { return mNumRawLinesEmitted; }
        void setFilterEnabled(bool enabled) { mRepetitionFilter.setEnabled(enabled); }

    protected:

        virtual void emitRawLine(const VString& line) { m2ndLastLoggedMessage = this->getLastMessage(); VInterceptLogger::emitRawLine(line); ++mNumRawLinesEmitted; }

    private:

        int mNumRawLinesEmitted;
        VString m2ndLastLoggedMessage;
    };

// VLoggerUnit ------------------------------------------------------------------------

VLoggerUnit::VLoggerUnit(bool logOnSuccess, bool throwOnError)
: VUnit("VLoggerUnit", logOnSuccess, throwOnError)
    {
    }

void VLoggerUnit::run()
    {
    V2xInterceptLogger logger(VLogger::kInfo, "VLoggerUnit's V2xInterceptLogger");
    
    const VString TEST_MESSAGE_1("Test message 1");
    logger.log(VLogger::kInfo, TEST_MESSAGE_1);
    this->test(logger.getLastMessage() == TEST_MESSAGE_1, TEST_MESSAGE_1);
    
    const VString TEST_MESSAGE_2("Test message 2");
    logger.log(VLogger::kInfo, TEST_MESSAGE_2);
    this->test(logger.getLastMessage() == TEST_MESSAGE_2, TEST_MESSAGE_2);
    
    const VString TEST_MESSAGE_SENTINEL("Test message sentinel");
    const VString TEST_MESSAGE_REPEATING("Test message that repeats");
    const VString TEST_MESSAGE_REPEATING_3X("[3x] Test message that repeats");

    // If we emit a given message twice, then emit another, we should see the repeated message
    // emitted in normal fashion, because a mere 1x repeat is not shortened by noting "[1x]" in
    // the repeat indicator output.
    logger.reset();
    logger.log(VLogger::kInfo, TEST_MESSAGE_SENTINEL);
    this->test(logger.getLastMessage() == TEST_MESSAGE_SENTINEL, TEST_MESSAGE_SENTINEL);
    this->test(logger.getNumRawLinesEmitted() == 1, "emitted 1");

    logger.log(VLogger::kInfo, TEST_MESSAGE_REPEATING); // original
    this->test(logger.get2ndLastMessage() == TEST_MESSAGE_SENTINEL, TEST_MESSAGE_SENTINEL);
    this->test(logger.getLastMessage() == TEST_MESSAGE_REPEATING, TEST_MESSAGE_REPEATING);
    this->test(logger.getNumRawLinesEmitted() == 2, "emitted 2");

    logger.log(VLogger::kInfo, TEST_MESSAGE_REPEATING); // 1x repeat, should get buffered
    this->test(logger.get2ndLastMessage() == TEST_MESSAGE_SENTINEL, TEST_MESSAGE_SENTINEL);
    this->test(logger.getLastMessage() == TEST_MESSAGE_REPEATING, TEST_MESSAGE_REPEATING);
    this->test(logger.getNumRawLinesEmitted() == 2, "still emitted 2");

    logger.log(VLogger::kInfo, TEST_MESSAGE_SENTINEL); // cause repeat to be output without indicator
    this->test(logger.get2ndLastMessage() == TEST_MESSAGE_REPEATING, TEST_MESSAGE_REPEATING);
    this->test(logger.getLastMessage() == TEST_MESSAGE_SENTINEL, TEST_MESSAGE_SENTINEL);
    this->test(logger.getNumRawLinesEmitted() == 4, "emitted 4");
    
    // Now do the same but repeat more than 1x, so that the eventual output contains
    // the correct indicator of the number of repeats.
    logger.reset();
    logger.log(VLogger::kInfo, TEST_MESSAGE_SENTINEL);
    this->test(logger.getLastMessage() == TEST_MESSAGE_SENTINEL, TEST_MESSAGE_SENTINEL);
    this->test(logger.getNumRawLinesEmitted() == 1, "emitted 1");

    logger.log(VLogger::kInfo, TEST_MESSAGE_REPEATING); // original
    this->test(logger.get2ndLastMessage() == TEST_MESSAGE_SENTINEL, TEST_MESSAGE_SENTINEL);
    this->test(logger.getLastMessage() == TEST_MESSAGE_REPEATING, TEST_MESSAGE_REPEATING);
    this->test(logger.getNumRawLinesEmitted() == 2, "emitted 2");

    logger.log(VLogger::kInfo, TEST_MESSAGE_REPEATING); // 1x repeat, should get buffered
    this->test(logger.get2ndLastMessage() == TEST_MESSAGE_SENTINEL, TEST_MESSAGE_SENTINEL);
    this->test(logger.getLastMessage() == TEST_MESSAGE_REPEATING, TEST_MESSAGE_REPEATING);
    this->test(logger.getNumRawLinesEmitted() == 2, "still emitted 2");

    logger.log(VLogger::kInfo, TEST_MESSAGE_REPEATING); // 2x repeat, should get buffered
    this->test(logger.get2ndLastMessage() == TEST_MESSAGE_SENTINEL, TEST_MESSAGE_SENTINEL);
    this->test(logger.getLastMessage() == TEST_MESSAGE_REPEATING, TEST_MESSAGE_REPEATING);
    this->test(logger.getNumRawLinesEmitted() == 2, "still emitted 2");

    logger.log(VLogger::kInfo, TEST_MESSAGE_REPEATING); // 3x repeat, should get buffered
    this->test(logger.get2ndLastMessage() == TEST_MESSAGE_SENTINEL, TEST_MESSAGE_SENTINEL);
    this->test(logger.getLastMessage() == TEST_MESSAGE_REPEATING, TEST_MESSAGE_REPEATING);
    this->test(logger.getNumRawLinesEmitted() == 2, "still emitted 2");

    logger.log(VLogger::kInfo, TEST_MESSAGE_SENTINEL); // cause repeat to be output WITH [3x] indicator
    this->test(logger.get2ndLastMessage() == TEST_MESSAGE_REPEATING_3X, TEST_MESSAGE_REPEATING_3X);
    this->test(logger.getLastMessage() == TEST_MESSAGE_SENTINEL, TEST_MESSAGE_SENTINEL);
    this->test(logger.getNumRawLinesEmitted() == 4, "emitted 4");
    
    // Make sure that we get all output emitted if the filter is disabled.
    logger.setFilterEnabled(false);

    logger.log(VLogger::kInfo, TEST_MESSAGE_1);
    this->test(logger.getLastMessage() == TEST_MESSAGE_1, "Unfiltered 1");
    
    logger.log(VLogger::kInfo, TEST_MESSAGE_2);
    this->test(logger.get2ndLastMessage() == TEST_MESSAGE_1, "Unfiltered 2a");
    this->test(logger.getLastMessage() == TEST_MESSAGE_2, "Unfiltered 2b");
    
    logger.log(VLogger::kInfo, TEST_MESSAGE_2);
    this->test(logger.get2ndLastMessage() == TEST_MESSAGE_2, "Unfiltered 3a");
    this->test(logger.getLastMessage() == TEST_MESSAGE_2, "Unfiltered 3b");
    
    logger.log(VLogger::kInfo, TEST_MESSAGE_1);
    this->test(logger.get2ndLastMessage() == TEST_MESSAGE_2, "Unfiltered 4a");
    this->test(logger.getLastMessage() == TEST_MESSAGE_1, "Unfiltered 4b");
    }

