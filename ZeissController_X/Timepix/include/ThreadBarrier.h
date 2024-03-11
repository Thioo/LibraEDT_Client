/*
*  ThreadBarrier.h
*
*  Created by Chris Desjardins on 20/02/13.
*  Copyright 2013 amscins.com All rights reserved.
*
*/
#ifndef ASI_THREAD_BARRIER_Hxx
#define ASI_THREAD_BARRIER_Hxx

#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>

class AsiThreadBarrier
{
public:
    explicit AsiThreadBarrier()
        : _count(0),
        _mutex(),
        _condition()
    {
    }

    void give(int numSignals = 1)
    {
        boost::unique_lock<boost::mutex> lock(_mutex);
        _count += numSignals;
        _condition.notify_all();
    }

    void take(int numSignals = 1)
    {
        boost::unique_lock<boost::mutex> lock(_mutex);
        _count -= numSignals;
    }

    void pend()
    {
        boost::unique_lock<boost::mutex> lock(_mutex);
        while (_count < 0)
        {
            _condition.wait(lock);
        }
    }

    bool status()
    {
        bool ret = false;
        boost::unique_lock<boost::mutex> lock(_mutex);
        if (_count >= 0)
        {
            ret = true;
        }
        return ret;
    }

    int count()
    {
        return _count;
    }

private:
    volatile int _count;
    boost::mutex _mutex;
    boost::condition_variable _condition;
};

#endif
