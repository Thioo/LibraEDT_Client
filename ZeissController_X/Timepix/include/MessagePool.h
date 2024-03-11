/*
*  MessagePool.h
*
*  Created by Chris Desjardins on 6/02/13.
*  Copyright 2013 amscins.com All rights reserved.
*
*/

/*
** This provides a pool of messages that can be used to tx/rx with the far end
*/
#ifndef ASI_MESSAGE_POOL_Hxx
#define ASI_MESSAGE_POOL_Hxx

#include "asitypes.h"
#include "relaxd.h"
#include "MessageQueue.h"
#include <boost/shared_ptr.hpp>

class AsiMessagePool
{
public:
    AsiMessagePool(const int initialPoolSize);
    ~AsiMessagePool();
    const bool getMessage(mpixd_reply_msg** msg);
    void releaseMessage(mpixd_reply_msg *msg);
protected:
    void allocateMessages(const int numMessages);
    void deleteMessages();

    AsiMessageQueue *_freeMessages;
    int _initialPoolSize;
    int _totalPoolSize;
};

#endif
