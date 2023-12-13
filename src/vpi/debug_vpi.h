/* debug.h
 * TRACE/ASSERT (DbC) macros
 *
 * Configuration constants (VP_CONFIG_H):
 *     NDEBUG             (symbol, disables assert, DBG, and stack-checking macros, NOT RECOMMENDED)
 *     VP_NASSERT         (synbol, disables VP_LOWLEVEL_ASSERT (and thus normal ASSERT, REQUIRE, ENSURE, EXPECT))
 *     VP_NDBG            (symbol, disables DBG macro)
 *     VP_NTRACE          (symbol, disables TRACE macro)
 *     VP_NCHECK_STACK    (symbol, disables stack-checking macros)
 *     VP_NEXPECT         (symbol, disables EXPECT macro)
 *     VP_WARN_EXPECT     (symbol, causes EXPECT to print warnings messages (formatted like compiler messages))
 *     VP_LOWLEVEL_ASSERT (macro; provides alternative (application-specific) implementation (affects REQUIRE, ENSURE, ASSERT, EXPECT))
 *     VP_ASSERT          (macro; provides alternative (application-specific) implementation)
 *     VP_REQUIRE         (macro; provides alternative (application-specific) implementation)
 *     VP_ENSURE          (macro; provides alternative (application-specific) implementation)
 *     VP_EXPECT          (macro; provides alternative (application-specific) implementation)
 *     VP_TRACE           (macro; see below)
 *     VP_TEST            (symbol, enables WARN_EXPECT and changes __MFC assert to similar output format)
 *     VP_NO_SHORT_NAMES  (symbol, disables short names (e.g., REQUIRE for VP_REQUIRE))
 *     __MFC              (symbol, with VP_TEST, use stdout rather than msgbox for assertion failures)
 *
 * Copyright 1997-2006 by Visionary Products, Inc.
 * All rights are reserved.
 */



#if !defined    VPI_debug_h
    #define     VPI_debug_h

    #include <stdlib.h>

    #ifndef __cplusplus
        #include <stdbool.h>
    #endif



    #ifndef     VP_LOWLEVEL_ASSERT
        #include <assert.h>
        #define VP_LOWLEVEL_ASSERT(x)      assert(x)
    #endif

    #ifdef      VP_NASSERT
        #undef  VP_LOWLEVEL_ASSERT
        #define VP_LOWLEVEL_ASSERT(x)      ((void)0)
    #endif



    #ifndef     VP_ASSERT
        #define VP_ASSERT(x)               VP_LOWLEVEL_ASSERT(x)
    #endif

    #ifndef     VP_REQUIRE
        #define VP_REQUIRE(x)              VP_LOWLEVEL_ASSERT(x)
    #endif

    #ifndef     VP_ENSURE
        #define VP_ENSURE(x)               VP_LOWLEVEL_ASSERT(x)
    #endif

    #ifndef     VP_EXPECT
        #define VP_EXPECT(x)               VP_LOWLEVEL_ASSERT(x)
    #endif

    #define     VP_CASSERT(x)              { typedef char VP_CAssert[(x)? 1: -1]; }

    #define     VP_EASSERT(x)              ( ASSERT(x), 0 ) /* ASSERT usable as an expression */



    #define     VP_ASSERT_NOTHING          true /* for VP_REQUIRE(VP_ASSERT_NOTHING), for example */



    /* By defining the VP_TRACE macro, any platform-specific trace printing may
     *     be implemented.
     * See the definition of TRACE() to see how these macros are used.
     */
    #ifndef     VP_TRACE
        #include <stdio.h>
        #define VP_TRACE(x)                (void)( printf x, fflush(stdout) )
    #endif



    #ifdef      NDEBUG
        #ifndef          VP_NDBG
            #define      VP_NDBG
        #endif
        #ifndef          VP_NCHECK_STACK
            #define      VP_NCHECK_STACK
        #endif
        #ifndef          VP_NTRACE
            #define      VP_NTRACE
        #endif
    #endif



    #define     xVP_DBG(x)                 ((void)0)

    #ifdef      VP_NDBG
        #define VP_DBG(x)                  ((void)0)
    #else
        #define VP_DBG(x)                  (x)
    #endif



    #ifdef      VP_NTRACE
        #undef  VP_TRACE
        #define VP_TRACE(s)                ((void)0)
    #endif

    #define     xVP_TRACE(s)               ((void)0)



    // "OLD" useful for post-condition checks for transformative functions
    #ifndef     NDEBUG
        #define VP_OLD(  type, var )        type old_##var = var
        #define VP_OLD_P(type, p   )        type old_##p   = *p
        #define VP_OLD_F(type, f   )        type old_##f   = f()
    #else
        #define VP_OLD(  type, var )        ((void)0)
        #define VP_OLD_P(type, p   )        ((void)0)
        #define VP_OLD_F(type, f   )        ((void)0)
    #endif



    #ifdef      VP_TEST
        #ifndef          VP_WARN_EXPECT
            #define      VP_WARN_EXPECT
        #endif
    #endif

    #ifdef      VP_WARN_EXPECT
        #undef  VP_EXPECT
        #define VP_EXPECT(b)                                                                \
                    (void)( (b)                                                             \
                          ||  ( VP_TRACE(("\n" __FILE__ "(%d):  EXPECT( " #b " ) failed\n", \
                                          __LINE__                                          \
                                        ))                                                  \
                              , true                                                        \
                              )                                                             \
                          )
    #endif

    #ifdef      VP_NEXPECT
        #undef  VP_EXPECT
        #define VP_EXPECT(x)               ((void)0)
    #endif



    #ifdef      VP_NCHECK_STACK
        #define VP_VALID_STACK()           ( true  )
        #define VP_CHECK_STACK()           ((void)0)
    #else
        #ifdef  __cplusplus
            extern "C"
        #endif
            int vp_validStack(void);
            /* provided by startup/run-time code;
             * returns zero if stack allocation is invalid (e.g., stack/heap overlap)
             */

        #define VP_VALID_STACK()           ( vp_validStack() )

        #define VP_CHECK_STACK()           VP_ASSERT( VP_VALID_STACK() )
            /* used to implement run-time stack checking via ASSERT
             */
    #endif



    #ifndef     VP_NO_SHORT_NAMES
        #define REQUIRE(x)          VP_REQUIRE(x)
        #define ENSURE( x)          VP_ENSURE( x)
        #define EXPECT( x)          VP_EXPECT( x)
        #define ASSERT( x)          VP_ASSERT( x)
        #define CASSERT(x)          VP_CASSERT(x)
        #define EASSERT(x)          VP_EASSERT(x)

        #define NOTHING             VP_ASSERT_NOTHING

        #define TRACE(s)            VP_TRACE(s)

        #define DBG(   x)           VP_DBG(   x)
        #define xDBG(  x)           xVP_DBG(  x)
        #define xTRACE(x)           xVP_TRACE(x)

        #define OLD(  type, var )   VP_OLD(  type, var )
        #define OLD_P(type, p   )   VP_OLD_P(type, p   )
        #define OLD_F(type, f   )   VP_OLD_F(type, f   )

        #define VALID_STACK()       VP_VALID_STACK()
        #define CHECK_STACK()       VP_CHECK_STACK()
    #endif



#endif
