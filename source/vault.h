/*
Copyright c1997-2008 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.0
http://www.bombaydigital.com/
*/

/** @file */

/*
This set of includes is enough to pull in all vault header files.
*/

#ifdef __cplusplus

#include "vtypes.h"
#include "vexception.h"
#include "vchar.h"
#include "vlogger.h"
#include "vsettings.h"
#include "vclassregistry.h"
#include "vsingleton.h"
#include "vsemaphore.h"
#include "vmutexlocker.h"
#include "vsocketstream.h"
#include "vsocketfactory.h"
#include "vsocketthread.h"
#include "vsocketthreadfactory.h"
#include "vbufferedfilestream.h"
#include "vdirectiofilestream.h"
#include "vmemorystream.h"
#include "vbinaryiostream.h"
#include "vtextiostream.h"
#include "vlistenersocket.h"
#include "vlistenerthread.h"
#include "vmanagementinterface.h"
#include "vbento.h"
#include "vserver.h"
#include "vclientsession.h"
#include "vmessage.h"
#include "vmessagepool.h"
#include "vmessagequeue.h"
#include "vmessagehandler.h"
#include "vmessageinputthread.h"
#include "vmessageoutputthread.h"

#endif

/**
    @defgroup toolbox Vault Toolbox (utilities)
*/
