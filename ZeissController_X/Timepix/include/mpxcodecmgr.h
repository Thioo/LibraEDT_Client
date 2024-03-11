#ifndef MXP_CODEC_MGR_Hxx
#define MXP_CODEC_MGR_Hxx

#include "mpxplatform.h"
#include "CodecMgr.h"
#include "../common/common.h"

template <class T> class  MpxFrame : public AsiFrame<T>
{
public:
    MpxFrame() : AsiFrame<T>(false){};
    void setFramePtr(T *p)
    {
        this->_frameData = p;
    };
protected:
};

class MPXMODULE_API MpxCodecMgr : public AsiCodecMgr
{
public:
    MpxCodecMgr();
    virtual ~MpxCodecMgr();
    virtual void allocateCodec();
    virtual void deleteCodec();
    void decodePar(u8 *bytes, i16 *pixels);
    void decodeSgl(u8 *stream, i16 *data);
    void encodeSgl(i16 *mask, u8 *stream);
    void setUseSSE2(const bool useSSE2);

protected:

    AsiCodecIntf *_codecPar;
    AsiCodecIntf *_codecSgl;
    MpxFrame<AsiDecodeWord_t> *_mpxDecodeFrames;
    MpxFrame<AsiEncodeWord_t> *_mpxEncodeFrames;
};

#endif

