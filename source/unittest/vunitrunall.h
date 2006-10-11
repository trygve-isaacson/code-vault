/*
Copyright c1997-2006 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.5
http://www.bombaydigital.com/
*/

#ifndef vunitrunall_h
#define vunitrunall_h

/** @file */

class VTextIOStream;

extern void runAllVUnitTests(bool logToFile, bool logOnSuccess, bool throwOnError, bool& success, int& numSuccessfulTests, int& numFailedTests, VTextIOStream* xmlOutputStream);

#endif /* vunitrunall_h */
