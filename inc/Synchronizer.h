#ifndef SYNCHRONIZER_H
#define SYNCHRONIZER_H

#include <VideoChannel.h>

namespace VideoPlayback
{

class Synchronizer
{
  public:
    Synchronizer(VideoChannel *channel_left, VideoChannel *channel_right,
                 std::function<void(bool)> enableControlCallback)
        : m_channel_left(channel_left), m_channel_right(channel_right), m_enableControlCallback(enableControlCallback)
    {
    }
    ~Synchronizer()
    {
    }

  private:
    VideoChannel *m_channel_left, *m_channel_right;
    std::function<void(bool)> m_enableControlCallback;
};

} // namespace VideoPlayback

#endif // SYNCHRONIZER_H