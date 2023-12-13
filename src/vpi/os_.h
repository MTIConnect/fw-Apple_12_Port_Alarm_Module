/* vpi/os_.h
 *
 * This should only be included by os.h
 *
 * Copyright 2011 by VPI Engineering
 * All rights reserved
 */

#if !defined VPI_os__h
	#define  VPI_os__h

    #if defined VPOS_WIN32

        #include <windows.h>

        typedef HANDLE VPOs_ThreadData;

    #elif defined VPOS_POSIX

        #include <pthread.h>

        typedef pthread_t VPOs_ThreadData;

    #else

        typedef int VPOs_ThreadData;

    #endif

#endif
