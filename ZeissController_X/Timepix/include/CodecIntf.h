/*
*  CodecIntf.h
*
*  Created by Chris Desjardins on 25/01/13.
*  Copyright 2013 amscins.com All rights reserved.
*
*/
#ifndef ASI_CODEC_INTF_Hxx
#define ASI_CODEC_INTF_Hxx

#define RLX_WITH_SSE2

#include "Frame.h"
#include "unparam.h"

class AsiCodecIntf
{
public:
    virtual void encodeRows(const AsiFrame<AsiDecodeWord_t> *inFrame, AsiFrame<AsiEncodeWord_t> *outFrame) const = 0;
    virtual void decodeRows(const int startRow, const int endRow, const AsiFrame<AsiEncodeWord_t> *inFrame, AsiFrame<AsiDecodeWord_t> *outFrame) const = 0;
    virtual void decodeSetup(AsiFrame<AsiDecodeWord_t> *outFrames)   const { UNREF_PARAM(outFrames); };
    virtual void decodeCleanup(AsiFrame<AsiDecodeWord_t> *outFrames) const { UNREF_PARAM(outFrames); };
    virtual ~AsiCodecIntf() {}
protected:
};

#endif
