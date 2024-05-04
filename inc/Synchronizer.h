#ifndef SYNCHRONIZER_H
#define SYNCHRONIZER_H

#include <VideoChannel.h>

namespace VideoPlayback
{

class Synchronizer
{
  public:
    Synchronizer(VideoChannel *channel_left, VideoChannel *channel_right)
        : m_channel_left(channel_left), m_channel_right(channel_right)
    {
    }
    ~Synchronizer()
    {
    }

    void setSyncMode(bool syncMode);

  private:
    VideoChannel *m_channel_left, *m_channel_right;
};

} // namespace VideoPlayback

#endif // SYNCHRONIZER_H