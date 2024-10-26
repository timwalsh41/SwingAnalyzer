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

        m_video_complete_dispatcher.connect([this]{handleEvent(PLAYBACK_COMPLETE, m_playback_complete_channel.load());});
        m_video_frame_update_dispatcher.connect([this]{
            this->m_synchronizer.m_new_active_frame = m_video_active_frame.load();
            handleEvent(VIDEO_FRAME_UPDATE, m_video_frame_update_channel.load());
        });

        // TODO: TEMPORARY TEST CODE TO LOAD AND PLAY VIDEO CHANNEL
        m_channel_left.loadVideo("/home/tim/Desktop/PXL_20240329_005944373.TS.mp4");
        // auto video_size = m_channel_left.getVideoSize();
        
        m_channel_right.loadVideo("/home/tim/Desktop/PXL_20240329_005944373.TS.mp4");
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

    // Thread-safe playback complete callback. This can be invoked by either video channel, so we use an atomic
    // to capture the source channel and then emit a signal via our dispatcher to then handle the event in the main
    // thread.
    void SyncManager::videoPlaybackCompleteCallback(WhichVideoChannel source_channel)
    {
        if (source_channel == LEFT_CHANNEL || source_channel == RIGHT_CHANNEL)
        {
            m_playback_complete_channel.store(source_channel);
            m_video_complete_dispatcher.emit();
        }
    }

    // Thread-safe video position update callback. This is invoked by the video channels as they progress through
    // their frames during playback. We will pass this signal on to the analyzer to update the position of the 
    // appropriate scrollbar
    void SyncManager::videoFrameUpdate(int active_frame, WhichVideoChannel source_channel)
    {
        if ((source_channel == LEFT_CHANNEL || source_channel == RIGHT_CHANNEL) && active_frame > 0)
        {
            m_video_frame_update_channel.store(source_channel);
            m_video_active_frame.store(active_frame);
            m_video_frame_update_dispatcher.emit();
        }
    }

} // namespace VideoPlayback