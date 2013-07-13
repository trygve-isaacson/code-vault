/*
Copyright c1997-2013 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.0
http://www.bombaydigital.com/
*/

#ifndef vsocket_h
#define vsocket_h

/** @file */

#include "vsocketbase.h"

/*
There are a couple of Unix APIs we call that take a socklen_t parameter.
On Windows the parameter is defined as an int. The cleanest way of dealing
with this is to have our own conditionally-defined type here and use it there.
We do something similar in the _unix version of this file for HP-UX, which
also uses int instead of socklen_t.
*/
typedef int VSocklenT;

#endif /* vsocket_h */

