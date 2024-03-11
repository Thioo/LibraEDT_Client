/*
*  CodecParSSE2.h
*
*  Created by Chris Desjardins on 25/01/13.
*  Copyright 2013 amscins.com All rights reserved.
*
*/
#ifndef ASI_CODEC_PAR_SSE2_Hxx
#define ASI_CODEC_PAR_SSE2_Hxx

#include "CodecSgl.h"
#ifdef RLX_WITH_SSE2

class AsiCodecParSSE2 : public AsiCodecSgl
{
public:
    virtual void decodeRows(const int startRow, const int endRow, const AsiFrame<AsiEncodeWord_t> *inFrame, AsiFrame<AsiDecodeWord_t> *outFrame) const;
protected:
    void decodeRows(const int startRow, const AsiFrame<AsiEncodeWord_t> *inFrame, AsiFrame<AsiDecodeWord_t> *outFrames) const;
};

#endif
#endif
