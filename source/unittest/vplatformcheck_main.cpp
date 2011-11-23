/*
Copyright c1997-2011 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.3
http://www.bombaydigital.com/
*/

#include "vault.h"

#include "vunitrunall.h"

class App {
    public:

        App(int argc, char** argv);
        ~App();
        void run();

        int getResult() { return mResult; }

    private:

        App(const App&); // not copyable
        App& operator=(const App&); // not assignable

        VStringVector mArgs;
        int           mResult;
};

App::App(int argc, char** argv) :
    mArgs(),
    mResult(0) {
    for (int i = 1; i < argc; ++i) // Omit argc[0] which is just the application name, not really an arg to be processed.
        mArgs.push_back(argv[i]);
}

App::~App() {
}

void App::run() {
    VTestSuitesWrapper wrapper(mArgs);

    bool success;
    int numSuccessfulTests;
    int numFailedTests;
    runAllVUnitTests(true, false, success, numSuccessfulTests, numFailedTests, &wrapper.mWriters);

    if (!success)
        mResult = -1;
}

// static
int VThread::userMain(int argc, char** argv) {
    VException::installWin32SEHandler(); // A no-op if not configured to be used.

#ifdef VAULT_MEMORY_ALLOCATION_TRACKING_SUPPORT
    VMemoryTracker memoryTracker;
#endif

    int    result = -1;
    App    app(argc, argv);

    VLOGGER_INFO("Platform check starting."); // Ensures that we create a logger, which will ensure we exercise shutdown machinery.

    try {
        app.run();
        result = app.getResult();
    } catch (const VException& ex) {
        std::cout << "ERROR: Caught VException (" << ex.getError() << "): '" << ex.what() << "'\n";
    } catch (const std::exception& ex) {
        std::cout << "ERROR: Caught STL exception: '" << ex.what() << "'\n";
    }

    if (result == 0)
        std::cout << "SUCCESS: Platform check completed with result 0." << std::endl;
    else
        std::cout << "ERROR: Platform check completed with result " << result << "." << std::endl;

    VShutdownRegistry::shutdown();

    return result;
}

int main(int argc, char** argv) {
    VMainThread mainThread;
    return mainThread.execute(argc, argv);
}

