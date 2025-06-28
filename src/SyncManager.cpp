#include <Analyzer.h>
#include <SyncManager.h>
#include <SynchronizerStateSync.h>

#include <memory>

namespace VideoPlayback
{
    SyncManager::SyncManager(Analyzer *analyzer) :
        m_pAnalyzer(analyzer), m_synchronizer(analyzer, &m_channel_left, &m_channel_right), m_logger(Logger::get())
    {
        m_channel_left.registerVideoPlaybackCompleteCallback([this](WhichVideoChannel source_channel){this->videoPlaybackCompleteCallback(source_channel);});
        m_channel_right.registerVideoPlaybackCompleteCallback([this](WhichVideoChannel source_channel){this->videoPlaybackCompleteCallback(source_channel);});

        // TODO: TEMPORARY TEST CODE TO LOAD AND PLAY VIDEO CHANNEL
        m_channel_left.loadVideo("/home/tim/Downloads/Golf/2025-05-27 4i DTL.mp4");
        // auto video_size = m_channel_left.getVideoSize();
        
        m_channel_right.loadVideo("/home/tim/Downloads/Golf/2025-05-27 4i FO.mp4");
        // video_size = m_channel_right.getVideoSize();
    }

    void SyncManager::start()
    {
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