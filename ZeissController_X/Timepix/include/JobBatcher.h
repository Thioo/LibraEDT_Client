/*
*  JobBatcher.h
*
*  Created by Chris Desjardins on 20/02/13.
*  Copyright 2013 amscins.com All rights reserved.
*
*/
#ifndef ASI_JOB_BATCHER_Hxx
#define ASI_JOB_BATCHER_Hxx

#include "ThreadBarrier.h"
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

class AsiJobBatcher
{
public:

    static boost::shared_ptr<AsiJobBatcher> create(const std::string &batchName, int poolSize)
    {
        boost::shared_ptr<AsiJobBatcher> ret(new AsiJobBatcher(batchName));
        ret->_pWork.reset(new boost::asio::io_service::work(ret->_ioService));
        for (int i = 0; i < poolSize; ++i)
        {
            ret->_threadGroup.create_thread(boost::bind(&boost::asio::io_service::run, &ret->_ioService));
        }
        return ret;
    }

    ~AsiJobBatcher()
    {
        _pWork.reset();
        _threadGroup.join_all();
    }

    // this will leave immediately
    template <typename TFunc>
    void enqueueJob(TFunc fun)
    {
        _barrier.take();
        _ioService.post(boost::bind(&AsiJobBatcher::executeJob<TFunc>, this, fun));
    }

    void waitForJobs()
    {
        _barrier.pend();
    }

    bool isJobDone()
    {
        return _barrier.status();
    }

private:

    AsiJobBatcher(const std::string& batchName)
        : _batchName(batchName)
    {
    }

    AsiJobBatcher()
    {
    }

    AsiJobBatcher(AsiJobBatcher const &);
    AsiJobBatcher &operator=(AsiJobBatcher const &);

    template <typename TFunc>
    void executeJob(TFunc fun)
    {
        fun();
        _barrier.give();
    }

    boost::asio::io_service _ioService;
    boost::shared_ptr<boost::asio::io_service::work> _pWork;

    AsiThreadBarrier _barrier;
    boost::thread_group _threadGroup;
    std::string _batchName;
};
#endif
