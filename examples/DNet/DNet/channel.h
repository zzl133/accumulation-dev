#ifndef _CHANNEL_H
#define _CHANNEL_H

class Channel
{
public:
    virtual void    canSend() = 0;
    virtual void    canRecv() = 0;
    virtual void    setEventLoop(EventLoop*) = 0;
    virtual void    disConnect() = 0;
};

#endif