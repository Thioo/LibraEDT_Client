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

#ifndef MPXMODULEMGR_H
#define MPXMODULEMGR_H

#include <fstream>

#include "mpxmodule.h"
class AsiModuleMgrPimpl;
#define ASI_MAX_GROUP_SIZE 8
// ----------------------------------------------------------------------------

class MpxModuleMgr
{
public:
    virtual ~MpxModuleMgr();

    static MpxModuleMgr *instance();

    int         scanDevices        (  );
    MpxModule  *device             ( int hw_id );
    int         lastErr            () {
        return _globalLastErr;
    }
    std::string      lastErrStr         () {
        return _globalLastErrStr;
    }

    int         getGroupSize() { return _groupSize; }
    static AsiModuleMgrPimpl *getPimpl();
protected:
    
    void        readIni            ();
    void        autoDetect         ();

#if !defined(_WINDLL) && !defined(WIN32)
    // Versions of Windows stuff for Linux
    int GetPrivateProfileInt       ( const char *app_name,
                                     const char *key_name,
                                     int         dflt,
                                     const char *file_name );
    int GetPrivateProfileString    ( const char *app_name,
                                     const char *key_name,
                                     const char *dflt,
                                     char       *returned_string,
                                     int         sz,
                                     const char *file_name );
#endif

private:
    void        initMgr            ();
    // This is a singleton class, so c'tors are private
    MpxModuleMgr();
    MpxModuleMgr( MpxModuleMgr &mgr );
    MpxModuleMgr &operator=( MpxModuleMgr &mgr );

    bool                fileExists(std::string filename);

    // The one and only instance is also private, access through ::instance()
    static MpxModuleMgr *_inst;

    std::vector<MpxModule *>  _modules;
    std::string               _iniFileName;
    int                  _modCnt;
    std::vector<int>          _id, _chipCnt, _chipType, _chipNr;
    int                  _globalLastErr;
    std::string               _globalLastErrStr;
    std::ofstream             _fLog; // Output stream for log messages
    int                  _groupSize;
    AsiModuleMgrPimpl *_moduleMgrPimpl;
};

// ----------------------------------------------------------------------------
#endif // MPXMODULEMGR_H
