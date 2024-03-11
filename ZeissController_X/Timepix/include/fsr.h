/*
       Copyright 2011 NIKHEF

       Licensed under the Apache License, Version 2.0 (the "License");
       you may not use this file except in compliance with the License.
       You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

       Unless required by applicable law or agreed to in writing, software
       distributed under the License is distributed on an "AS IS" BASIS,
       WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
       See the License for the specific language governing permissions and
       limitations under the License.

*/

#ifndef FSR_H
#define FSR_H

#include <fstream>


//-----------------------------------------------------------------------------

#if defined(_WINDLL) || defined(WIN32)
//class __declspec( dllexport ) Fsr
class Fsr
#else
class Fsr
#endif
{
public:
    Fsr( int nbits );
    virtual ~Fsr();

    // Assignment operator
    Fsr           &operator=( const Fsr &f );

    // General-purpose bitstring get/set methods
    void           setOnes   ();
    void           setZeroes ();
    void           setBits   ( int i_dst, int nbits, int value );
    int            bits      ( int index, int nbits ) const;
    int            bitLength () const {
        return _nbits;
    };
    int            byteLength() const {
        return _nbytes;
    };
    unsigned char *bytes     () const {
        return _bytes;
    };
    virtual void   log       ( std::ofstream &flog );

    // Medipix FSR specific methods
    bool           toDacSettings  ( int type, int *dac, int *sz );
    bool           fromDacSettings( int type, int *dac );

private:
    unsigned char *_bytes;
    int           _nbits, _nbytes;
};

//-----------------------------------------------------------------------------
#endif // FSR_H
