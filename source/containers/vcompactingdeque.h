/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.0
http://www.bombaydigital.com/
*/

#ifndef vcompactingdeque_h
#define vcompactingdeque_h

/** @file */

#include "vtypes.h"

/**
VCompactingDeque is a template class that subclasses std::deque and adds the ability to
"compact" the deque internal structures when the deque drains. Otherwise, with
a standard deque, you can have an apparent memory leak when the queue drains but
holds onto lots of internal bookkeeping memory. You can specify in the constructor
what thresholds trigger the compaction. Compaction only can happen when you call
pop_front() or pop_back() (the methods that may shrink the queue) and do
so via a reference or pointer to this subclass. Compaction does not occur on erase()
because the returned iterator would be invalid or have to be recalculated.
*/
template < typename _Tp, typename _Alloc = std::allocator<_Tp> >
class VCompactingDeque : public std::deque<_Tp, _Alloc> {
    public:

        /**
        Constructs the deque with optional specified thresholds.
        @param  highWaterMarkRequired   compaction occurs only after the queue has grown to
            this size, and later is drained down to the lowWaterMarkRequired
        @param  lowWaterMarkRequired    compaction occurs only after the highWaterMarkRequired was
            previously reached, and the queue then drains down to this size
        */
        VCompactingDeque(size_t highWaterMarkRequired = 64, size_t lowWaterMarkRequired = 0)
            : std::deque<_Tp, _Alloc>()
            , mHighWaterMark(0)
            , mHighWaterMarkRequired(highWaterMarkRequired)
            , mLowWaterMarkRequired(lowWaterMarkRequired)
            {}
        ~VCompactingDeque() {}

        /**
        Removes the first element from the queue, possibly triggering a compaction afterward.
        You will have already called front() to get that element beforehand if you want it.
        Note that because STL templates do not use virtual methods, this override will not
        be called should you access this deque via a base class reference or pointer.
        */
        void pop_front() {
            this->_saveHighWaterMark();

            std::deque<_Tp, _Alloc>::pop_front();

            this->_compactIfReachedThreshold();
        }

        /**
        Removes the last element from the queue, possibly triggering a compaction afterward.
        You will have already called back() to get that element beforehand if you want it.
        Note that because STL templates do not use virtual methods, this override will not
        be called should you access this deque via a base class reference or pointer.
        */
        void pop_back() {
            this->_saveHighWaterMark();

            std::deque<_Tp, _Alloc>::pop_back();

            this->_compactIfReachedThreshold();
        }

        /**
        Performs compaction of the deque bookkeeping overhead.
        You can manually direct a compaction by calling this method. Not necessary if you always use
        the defined pop_front() and pop_back() functions, because they do this automatically when the
        high/low water mark thresholds are reached.
        */
        void compact() { this->_compact(); }

    private:

        void _saveHighWaterMark() {
            mHighWaterMark = V_MAX(mHighWaterMark, this->size());
        }

        void _compactIfReachedThreshold() {
            if ((mHighWaterMark >= mHighWaterMarkRequired) && (this->size() <= mLowWaterMarkRequired)) {
                this->_compact();
            }
        }

        void _compact() {
            // See: http://www.gotw.ca/gotw/054.htm for details. Swap causes a fairly efficient copy to a fresh data structure.
            std::deque<_Tp, _Alloc> temp(*this);
            temp.swap(*this);
            mHighWaterMark = 0; // Doesn't mean current size is zero, means previous hwm is obsolete.
        }

        size_t mHighWaterMark;          ///< Max size of q since last compact, but we only update during pop() calls, don't care about it on push().
        size_t mHighWaterMarkRequired;  ///< Upper q size required to trigger compaction later.
        size_t mLowWaterMarkRequired;   ///< Lower q size that triggers compaction on pop() if high water mark was hit earlier.

        friend class VMessageUnit; // Unit test can peek at our internal state between operations.
};

#endif /* vcompactingdeque_h */
