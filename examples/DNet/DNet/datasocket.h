#ifndef _DATASOCKET_H
#define _DATASOCKET_H

#include "eventloop.h"
#include "channel.h"

#include <memory>
#include <functional>
#include <deque>
using namespace std;

class DataSocket : public Channel
{
public:
    typedef function<void(DataSocket*, const char* buffer, int len)>    DATA_PROC;
    typedef function<void(DataSocket*)>                                 DISCONNECT_PROC;
    typedef std::shared_ptr<string>                                     PACKET_PTR;

public:
    DataSocket(int fd);
    ~DataSocket();
    int                             getFD() const;

                                    /*  添加发送数据队列    */
    void                            send(const char* buffer, int len);
    void                            sendPacket(PACKET_PTR&);
    void                            sendPacket(PACKET_PTR&&);
    void                            setDataHandle(DATA_PROC proc);
    void                            setDisConnectHandle(DISCONNECT_PROC proc);

    void                            disConnect();

    static  PACKET_PTR              makePacket(const char* buffer, int len);
private:
    void                            setEventLoop(EventLoop* el);

    void                            canRecv();
    void                            canSend();

    bool                            checkRead();
    bool                            checkWrite();

    void                            flush();
    void                            onClose();

    void                            runAfterFlush();
private:
#ifdef PLATFORM_WINDOWS
    struct ovl_ext_s
    {
        OVERLAPPED  base;
    };
    struct ovl_ext_s                mOvlRecv;
    struct ovl_ext_s                mOvlSend;
#else

#endif

    int                             mFD;
    EventLoop*                      mEventLoop;

    struct pending_buffer
    {
        PACKET_PTR  packet;
        size_t      left;
    };
    std::deque<pending_buffer>      mSendList;

    bool                            mCanWrite;

    DATA_PROC                       mDataHandle;
    DISCONNECT_PROC                 mDisConnectHandle;

    bool                            mIsPostFlush;
};

#endif