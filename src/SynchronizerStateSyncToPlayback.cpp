#include <SynchronizerStateSyncToPlayback.h>
#include <SynchronizerStatePlaybackPaused.h>

/*
The SYNC_TO_PLAYBACK state exists only to lock in the video channel offset as we transition from sync
mode to playback mode. Thus we enter the state, set up the offset, set the videos to the appropriate
start position, and then transition to PLAYBACK_PAUSED.
*/
namespace VideoPlayback
{
    SynchronizerStateSyncToPlayback::SynchronizerStateSyncToPlayback(ISynchronizerActions* synchronizer) : 
        ISynchronizerState("SYNCTOPLAYBACK", synchronizer)
    {
        m_logger.debug("SyncManager: entering state SYNCTOPLAYBACK");

        // on state entry we update the offset between the left and right channel
        m_pSynchronizer->setupVideoSync();

        // we'll set both videos to the start frame to be ready to play
        m_pSynchronizer->setVideosToStartPosition();

        // and set the position to be notified to observers
        m_pSynchronizer->setNewVideoPosition(0.0f);

        m_logger.debug("SyncManager: updating video channel offset to %d", m_pSynchronizer->getVideoFrameOffset());
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