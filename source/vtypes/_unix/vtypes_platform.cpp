/*
Copyright c1997-2008 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.0
http://www.bombaydigital.com/
*/

/** @file */

#include "vtypes.h"

Vs64 vault::VgetMemoryUsage()
    {
    return 0; // FIXME - find an API to use on Unix
    }

static const Vu8 kUnixLineEnding = 0x0A;

const Vu8* vault::VgetNativeLineEnding(int& numBytes)
    {
    numBytes = 1;
    return &kUnixLineEnding;
    }

