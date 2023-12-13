// circBuf.c

#include <vpi/circBuf.h>
#include <vpi/os.h> // for critical regions
#include <string.h>



#ifndef     VP_CIRCBUF_MOVE_SIZE
    #define VP_CIRCBUF_MOVE_SIZE 8
#endif



#define MIN_(a, b) ( (a) < (b)? (a): (b) )



static const VPCircBuf_Element* findElement(const VPCircBuf_Element* s, VPCircBuf_Element x, size_t n)
{
    const VPCircBuf_Element* stop = s + n;

    for(; s < stop; ++s)
        if( *s == x )
            return s;

    return NULL;
}



void vpCircBuf_init(VPCircBuf* p, VPCircBuf_Element* buffer, size_t bufSize_elements)
{
    REQUIRE(p);
    REQUIRE(buffer);
    REQUIRE(bufSize_elements > 0);

    vpOs_init();

    p->start = buffer;
    p->stop  = buffer + bufSize_elements;
    p->head  = NULL;
    p->tail  = NULL;
}



bool vpCircBuf_isEmpty(VPCircBuf* p)
{
    bool b;

    REQUIRE(p);

    vpOs_criticalRegionEnter();
        b = !p->head;
    vpOs_criticalRegionLeave();

    return b;
}



bool vpCircBuf_isFull(VPCircBuf* p)
{
    bool b;

    REQUIRE(p);

    vpOs_criticalRegionEnter();

        ASSERT(p->tail || !p->head);

        b = p->head  &&  p->head == p->tail;

    vpOs_criticalRegionLeave();

    return b;
}



size_t vpCircBuf_count(VPCircBuf* p)
{
    ptrdiff_t count_elements = 0;

    REQUIRE(p);
    REQUIRE(p->start);
    REQUIRE(p->stop);

    vpOs_criticalRegionEnter();

        ASSERT(!p->head
            ||  (  p->head >= p->start
                && p->head <  p->stop
                && p->tail >= p->start
                && p->tail <  p->stop
                )
            );

        if( p->head /* i.e., not empty */ )
        {
            count_elements = p->tail - p->head;

            if( count_elements <= 0 )
                count_elements += p->stop - p->start;
        }

    vpOs_criticalRegionLeave();

    return count_elements;
}



size_t vpCircBuf_freeCount(VPCircBuf* p)
{
    REQUIRE(p);
    REQUIRE(p->start);
    REQUIRE(p->stop);

    return p->stop - p->start - vpCircBuf_count(p);
}



size_t vpCircBuf_put(VPCircBuf* p, const VPCircBuf_Element* buffer, size_t count_elements)
{
    /* case empty (null head):
     *     before: start .............................................. stop
     *     after:  start,head <-new-data-> tail ....................... stop
     *
     * case head > tail:
     *     before: start --data-> tail .................. head <-data-- stop
     *     after:  start --data-> <-new-data-> tail ..... head <-data-- stop
     *
     * case head < tail:
     *     before: start ................... head <--data--> tail ..... stop
     *     after:  start -new-data> tail ... head <--data--> <new-data- stop
     *
     * case full (head == tail):
     *     before: start ------data------> head,tail <------data------- stop
     *     after:  start ------data------> head,tail <------data------- stop
     */

    size_t                   n;
    size_t                   total            = 0;
    const VPCircBuf_Element* buf              = (const VPCircBuf_Element*)buffer;
    VPCircBuf_Element*       upperRegionStart = NULL;
    VPCircBuf_Element*       upperRegionStop;
    VPCircBuf_Element*       lowerRegionStart = NULL;
    VPCircBuf_Element*       lowerRegionStop;

    REQUIRE(p);
    REQUIRE(p->start);
    REQUIRE(p->stop);

    vpOs_criticalRegionEnter();

        ASSERT(!p->head
            ||  (  p->head >= p->start
                && p->head <  p->stop
                && p->tail >= p->start
                && p->tail <  p->stop
                )
            );

        if( !p->head )
        {
            p->head          = p->start;
            lowerRegionStart = p->start;
            lowerRegionStop  = p->stop;
        }
        else if( p->head > p->tail )
        {
            lowerRegionStart = p->tail;
            lowerRegionStop  = p->head;
        }
        else if( p->head < p->tail )
        {
            upperRegionStart = p->tail;
            upperRegionStop  = p->stop;
            lowerRegionStart = p->start;
            lowerRegionStop  = p->head;
        }

        if( upperRegionStart )
        {
            n = MIN_(count_elements, (size_t)(upperRegionStop - upperRegionStart));
            memcpy(upperRegionStart, buf, n * sizeof *upperRegionStart);
            total += n;
            p->tail = upperRegionStart + n;
        }

        if( lowerRegionStart  &&  total < count_elements )
        {
            n = MIN_(count_elements - total, (size_t)(lowerRegionStop - lowerRegionStart));
            memcpy(lowerRegionStart, buf + total, n * sizeof *lowerRegionStart);
            total += n;
            p->tail = lowerRegionStart + n;
        }

        if( p->tail == p->stop )
            p->tail =  p->start;

    vpOs_criticalRegionLeave();
        /* Note: the head and tail pointers are declared volatile (in circBuf.h) so that at this point
         *     (i.e., leaving critical region), the pointer updates are not held pending (e.g., in registers)
         *     until the return of the function. In the event that an interrupt was pending at this point,
         *     it's important that the head/tail updates have been applied before any ISR uses them.
         *     (The data in the actual buffer is not volatile, but changes to it are not held pending because
         *     access to the buffer are bracketed by accesses of the volatile head/tail pointers.
         *     (This comment applies to all of the functions that modify the head and/or tail pointers.)
         */

    xTRACE(("\t\t%p:%p\n", p->head, p->tail));

    return total;
}



size_t vpCircBuf_putAll(VPCircBuf* p, const VPCircBuf_Element* buffer, size_t count_elements)
{
    bool                     ok;
    const VPCircBuf_Element* buf = (const VPCircBuf_Element*)buffer;

    vpOs_criticalRegionEnter();

        ok = vpCircBuf_freeCount(p) >= count_elements;

        if( ok )
        {
            size_t n = vpCircBuf_put(p, buf, count_elements);
            ASSERT(n == count_elements);
        }

    vpOs_criticalRegionLeave();

    return ok? count_elements: 0;
}



bool vpCircBuf_putElement(VPCircBuf* p, VPCircBuf_Element x)
{
    return vpCircBuf_putAll(p, &x, 1) == 1;
}



size_t vpCircBuf_get(VPCircBuf* p, VPCircBuf_Element* buffer, size_t count_elements)
{
    /* case empty (null head):
     *     before: start .............................................. stop
     *     after:  start .............................................. stop
     *
     * case head < tail:
     *     before: start ... head <--------data--------> tail ......... stop
     *     after:  start ............. head <---data---> tail ......... stop
     *
     * case head >= tail:
     *     before: start -------data-----> tail ... head <-----data---- stop
     *     after1: start -------data-----> tail .......... head <-data- stop
     *     after2: start ... head <-data-> tail ....................... stop
     */

    size_t              n;
    size_t              total            = 0;
    VPCircBuf_Element*  upperRegionStart = NULL;
    VPCircBuf_Element*  upperRegionStop;
    VPCircBuf_Element*  lowerRegionStart = NULL;
    VPCircBuf_Element*  lowerRegionStop;

    REQUIRE(p);
    REQUIRE(p->start);
    REQUIRE(p->stop);

    vpOs_criticalRegionEnter();
       if( p->head )
       {
            ASSERT(!p->head
                ||  (  p->head >= p->start
                    && p->head <  p->stop
                    && p->tail >= p->start
                    && p->tail <  p->stop
                    )
                );

            if( p->head < p->tail )
            {
                lowerRegionStart = p->head;
                lowerRegionStop  = p->tail;
            }
            else
            {
                upperRegionStart = p->head;
                upperRegionStop  = p->stop;
                lowerRegionStart = p->start;
                lowerRegionStop  = p->tail;
            }

            if( upperRegionStart )
            {
                n = MIN_(count_elements, (size_t)(upperRegionStop - upperRegionStart));
                memcpy(buffer, upperRegionStart, n * sizeof *upperRegionStart);
                total += n;
                p->head = upperRegionStart + n;
            }

            if( lowerRegionStart  &&  total < count_elements )
            {
                n = MIN_(count_elements - total, (size_t)(lowerRegionStop - lowerRegionStart));
                memcpy(buffer + total, lowerRegionStart, n * sizeof *buffer);
                total += n;
                p->head = lowerRegionStart + n;
            }

            if( p->head == p->stop )
                p->head =  p->start;

            if( p->head == p->tail )
                p->head =  NULL;
        }
    vpOs_criticalRegionLeave();

    xTRACE(("\t\t%p:%p\n", p->head, p->tail));

    return total;
}



bool vpCircBuf_getAll(VPCircBuf* p, VPCircBuf_Element* buffer, size_t count_elements)
{
    bool ret;

    vpOs_criticalRegionEnter();

        ret = vpCircBuf_count(p) >= count_elements;

        if( ret )
        {
            size_t n = vpCircBuf_get(p, buffer, count_elements);
            ASSERT(n == count_elements);
        }

    vpOs_criticalRegionLeave();

    return ret;
}



bool vpCircBuf_getElement(VPCircBuf* p, VPCircBuf_Element* buffer)
{
    return vpCircBuf_getAll(p, buffer, 1);
}



void vpCircBuf_accessPutBuffer(VPCircBuf* p, VPCircBuf_Element** buffer, size_t* size_elements)
{
    /* case empty (null head):
     *     before: start .............................................. stop
     *     after:  start,head <-new-data-> tail ....................... stop
     *
     * case head > tail:
     *     before: start --data-> tail .................. head <-data-- stop
     *     after:  start --data-> <-new-data-> tail ..... head <-data-- stop
     *
     * case head < tail:
     *     before: start ................... head <--data--> tail ..... stop
     *     after:  start -new-data> tail ... head <--data--> <new-data- stop
     *
     * case full (head == tail):
     *     before: start ------data------> head,tail <------data------- stop
     *     after:  start ------data------> head,tail <------data------- stop
     */

    VPCircBuf_Element* buf;

    REQUIRE(p);
    REQUIRE(p->start);
    REQUIRE(p->stop);
    REQUIRE(buffer);
    REQUIRE(size_elements);

    vpOs_criticalRegionEnter();

        ASSERT(!p->head
            ||  (  p->head >= p->start
                && p->head <  p->stop
                && p->tail >= p->start
                && p->tail <  p->stop
                )
            );

        if( !p->head )
        {
            buf            = p->start;
            *size_elements = p->stop - p->start;
        }
        else
        {
            const VPCircBuf_Element* stop = p->head < p->tail? p->stop: p->head;

            buf            = p->tail;
            *size_elements = stop - buf;
        }

    vpOs_criticalRegionLeave();

    *buffer = buf;
}



void vpCircBuf_commitPutBuffer(VPCircBuf* p, VPCircBuf_Element* buffer, size_t count_elements)
{
    /* case empty (null head):
     *     before: start .............................................. stop
     *     after:  start,head <-new-data-> tail ....................... stop
     *
     * case head > tail:
     *     before: start --data-> tail .................. head <-data-- stop
     *     after:  start --data-> <-new-data-> tail ..... head <-data-- stop
     *
     * case head < tail:
     *     before: start ................... head <--data--> tail ..... stop
     *     after:  start -new-data> tail ... head <--data--> <new-data- stop
     *
     * case full (head == tail):
     *     before: start ------data------> head,tail <------data------- stop
     *     after:  start ------data------> head,tail <------data------- stop
     */

    VPCircBuf_Element* buf;

    REQUIRE(p);
    REQUIRE(p->start);
    REQUIRE(p->stop);
    REQUIRE(buffer);
    REQUIRE(count_elements <= vpCircBuf_freeCount(p));

    #ifndef NDEBUG
        {
            VPCircBuf_Element* q;
            size_t             n;
            vpCircBuf_accessPutBuffer(p, &q, &n);
            REQUIRE((vpCircBuf_isEmpty(p)  ||  buffer == q)  &&  count_elements <= n);
        }
    #endif

    buf = buffer;

    vpOs_criticalRegionEnter();

        ASSERT(!p->head
            ||  (  p->head >= p->start
                && p->head <  p->stop
                && p->tail >= p->start
                && p->tail <  p->stop
                )
            );

        if( !p->head )
        {
            p->head = buf;
            p->tail = buf + count_elements;
        }
        else
        {
            p->tail += count_elements;
        }

        ASSERT(p->tail <= p->stop);
        if( p->tail == p->stop )
            p->tail =  p->start;

    vpOs_criticalRegionLeave();

    ENSURE(p->tail < p->stop  &&  p->tail >= p->start);
}



size_t vpCircBuf_move(VPCircBuf* dest, VPCircBuf* source, size_t maxCount)
{
    //TODO (when extra performance is needed): reimplement in terms of (newer) lock/unlock API, avoiding temporary buffer

    VPCircBuf_Element buf[VP_CIRCBUF_MOVE_SIZE];
    size_t            sourceCount;
    size_t            destAvail;
    size_t            total = 0;
    size_t            n;

    REQUIRE(dest && source);

    vpOs_criticalRegionEnter();

        sourceCount = vpCircBuf_count(source);
        destAvail   = vpCircBuf_freeCount(dest);

        for(;;)
        {
            n = MIN_( maxCount - total, MIN_(sourceCount, destAvail) );
            xTRACE(("vpCircBufMove: n=%d (%d-%d %d %d)\n", n, maxCount, total, sourceCount, destAvail));
            if( n == 0 )
                break;

            vpCircBuf_get(source, buf, n);
            vpCircBuf_put(dest,   buf, n);

            total       += n;
            sourceCount -= n;
            destAvail   -= n;
        }

    vpOs_criticalRegionLeave();

    return total;
}



bool vpCircBuf_findElementFrom(VPCircBuf* p, VPCircBuf_Element c, size_t* index, size_t startIndex)
{
    const VPCircBuf_Element* found = NULL;

    REQUIRE(p);
    REQUIRE(p->start);
    REQUIRE(p->stop);

    vpOs_criticalRegionEnter();

        if( p->head )
        {
            VPCircBuf_Element*  upperRegionStart = NULL;
            VPCircBuf_Element*  upperRegionStop;
            VPCircBuf_Element*  lowerRegionStart = NULL;
            VPCircBuf_Element*  lowerRegionStop;
            size_t              upperRegionSize  = 0;
            size_t              lowerRegionSize  = 0;

            ASSERT(    p->head >= p->start
                    && p->head <  p->stop
                    && p->tail >= p->start
                    && p->tail <  p->stop
                  );

            if( p->head < p->tail )
            {
                lowerRegionStart = p->head;
                lowerRegionStop  = p->tail;
            }
            else
            {
                upperRegionStart = p->head;
                upperRegionStop  = p->stop;
                upperRegionSize  = upperRegionStop - upperRegionStart;

                lowerRegionStart = p->start;
                lowerRegionStop  = p->tail;
            }

            lowerRegionSize = lowerRegionStop - lowerRegionStart;

            if( upperRegionStart )
            {
                if( startIndex >= upperRegionSize )
                    startIndex -= upperRegionSize;
                else
                {
                    found = findElement(upperRegionStart + startIndex, c, upperRegionSize - startIndex);

                    if( found && index )
                        *index = found - upperRegionStart;

                    startIndex = 0;
                }
            }

            if( !found  &&  lowerRegionStart  &&  startIndex < lowerRegionSize )
            {
                found = findElement(lowerRegionStart + startIndex, c, lowerRegionSize - startIndex);

                if( found && index )
                    *index = upperRegionSize + (found - lowerRegionStart);
            }
        }

    vpOs_criticalRegionLeave();

    return !!found;
}



bool vpCircBuf_findElement(VPCircBuf* p, VPCircBuf_Element x, size_t* index)
{
    return vpCircBuf_findElementFrom(p, x, index, 0);
}



void vpCircBuf_clear(VPCircBuf* p)
{
    REQUIRE(p);

    vpOs_criticalRegionEnter();

        p->head = NULL;

    vpOs_criticalRegionLeave();
}



//EOF
