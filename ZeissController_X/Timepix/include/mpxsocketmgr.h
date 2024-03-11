
#ifndef MPXSOCKET_H
#define MPXSOCKET_H

#include "mpxlogger.h"
#include "../common/common.h"
#include <string>
#include <map>
#include <vector>

#if defined(_WINDLL) || defined(WIN32)
#include <winsock.h>
#else
// Linux
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <errno.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#endif

#include "mpxplatform.h"
#include "relaxd.h"

class AsiMessageQueue;
class AsiMessagePool;
struct MpxSocketMgrPimpl;

class MPXMODULE_API MpxSocketMgr
{
public:
	MpxSocketMgr(MpxLogger *log);
	virtual ~MpxSocketMgr();
	
	int initSocket();

	int connectToSocket(u32 ipAddress, u32 portNumber);
	int bindSocketFunc(u32 ipAddress, u32 portNumber, const char *dbg);
#define bindSocket(i,p) bindSocketFunc(i,p,__FUNCTION__)

	int setSockOptions(int optname, const char *optval, int optlen);
	int getSockOptions(int optname, char *optval, int *optlen);

	void setSockTimeout( bool forever, int ms  = 1000);
	bool msgAvailable(mpixd_type msgId);

	bool recvData(mpixd_reply_msg **buf, const std::vector<mpixd_type> &msgs);
	bool recvData(mpixd_reply_msg **buf, const mpixd_type msgId);
	int sendDataFunc(const char *buf, int len, const char *dbg);
#define sendData(b,l) sendDataFunc(b,l,__FUNCTION__)
    void releaseRxMsg(mpixd_reply_msg **buf);

    void clearRxQueue(mpixd_type msgId);
protected:
	bool recvData(mpixd_reply_msg **buf, const mpixd_type msgId, const int timeoutMs);
    bool msgAvailable(const std::vector<mpixd_type> &msgs, mpixd_type *msgId);

	int ioctlSock(long cmd, u_long *argp);
    int fcntlSock(long cmd, int args);
    int fcntlSock(long cmd);
    void setNonBlocking(bool nonBlocking);
    int getErrno();
    virtual void socketRxThread();
    volatile bool _threadRun;
    MpxSocketMgrPimpl *_socketMgrData;
    AsiMessagePool  *_freeMsgPool;
    int _timeoutMs;

    SOCKET _sock;
	MpxLogger *_log;

	std::string getErrorString();
	void closeSocket();

    u64 _txPacketCnt;
    u64 _rxPacketCnt;
    u64 _txByteCnt;
    u64 _rxByteCnt;

    struct sockaddr_in _sendToaddr;
    bool _isNonBlocking;
private:
};

class MPXMODULE_API MpxSocketMgrRt : public MpxSocketMgr
{
public:
	MpxSocketMgrRt(MpxLogger *log) : MpxSocketMgr(log) {};
protected:
    void socketThreadPriority();
    virtual void socketRxThread();

};
#endif
