/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.1
http://www.bombaydigital.com/
*/

/** @file */

#include "vstringiterator.h"

#include "vexception.h"

void VStringIteratorThrowOutOfBoundsBegin() {
    throw VRangeException(VSTRING_COPY("Attempt to iterate backward beyond start of string."));
}

void VStringIteratorThrowOutOfBoundsEnd() {
    throw VRangeException(VSTRING_COPY("Attempt to iterate forward beyond end of string."));
}
