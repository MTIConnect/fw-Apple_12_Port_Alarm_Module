/* vpi/circBuf.h
 *
 * Circular Buffer API
 *
 * Configuration constants (VP_CONFIG_H):
 *     VP_CIRCBUF_MOVE_SIZE size in elements of automatic buffer used in rtCircBuf_move, default 8
 *     VP_CIRCBUF_ELEMENT_TYPE typename for circBuf element type, default char
 *
 * Copyright 2006 by VPI Engineering
 * All rights are reserved.
 */



#if !defined VPI_circBuf_h
    #define  VPI_circBuf_h



    #include <vpi/standard.h>



    #ifdef __cplusplus
        extern "C"
        {
    #endif



    #if !defined VP_CIRCBUF_ELEMENT_TYPE
        typedef  char                    VPCircBuf_Element;
    #else
        typedef  VP_CIRCBUF_ELEMENT_TYPE VPCircBuf_Element;
    #endif



    typedef struct
    {
        VPCircBuf_Element*          start;
        VPCircBuf_Element*          stop; /* points just beyond end of buffer                             */
        VPCircBuf_Element* volatile head; /* points to item to read next,   NULL means empty              */
        VPCircBuf_Element* volatile tail; /* points to item to write next, !NULL && head==tail means full */
    } VPCircBuf;



    void   vpCircBuf_init              (VPCircBuf* p, VPCircBuf_Element* buf, size_t bufSize_elements);
        /* must be called (once) before any other vpCircBuf_ functions are called
         */

    bool   vpCircBuf_isEmpty           (VPCircBuf* p);

    bool   vpCircBuf_isFull            (VPCircBuf* p);

    size_t vpCircBuf_count             (VPCircBuf* p);
        /* returns number of elements queued (i.e., used space)
         */

    size_t vpCircBuf_freeCount         (VPCircBuf* p);
        /* returns number of elements available (i.e., ununused space)
         */

    size_t vpCircBuf_put               (VPCircBuf* p, const VPCircBuf_Element* buf, size_t maxSize_elements);
        /* puts up to count elements (count must be <= SSIZE_MAX); returns count put
         */

    bool   vpCircBuf_putElement        (VPCircBuf* p, VPCircBuf_Element x);
        /* puts a single element (all or nothing)
         */

    size_t vpCircBuf_putAll            (VPCircBuf* p, const VPCircBuf_Element* buf, size_t size_elements);
        /* like vpCircBuf_Put but puts either all or nothing
         */

    size_t vpCircBuf_get               (VPCircBuf* p, VPCircBuf_Element* buf, size_t maxSize_elements);
        /* gets up to count elements (count must be <= SSIZE_MAX); returns count got
         */

    bool   vpCircBuf_getElement        (VPCircBuf* p, VPCircBuf_Element* x);
        /* gets a single element (all or nothing)
         */

    bool   vpCircBuf_getAll            (VPCircBuf* p, VPCircBuf_Element* buf, size_t size_elements);
        /* like vpCircBuf_Get but gets either all or nothing
         */

    void   vpCircBuf_accessPutBuffer   (VPCircBuf* p, VPCircBuf_Element** buf, size_t* size_elements);
        /* obtains access to largest contiguous available block for put, returns pointer and size of the buffer;
         * application code may then fill the buffer and must finally call commitPutBuffer;
         * the block will not be read (via get/getAll until it is committed),
         *     but the data already in the circBuf may;
         * put/putAll may not be called until commitPutBuffer has been called
         */

    void   vpCircBuf_commitPutBuffer   (VPCircBuf* p, VPCircBuf_Element* buf, size_t count_elements);
        /* commits put of count elements at the beginning of buf,
         *     which must have been obtained from previous call to accessPutBuffer
         */

    size_t vpCircBuf_move              (VPCircBuf* dest, VPCircBuf* source, size_t maxCount_elements);
        /* gets up to maxCount elements from source and puts them to dest;
         * returns actual count moved (which will be the lesser of source count or dest avail)
         */

    bool   vpCircBuf_findElement       (VPCircBuf* p, VPCircBuf_Element x, size_t* index);
        /* returns nonzero if the circBuf contains the specified element (such as a delimiter);
         * if x is found and index is not null, *index is set to the index of the first occurrence of x
         */

    bool   vpCircBuf_findElementFrom   (VPCircBuf* p, VPCircBuf_Element x, size_t* index, size_t startIndex);
        /* like findElement but starts the search at startIndex;
         * (*index is relative to entire buffer, not to startIndex);
         * if startIndex is beyond the end of the buffer, false is returned
         */

    void   vpCircBuf_clear             (VPCircBuf* p);
        /* discards all data in the buffer
         */



    #ifdef __cplusplus
        } // extern "C"
    #endif



#endif
