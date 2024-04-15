#ifndef SYNCHRONIZER_H
#define SYNCHRONIZER_H

#include <VideoChannel.h>

class Synchronizer
{
    public:
        Synchronizer(VideoChannel* channel_left, VideoChannel* channel_right) : m_channel_left(channel_left), m_channel_right(channel_right) { }
        ~Synchronizer() { }

    private:
        VideoChannel *m_channel_left, *m_channel_right;
};

#endif // SYNCHRONIZER_H