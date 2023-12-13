// vpi/os.h
// declares platform-independent thread and IPC functions
//
// Note: all functions are not implemented for all platforms.
//     (You'll get a linker error if you try to use unimplemented functions.)
//
// Note: os objects may consume a small amount of heap and platform-specific resources.
//     Failure to allocate such resources are nonrecoverable errors trapped by assert().
//
// Configuration constants (VP_CONFIG_H):
//     VPOS_PLAT_H (symbol, included at bottom of this header to define platform-specific optimizations)
//     VPOS_ISR_USE_VPEVENT (symbol, if defined, voOs_isrEnter/Leave use vpEvent_isrEnter/Leave)
//
// Copyright 1997-2005 by Visionary Products, Inc.
// All rights are reserved.



#if !defined VPI_os_h
    #define  VPI_os_h

    #include <time.h>
    #include <vpi/standard.h>
    #include "os_.h"



    #if defined __cplusplus
        extern "C"
        {
    #endif



    void           vpOs_init(void);
        // call once before any other vpOs_ function



    // Mutex

    typedef struct VPOs_MutexData*       VPOs_Mutex;

    VPOs_Mutex     vpOs_mutexCreate     (void);

    void           vpOs_mutexDestroy    (VPOs_Mutex m);

    bool           vpOs_mutexTryLock    (VPOs_Mutex m);
        // returns true iff lock was attained

    void           vpOs_mutexWaitAndLock(VPOs_Mutex m);

    void           vpOs_mutexUnlock     (VPOs_Mutex m);



    // Critical Region (implemented as a system mutex, spin-lock, or by disabling interrupts)
    // Enter/Leave can nest, i.e., the region can be re-Entered and requires a corresponding number of Leaves.

    void           vpOs_criticalRegionEnter(void);

    void           vpOs_criticalRegionLeave(void);



    // ISR enter/leave

    void           vpOs_isrEnter(void);  // call at start of every ISR
    void           vpOs_isrLeave(void);  // call at end of every ISR



    // Semaphore (counting semaphore)

    typedef struct VPOs_SemaphoreData*      VPOs_Semaphore;

    VPOs_Semaphore vpOs_semaphoreCreate    (int initialCount);

    void           vpOs_semaphoreDestroy   (VPOs_Semaphore sem);

    void           vpOs_semaphoreSignal    (VPOs_Semaphore sem);

    void           vpOs_semaphoreWait      (VPOs_Semaphore sem);

    bool           vpOs_semaphoreTryWait   (VPOs_Semaphore sem);
        // iff semaphore was signaled, decrements the semaphore and returns true

    bool           vpOs_semaphoreTryWait_ms(VPOs_Semaphore sem, time_t timeout_ms);
        // like TryWait, above, but waits up to timeout_ms for sem to become signaled

    int            vpOs_semaphoreCount     (VPOs_Semaphore sem);



    // sleep / time

    void           vpOs_sleep_ms    (time_t ms  );
    void           vpOs_sleep_usec  (time_t usec);
    void           vpOs_sleep_ns    (time_t ns  );
        // May be called by any thread to make the calling thread sleep for at least specified time
        // If parameter < 0, the function returns immediately.
        // Important note: small duration sleeps may be busy waits.

    void           vpOs_setPeriod_ms(time_t ms);
        // similar to sleep_ms but attempts to maintian a regular period by deducting elapsed time since last call
        //
        // actual time between calls must be <= TIME_MAX ms; otherwise undefined behavior results (due to wrap)
        //
        // compare example times for repeated calls to sleep_ms and setPeriod_ms each with 10 ms specified:
        //             sleep                     setPeriod
        //     call time   return time     call time   return time
        //             0            10             0            10
        //            12            22            12            20
        //            24            35            22            31
        //            35            45            31            40
        //            50            60            45            50
        //            60            75            50            65
        //           100           110            90            90
        //           110           120            90            90
        //           120           130            90            90
        //           130           140            90           100

    void           vpOs_delay_ms    (time_t ms  );
    void           vpOs_delay_usec  (time_t usec);
    void           vpOs_delay_ns    (time_t ns  );
        // these are the same as the sleep functions except that they are
        // always busy waits

    time_t         vpOs_time_sec (void);
    time_t         vpOs_time_ms  (void);
    time_t         vpOs_time_usec(void);

    void           vpOs_setTime_sec(time_t sec);



    // threads

    VPOs_ThreadData vpOs_beginThread(void (*func)(void *), size_t desiredStackSize, void* arg);
        // desiredStackSize of 0 selects default;
        // some platforms may ignore desired stack size

    void            vpOs_joinThread(VPOs_ThreadData data);


    // Signals

    typedef void (*VPOs_SignalFunc)(int);



    typedef enum VPOs_SignalId_
    {
        vpOs_signalId_user1,
        vpOs_signalId_user2,
        vpOs_signalId_childTerminated
    } VPOs_SignalId;



    VPOs_SignalFunc vpOs_signalRegister(VPOs_SignalId signalId, VPOs_SignalFunc handler);
    int             vpOs_signalPost(int processId, VPOs_SignalId signalId);
    int             vpOs_signalPostAll(const char* processName, VPOs_SignalId signalId);



    #if defined __cplusplus
        } //extern "C"
    #endif



    #ifdef       VPOS_PLAT_H
        #include VPOS_PLAT_H
    #endif



    #ifdef VPOS_ISR_USE_VPEVENT
        #include <vpi/event.h>

        #define vpOs_isrEnter()  vpEvent_isrEnter()
        #define vpOs_isrLeave()  vpEvent_isrLeave()
    #endif



#endif
