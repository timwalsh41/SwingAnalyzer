#include <SynchronizerStateSyncToPlayback.h>
#include <SynchronizerStatePlaybackPaused.h>
#include <Analyzer.h>

/*
The SYNC_TO_PLAYBACK state exists only to lock in the video channel offset as we transition from sync
mode to playback mode. Thus we enter the state, set up the offset, set the videos to the appropriate
start position, and then transition to PLAYBACK_PAUSED.
*/
namespace VideoPlayback
{
    SynchronizerStateSyncToPlayback::SynchronizerStateSyncToPlayback(Synchronizer* synchronizer) : 
        ISynchronizerState("SYNCTOPLAYBACK", synchronizer)
    {
        m_logger.debug("SyncManager: entering state SYNCTOPLAYBACK");

        // on state entry we update the offset between the left and right channel
        m_pSynchronizer->setupVideoSync();

        // we'll set both videos to the start frame to be ready to play
        m_pSynchronizer->setVideosToStartPosition();

        // and move the scrollbar to the zero position
        m_pSynchronizer->m_pAnalyzer->updateCombinedVideoPosition(0.0);

        m_logger.debug("SyncManager: updating video channel offset to %d", m_pSynchronizer->m_video_frame_offset);
    }

    std::unique_ptr<ISynchronizerState> SynchronizerStateSyncToPlayback::handleEvent(VideoSyncEvent event, WhichVideoChannel channel)
    {
        switch(event)
        {
            case TICK:
                // on tick we transition to PLAYBACK_PAUSED
                return std::make_unique<SynchronizerStatePlaybackPaused>(m_pSynchronizer);
            
            default:
                // we shouldn't get any events other than the above
                m_logger.warn("Unhandled event from SYNCTOPLAYBACK state: %s", sync_event_name_map.at(event));
                return nullptr;
        }
        
    }
}