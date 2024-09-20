#ifndef SYNCHMANAGER_H
#define SYNCHMANAGER_H

#include <Synchronizer.h>
#include <ISynchronizerState.h>
#include <Logger.h>

#include <atomic>
#include <string>

class Analyzer;
class VideoChannel;

namespace VideoPlayback
{

class SyncManager
{
  public:
    SyncManager(Analyzer *analyzer, VideoChannel *channel_left, VideoChannel *channel_right);
    ~SyncManager() = default;
    
    // general event handler for synchronizer state machine
    void handleEvent(VideoSyncEvent event, WhichVideoChannel channel);

  private:
    // callback for video playback complete, implemented as thread-safe
    void videoPlaybackCompleteCallback(std::string& source_channel);
    
    std::unique_ptr<ISynchronizerState> m_state;

    Analyzer *m_pAnalyzer;
    Synchronizer m_synchronizer;

    // the dispatcher emits a signal to indicate when playback is complete on one of the
    // video channels
    std::atomic<WhichVideoChannel> m_playback_complete_channel{WhichVideoChannel::NO_CHANNEL};
    Glib::Dispatcher m_video_complete_dispatcher;

    Logger &m_logger;
};

} // namespace VideoPlayback

#endif // SYNCHRONIZER_H