#ifndef SYNCHMANAGER_H
#define SYNCHMANAGER_H

#include <Synchronizer.h>
#include <ISynchronizerState.h>
#include <Logger.h>

#include <atomic>
#include <string>

namespace VideoPlayback
{

class SyncManager
{
  public:
    SyncManager(Analyzer *analyzer);
    ~SyncManager() = default;
    
    // general event handler for synchronizer state machine
    void handleEvent(VideoSyncEvent event, WhichVideoChannel channel);

    // start the Synchronizer state machine
    void start();

    // Receive video position update from GUI slider
    // Could be for individual video channel (e.g. in sync mode) or
    // both channels (e.g. in playback mode)
    void videoPositionUpdate(float video_position, WhichVideoChannel channel);

    // Callback from video channel for position update, to pass on
    // updated position to slider
    void videoFrameUpdateCallback(float video_position, WhichVideoChannel channel);

    VideoChannel* getLeftChannel() { return &m_channel_left; }
    VideoChannel* getRightChannel() { return &m_channel_right; }

  private:
    // the sync manager owns the video channels
    VideoPlayback::VideoChannel m_channel_left{WhichVideoChannel::LEFT_CHANNEL,
                                               [this](int active_frame, WhichVideoChannel channel) { this->videoFrameUpdate(active_frame, channel); }};
    VideoPlayback::VideoChannel m_channel_right{WhichVideoChannel::RIGHT_CHANNEL,
                                               [this](int active_frame, WhichVideoChannel channel) { this->videoFrameUpdate(active_frame, channel); }};

    // callback for video playback complete, implemented as thread-safe
    void videoPlaybackCompleteCallback(WhichVideoChannel source_channel);
    
    // callback for video frame update from video channel, implemented as thread-safe
    void videoFrameUpdate(int active_frame, WhichVideoChannel source_channel);

    std::unique_ptr<ISynchronizerState> m_state;

    Analyzer *m_pAnalyzer;
    Synchronizer m_synchronizer;

    // the dispatcher emits a signal to indicate when playback is complete on one of the
    // video channels
    std::atomic<WhichVideoChannel> m_playback_complete_channel{WhichVideoChannel::NO_CHANNEL};
    std::atomic<WhichVideoChannel> m_video_frame_update_channel{WhichVideoChannel::NO_CHANNEL};
    std::atomic<int> m_video_active_frame{0};

    Glib::Dispatcher m_video_complete_dispatcher;
    Glib::Dispatcher m_video_frame_update_dispatcher;

    Logger &m_logger;
};

} // namespace VideoPlayback

#endif // SYNCHRONIZER_H