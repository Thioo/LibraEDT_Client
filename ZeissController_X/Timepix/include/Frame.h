/*
*  Frame.h
*
*  Created by Chris Desjardins on 25/01/13.
*  Copyright 2013 amscins.com All rights reserved.
*
*/
#ifndef ASI_FRAME_Hxx
#define ASI_FRAME_Hxx

#include <cstddef>
#include "../common/Relaxdhw.h"
#include "../common/asitypes.h"

typedef u16 AsiDecodeWord_t;
typedef u8 AsiEncodeWord_t;

template <class T> class  AsiFrame
{
public:
    AsiFrame(const bool allocFrameData = true, const int bpp = MPIX_PIXEL_BITS, const int rows = MPIX_ROWS, const int cols = MPIX_COLS)
    {
        _frameData = NULL;
        _bpp = bpp;
        _rows = rows;
        _cols = cols;
        _allocFrameData = allocFrameData;
        allocateFrame();
    }
    ~AsiFrame() { deallocateFrame(); }
    const int getRowElements() const
    {
        int ret = -1;
        int s = sizeof(T);
        if (s == sizeof(AsiDecodeWord_t))
        {
            /* Number of elements in decode row is the number of cols. */
            ret = _cols;
        }
        else if (s == sizeof(AsiEncodeWord_t))
        {
            /* 
            ** Number of elements in encode row is packed, i.e.
            ** number of cols TIMES
            ** number of bits/pixel DIVIDED BY
            ** number of bits/byte
            */
            ret = ((_cols * _bpp) / 8);
        }
        return ret;
    }
    const int getFrameElements() const { return getRowElements() * _rows; };
    const int getRowCnt() const { return _rows; };
    const int getColCnt() const { return _cols; };
    const int getBitsPerPixel() const { return _bpp; };

    T *getFramePtr(const int startRow = 0) const 
    {
        if (startRow < getRowCnt())
        {
            return _frameData + (startRow * getRowElements()); 
        }
        return NULL;
    }

    void zeroRows(const int startRow = 0, const int endRow = -1)
    {
        int e = (endRow == -1) ? getRowCnt() : endRow;
        int numRows =  (e - startRow);
        int numElements = getRowElements() * numRows;
        T *start = getFramePtr(startRow);
    
        for (int i = 0; i < numElements; i++)
        {
            start[i] = 0;
        }
    }

    /*
    ** Helper function for when you have a list of frames, and you want to get
    ** row > getRowCnt(), this function will navigate through the frame list
    ** to the right frame and select the right row from that frame.
    */
    static T *getFrameListRowPtr(const AsiFrame<T> *frameList, const int startRow)
    {
        int row = startRow % frameList->getRowCnt();
        int frameIndex = startRow / frameList->getRowCnt();
        return frameList[frameIndex].getFramePtr(row);
    }

protected:
    virtual void allocateFrame()
    {
        _frameData = NULL;
        if (_allocFrameData == true)
        {
            _frameData = new T[getFrameElements()];
        }
    }

    virtual void deallocateFrame()
    {
        if ((_allocFrameData == true) && (_frameData != NULL))
        {
            delete []_frameData;
            _frameData = NULL;
        }
    }

    int _bpp;
    int _rows;
    int _cols;
    bool _allocFrameData;

    T *_frameData;
};

#endif
