/*
Copyright c1997-2006 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.5
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
    
        int     mArgc;
        char**  mArgv;
        int     mResult;
    };

int main(int argc, char** argv)
    {
    int    result = -1;
    App    app(argc, argv);
    
    VLOGGER_INFO("Platform check starting."); // Ensures that we create a logger, which will ensure we exercise shutdown machinery.
    
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
    
	VShutdownRegistry::shutdown();
	
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
    bool success;
    int numSuccessfulTests;
    int numFailedTests;
    runAllVUnitTests(false, true, false, success, numSuccessfulTests, numFailedTests, NULL);

	VLogger::getLogger("VUnit")->rawLog(VString("[results] TOTAL tests passed: %d", numSuccessfulTests));
	VLogger::getLogger("VUnit")->rawLog(VString("[results] TOTAL tests failed: %d", numFailedTests));
	VLogger::getLogger("VUnit")->rawLog(VString("[results] TOTAL summary: %s", (numFailedTests == 0 ? "SUCCESS" : "FAILURE")));
    
    if (!success)
        mResult = -1;
    }

