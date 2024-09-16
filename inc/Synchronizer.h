#ifndef SYNCHRONIZER_H
#define SYNCHRONIZER_H

#include <VideoChannel.h>
#include <Logger.h>

class Analyzer;

namespace VideoPlayback
{

enum VideoSyncMode
{
  SYNC,
  PLAYBACK
};

class Synchronizer
{
  public:
    Synchronizer(Analyzer *analyzer, VideoChannel *channel_left, VideoChannel *channel_right);
    ~Synchronizer() = default;
    
    void setSyncMode(bool syncMode);
    void playOrPauseVideo();

  private:
    VideoSyncMode m_syncMode{SYNC};

    Analyzer *m_pAnalyzer;
    VideoChannel *m_pChannel_left, *m_pChannel_right;

    Logger &m_logger;
};

} // namespace VideoPlayback

#endif // SYNCHRONIZER_H