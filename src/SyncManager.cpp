#include <SyncManager.h>
#include <SynchronizerStateSync.h>

#include <memory>

namespace VideoPlayback
{
    SyncManager::SyncManager(std::shared_ptr<VideoChannel> leftChannel, 
                             std::shared_ptr<VideoChannel> rightChannel) :
        m_logger(Logger::get()),
        m_channel_left(leftChannel),
        m_channel_right(rightChannel),
        m_synchronizer(m_channel_left, m_channel_right)
    {
        // Set up frame update callbacks
        m_channel_left->registerFrameUpdateCallback([this](int active_frame, WhichVideoChannel channel) {
            this->videoFrameUpdate(active_frame, channel);
        });
        
        m_channel_right->registerFrameUpdateCallback([this](int active_frame, WhichVideoChannel channel) {
            this->videoFrameUpdate(active_frame, channel);
        });
        
        // Set up playback complete callbacks
        m_channel_left->registerVideoPlaybackCompleteCallback([this](WhichVideoChannel source_channel){
            this->videoPlaybackCompleteCallback(source_channel);
        });
        m_channel_right->registerVideoPlaybackCompleteCallback([this](WhichVideoChannel source_channel){
            this->videoPlaybackCompleteCallback(source_channel);
        });
    }

    void SyncManager::setObserver(std::weak_ptr<IVideoSynchronizationObserver> observer)
    {
        m_observer = observer;
        m_synchronizer.setObserver(observer);
    }

    void SyncManager::start()
    {
        // start the state machine in Sync state
        m_state = std::make_unique<SynchronizerStateSync>(&m_synchronizer);
    }

    void SyncManager::handleEvent(VideoSyncEvent event, WhichVideoChannel channel)
    {
        std::unique_ptr<ISynchronizerState> next_state = m_state->handleEvent(event, channel);
        
        while (next_state)
        {
            m_state = std::move(next_state);

            // immediately tick the new state, as some states can transition directly out
            next_state = m_state->handleEvent(TICK, NO_CHANNEL);
        }
    }

    // Receive video position update, affects playback mode only, for updating video position based on
    // joint slider position
    void SyncManager::videoPositionUpdate(float video_position, WhichVideoChannel channel)
    {
        if (video_position < 0 || video_position > 1)
        {
            m_logger.warn("Invalid video position %.3f, valid range is (0.0,1.0)", video_position);
            return;
        }

        // load the new position into the synchronizer
        m_synchronizer.m_new_video_position = video_position;

        // and trigger the video position update event for the state machine to handle this event
        handleEvent(VIDEO_POSITION_UPDATE, channel);
    } 

    // Playback complete callback, triggered from either video channel, we hand off to the state machine
    // to handle the event
    void SyncManager::videoPlaybackCompleteCallback(WhichVideoChannel source_channel)
    {
        handleEvent(PLAYBACK_COMPLETE, source_channel);
    }

    // Video position update callback, this is invoked by the video channels as they progress through
    // their frames during playback; we pass this on to the state machine to handle the event
    void SyncManager::videoFrameUpdate(int active_frame, WhichVideoChannel source_channel)
    {
        if ((source_channel == LEFT_CHANNEL || source_channel == RIGHT_CHANNEL) && active_frame > 0)
        {
            this->m_synchronizer.m_new_active_frame = active_frame;
            handleEvent(VIDEO_FRAME_UPDATE, source_channel);
        }
    }

} // namespace VideoPlayback