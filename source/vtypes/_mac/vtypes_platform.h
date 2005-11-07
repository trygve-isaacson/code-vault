/*
Copyright c1997-2005 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.3.2
http://www.bombaydigital.com/
*/

#ifndef vtypes_platform_h
#define vtypes_platform_h

/** @file */

//define VTARGET_CARBON 1

#ifdef VTARGET_CARBON
    #include <types.h>
#else
    #include <sys/types.h>
#endif

#include <sys/stat.h>

#define _BSD_WCHAR_T_DEFINED_
#ifdef _lint
typedef    _BSD_WCHAR_T_    wchar_t;
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <limits.h>

#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif

#include <unistd.h>

#define VPLATFORM_MAC

#ifdef __MWERKS__
#define VCOMPILER_CODEWARRIOR
#define VLIBRARY_METROWERKS    /* might want to consider adding a check for using CW but not MSL */
#endif

#ifndef VLIBRARY_METROWERKS
#define V_HAVE_REENTRANT_TIME    // we can and should use the _r versions of time.h calls
#endif

#define O_BINARY        0x8000        ///< Macro to define O_BINARY mode, which is not in the standard headers.

#undef FD_ZERO
#define FD_ZERO(p) memset(p, 0, sizeof(*(p)))

#define V_BYTESWAP_HTONS_GET(x)            /*lint -save -e522*/ (x) /*lint -restore */
#define V_BYTESWAP_NTOHS_GET(x)            /*lint -save -e522*/ (x) /*lint -restore */
#define V_BYTESWAP_HTONS_IN_PLACE(x)    /*lint -save -e522*/ (x) /*lint -restore */
#define V_BYTESWAP_NTOHS_IN_PLACE(x)    /*lint -save -e522*/ (x) /*lint -restore */

#define V_BYTESWAP_HTONL_GET(x)            /*lint -save -e522*/ (x) /*lint -restore */
#define V_BYTESWAP_NTOHL_GET(x)            /*lint -save -e522*/ (x) /*lint -restore */
#define V_BYTESWAP_HTONL_IN_PLACE(x)    /*lint -save -e522*/ (x) /*lint -restore */
#define V_BYTESWAP_NTOHL_IN_PLACE(x)    /*lint -save -e522*/ (x) /*lint -restore */

#define V_BYTESWAP_HTON64_GET(x)        /*lint -save -e522*/ (x) /*lint -restore */
#define V_BYTESWAP_NTOH64_GET(x)        /*lint -save -e522*/ (x) /*lint -restore */
#define V_BYTESWAP_HTON64_IN_PLACE(x)    /*lint -save -e522*/ (x) /*lint -restore */
#define V_BYTESWAP_NTOH64_IN_PLACE(x)    /*lint -save -e522*/ (x) /*lint -restore */

#define V_BYTESWAP_HTONF_GET(x)            /*lint -save -e522*/ (x) /*lint -restore */
#define V_BYTESWAP_NTOHF_GET(x)            /*lint -save -e522*/ (x) /*lint -restore */
#define V_BYTESWAP_HTONF_IN_PLACE(x)    /*lint -save -e522*/ (x) /*lint -restore */
#define V_BYTESWAP_NTOHF_IN_PLACE(x)    /*lint -save -e522*/ (x) /*lint -restore */

#ifdef VLIBRARY_METROWERKS
#define V_EFFICIENT_SPRINTF
#endif

#endif /* vtypes_platform_h */
