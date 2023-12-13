/* standard.h
 * includes standard include files common to all VPI code
 *
 * Configuration constants (most normally defined in VP_CONFIG_H):
 *     VP_CONFIG_H:   specifies configuration include file, e.g., "config_xyz.h"
 *     VP_STDBOOL_H:  (for pre-C99 compilers) specifies stdbool header file, e.g., "_stdbool/stdbool.h"
 *     VP_STDINT_H:   (for pre-C99 compilers) specifies stdint header file, e.g., "_norm16/stdint.h"
 *     VP_SYSTYPES_H: specifies sys/types.h header, e.g., "_norm16/sys/types.h"
 *
 * Copyright 1997-2006 by VPI Engineering
 * All rights are reserved.
 */

#if !defined VPI_standard_h
    #define  VPI_standard_h

    #ifdef       VP_CONFIG_H
        #include VP_CONFIG_H
    #endif



    #define VP_MAX(a, b) ( (a) < (b)? (b): (a) )
    #define VP_MIN(a, b) ( (b) < (a)? (b): (a) )



    #ifdef __cplusplus
        #include <climits>
    #else
        #include <limits.h>

        #ifdef       VP_STDBOOL_H
            #include VP_STDBOOL_H
        #else
            #include <stdbool.h>
        #endif
    #endif

    #ifdef       VP_STDINT_H
        #include VP_STDINT_H
    #else
        #include <stdint.h>
    #endif

    #ifdef       VP_SYSTYPES_H
        #include VP_SYSTYPES_H
    #else
        #include <sys/types.h>          /* for ssize_t */
    #endif

    #include <stddef.h>
    #include <ctype.h>



    #include "debug_vpi.h"



    #ifdef VP_NONSTANDARD_CTYPE
        #define VP_CYTPE_UNSPECIFIED    1
        #define VP_CTYPE_FORCE_UNSIGNED 2

        #if VP_NONSTANDARD_CTYPE == VP_CTYPE_FORCE_UNSIGNED
            #define isdigit( x)  ( isdigit( (unsigned char)(x)) )
            #define isxdigit(x)  ( isxdigit((unsigned char)(x)) )
            //TODO: others as needed
        #else
            #error please define VP_NONSTANDARD_CTYPE to a supported value
        #endif
    #endif



    enum Standard_Constants
    {
        unity_percent = 100,  // deprecated

        standard_dummy
    };



#endif
