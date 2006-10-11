/*
Copyright c1997-2006 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.5
http://www.bombaydigital.com/
*/

/** @file */

/*
This file is where you can define the switches that control
which features are enabled or disabled in the Vault library.
For example, you can enable native strings support for Qt and
for Mac OS X Core Foundation, and you can even disable the
use of the sprintf-like string formatting APIs in favor of
the boost::format interface.

Feel free to customize this file. It just contains a default
configuration.
*/

#define VAULT_VARARG_STRING_FORMATTING_SUPPORT
//#define VAULT_BOOST_STRING_FORMATTING_SUPPORT
//#define VAULT_QT_SUPPORT
