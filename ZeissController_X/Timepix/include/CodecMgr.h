/*
*  CodecMgr.h
*
*  Created by Chris Desjardins on 25/01/13.
*  Copyright 2013 amscins.com All rights reserved.
*
*/
#ifndef ASI_CODEC_MGR_Hxx
#define ASI_CODEC_MGR_Hxx

#include "CodecIntf.h"

#ifndef MPXMODULE_API 
#define MPXMODULE_API 
#endif

class MPXMODULE_API AsiCodecMgr
{
public:
    AsiCodecMgr();
    ~AsiCodecMgr();

    const bool useSSE2() const
    {
        return _usingSSE2;
    }
    virtual void setUseSSE2(const bool useSSE2)
    {
        if (_supportsSSE2 == true)
        {
            _usingSSE2 = useSSE2;
        }
    }


protected:
    virtual void detectSSE2();
    virtual void initCodec();
    virtual void spawnDecodeThreads(AsiCodecIntf *codec, AsiFrame<AsiEncodeWord_t>* encodeFrames, AsiFrame<AsiDecodeWord_t>* decodeFrames);

    bool _supportsSSE2;
    bool _usingSSE2;
    int _numThreads;

};

#endif
