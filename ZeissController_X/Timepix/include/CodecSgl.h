/*
*  CodecSgl.h
*
*  Created by Chris Desjardins on 25/01/13.
*  Copyright 2013 amscins.com All rights reserved.
*
*/
#ifndef ASI_CODEC_SGL_Hxx
#define ASI_CODEC_SGL_Hxx

#include "CodecIntf.h"

#define RLX_SIMD_SIZE       8

class AsiCodecSgl : public AsiCodecIntf
{
public:
    virtual void encodeRows(const AsiFrame<AsiDecodeWord_t> *inFrame, AsiFrame<AsiEncodeWord_t> *outFrame) const;
    virtual void decodeRows(const int startRow, const int endRow, const AsiFrame<AsiEncodeWord_t> *inFrame, AsiFrame<AsiDecodeWord_t> *outFrame) const;
protected:
};

#endif
