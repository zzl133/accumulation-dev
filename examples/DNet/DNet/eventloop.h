#ifndef _EVENT_LOOP_H
#define _EVENT_LOOP_H

#include "platform.h"
#ifdef PLATFORM_WINDOWS
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>

    typedef BOOL(WINAPI *sGetQueuedCompletionStatusEx)
    (HANDLE CompletionPort,
    LPOVERLAPPED_ENTRY lpCompletionPortEntries,
    ULONG ulCount,
    PULONG ulNumEntriesRemoved,
    DWORD dwMilliseconds,
    BOOL fAlertable);

    #define OVL_RECV    (1)
    #define OVL_SEND    (2)
#else
    #include <sys/epoll.h>
    #include <signal.h>
    #include <sys/uio.h>
#endif

#include <stdint.h>

#include <functional>
#include <vector>
#include <mutex>
#include <thread>

using namespace std;

class Channel;

class EventLoop
{
public:
    typedef std::function<void(void)>       USER_PROC;
    typedef std::function<void(Channel*)>   CONNECTION_ENTER_HANDLE;
public:
    EventLoop();
    ~EventLoop();

    void                            loop(int64_t    timeout);

    bool                            wakeup();

    /*  投递一个链接，跟此eventloopEventLoop绑定，当被eventloop处理时会触发f回调  */
    void                            addConnection(int fd, Channel*, CONNECTION_ENTER_HANDLE f);

    /*  投递一个异步function，此eventloop被唤醒后，会回调此f*/
    void                            pushAsyncProc(USER_PROC f);

                                    /*  放入一个每次loop最后要执行的函数(TODO::仅在loop线程自身内调用)  */
                                    /*  比如实现一个datasocket有多个buffer要发送时，采用一个function进行合并包flush，则不是每一个buffer进行一次send   */
    void                            pushAfterLoopProc(USER_PROC f);

private:
    void                            recalocEventSize(int size);
    void                            linkConnection(int fd, Channel* ptr);

private:
    int                             mEventEntriesNum;
#ifdef PLATFORM_WINDOWS
    OVERLAPPED_ENTRY*               mEventEntries;
    sGetQueuedCompletionStatusEx    mPGetQueuedCompletionStatusEx;
    HANDLE                          mIOCP;
    OVERLAPPED                      mWakeupOvl;
    Channel*                        mWakeupChannel;
#else
    int                             mEpollFd;
    epoll_event*                    mEventEntries;
    int                             mWakeupFd;
    Channel*                        mWakeupChannel;
#endif

    std::mutex                      mFlagMutex;
    std::unique_lock<std::mutex>    mFlagLock;
    bool                            mInWaitIOEvent;             /*  如果为false表示肯定没有等待IOCP，如果为true，表示即将或已经等待iocp*/
    bool                            mIsAlreadyPostedWakeUp;     /*  表示是否已经投递过wakeup(避免其他线程投递太多(不必要)的wakeup) */

    vector<USER_PROC>               mAsyncProcs;                /*  异步function队列,投递到此eventloop执行的回调函数   */

    vector<USER_PROC>               mAfterLoopProcs;            /*  eventloop每次循环的末尾要执行的一系列函数，只能在io线程自身内对此队列做添加操作    */

    std::mutex                      mMutex;
    std::unique_lock<std::mutex>    mLock;
    std::thread::id                 mSelfThreadid;
};

#endif