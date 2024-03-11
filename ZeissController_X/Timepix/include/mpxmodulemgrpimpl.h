#ifndef MPXMODULEMGRPIMPL_H
#define MPXMODULEMGRPIMPL_H
#include "JobBatcher.h"
class AsiModuleMgrPimpl
{
public:
    AsiModuleMgrPimpl()
        : _readFrameJobs(AsiJobBatcher::create("Readframe", 4)),
        _startAcqJobs(AsiJobBatcher::create("StartAcq", 4)),
        _stopAcqJobs(AsiJobBatcher::create("StopAcq", 4)),
        _isBusyJobs(AsiJobBatcher::create("IsBusy", 4))
    {
    }

    boost::shared_ptr<AsiJobBatcher> _readFrameJobs;
    boost::shared_ptr<AsiJobBatcher> _startAcqJobs;
    boost::shared_ptr<AsiJobBatcher> _stopAcqJobs;
    boost::shared_ptr<AsiJobBatcher> _isBusyJobs;
};

#endif