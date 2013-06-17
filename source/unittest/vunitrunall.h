/*
Copyright c1997-2013 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.0
http://www.bombaydigital.com/
*/

#ifndef vunitrunall_h
#define vunitrunall_h

/** @file */

#include "vunit.h"

class VTextIOStream;

extern void runAllVUnitTests(bool logOnSuccess, bool throwOnError, bool& success, int& numSuccessfulTests, int& numFailedTests, VUnitOutputWriterList* output);

#endif /* vunitrunall_h */
