/*
*  MessageQueue.h
*
*  Created by Chris Desjardins on 31/01/13.
*  Copyright 2013 amscins.com All rights reserved.
*
*/

/*
** This provides a thread safe queue of AsiMessages
*/

#ifndef ASI_MESSAGE_QUEUE_Hxx
#define ASI_MESSAGE_QUEUE_Hxx

#include "asitypes.h"
#include "relaxd.h"
#include <queue>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

class AsiMessageQueue
{
public:
    AsiMessageQueue();
    ~AsiMessageQueue();
    void enqueueMessage(mpixd_reply_msg* msg);
    bool dequeueMessage(mpixd_reply_msg** msg);
    /* Wait for a message, if timeout == -1, then wait forever... */
    const bool waitForMessage(mpixd_reply_msg** msg, const int msTimeout = -1);
    const int numEnqueued();

private:
    void pushMessage(mpixd_reply_msg* msg);
    bool popMessage(mpixd_reply_msg** msg);
    std::queue<mpixd_reply_msg*> _queue;
    boost::mutex _queueMutex;
    boost::condition_variable_any _msgNotification;
};

#endif
