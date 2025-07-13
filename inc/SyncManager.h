#ifndef SYNCHMANAGER_H
#define SYNCHMANAGER_H

#include <ISynchronizerState.h>
#include <IVideoSynchronizationObserver.h>
#include <Logger.h>
#include <Synchronizer.h>

#include <memory>
#include <string>

namespace VideoPlayback
{

class SyncManager
{
  public:
    SyncManager();
    ~SyncManager() = default;
    
    // Observer management
    void setObserver(std::weak_ptr<IVideoSynchronizationObserver> observer);
    
    // general event handler for synchronizer state machine
    void handleEvent(VideoSyncEvent event, WhichVideoChannel channel);

    // Receive video position update from GUI slider
    // Could be for individual video channel (e.g. in sync mode) or
    // both channels (e.g. in playback mode)
    void videoPositionUpdate(float video_position, WhichVideoChannel channel);

    // Callback from video channel for position update, to pass on
    // updated position to slider
    void videoFrameUpdateCallback(float video_position, WhichVideoChannel channel);

    // Start the sync manager state machine
    void start();

    VideoChannel* getLeftChannel() { return &m_channel_left; }
    VideoChannel* getRightChannel() { return &m_channel_right; }

  private:
    // the sync manager owns the video channels; in the constructor we provide the callback
    // for the video frame update to update the slider position
    VideoPlayback::VideoChannel m_channel_left{WhichVideoChannel::LEFT_CHANNEL,
                                               [this](int active_frame, WhichVideoChannel channel) { this->videoFrameUpdate(active_frame, channel); }};
    VideoPlayback::VideoChannel m_channel_right{WhichVideoChannel::RIGHT_CHANNEL,
                                               [this](int active_frame, WhichVideoChannel channel) { this->videoFrameUpdate(active_frame, channel); }};

    // callback for video playback complete, implemented as thread-safe
    void videoPlaybackCompleteCallback(WhichVideoChannel source_channel);
    
    // callback for video frame update from video channel, implemented as thread-safe
    void videoFrameUpdate(int active_frame, WhichVideoChannel source_channel);

    std::unique_ptr<ISynchronizerState> m_state;

    std::weak_ptr<IVideoSynchronizationObserver> m_observer;
    Synchronizer m_synchronizer;

    Logger &m_logger;
};

} // namespace VideoPlayback

#endif // SYNCHRONIZER_H