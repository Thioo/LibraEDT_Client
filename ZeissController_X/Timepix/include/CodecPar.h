/*
*  CodecPar.h
*
*  Created by Chris Desjardins on 25/01/13.
*  Copyright 2013 amscins.com All rights reserved.
*
*/
#ifndef ASI_CODEC_PAR_Hxx
#define ASI_CODEC_PAR_Hxx

#include "CodecSgl.h"

class AsiCodecPar : public AsiCodecSgl
{
public:
    virtual void decodeRows(const int startRow, const int endRow, const AsiFrame<AsiEncodeWord_t> *inFrame, AsiFrame<AsiDecodeWord_t> *outFrame) const;
    virtual void decodeSetup(AsiFrame<AsiDecodeWord_t> *outFrames) const;
    virtual void decodeCleanup(AsiFrame<AsiDecodeWord_t> *outFrames) const;
protected:
    void decodeRows(const int startRow, const AsiFrame<AsiEncodeWord_t> *inFrame, AsiFrame<AsiDecodeWord_t> *outFrame) const;
};

#endif
