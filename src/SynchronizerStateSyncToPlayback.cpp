#include <SynchronizerStateSyncToPlayback.h>
#include <SynchronizerStatePlaybackPaused.h>

namespace VideoPlayback
{
    SynchronizerStateSyncToPlayback::SynchronizerStateSyncToPlayback(Synchronizer* synchronizer) : 
        ISynchronizerState("SYNCTOPLAYBACK", synchronizer)
    {
        m_logger.debug("SyncManager: entering state SYNCTOPLAYBACK");

        // on state entry we update the offset between the left and right channel
        m_pSynchronizer->m_video_frame_offset = m_pSynchronizer->m_pChannel_left->getActiveFrame() - 
                                                m_pSynchronizer->m_pChannel_right->getActiveFrame();

        // we'll set both videos to the start frame to be ready to play
        m_pSynchronizer->m_pChannel_left->setVideoToStart();
        m_pSynchronizer->m_pChannel_left->setVideoToStart();

        m_logger.debug("SyncManager: updating video channel offset to %d", m_pSynchronizer->m_video_frame_offset);
    }

    std::unique_ptr<ISynchronizerState> SynchronizerStateSyncToPlayback::handleEvent(VideoSyncEvent event, WhichVideoChannel channel)
    {
        switch(event)
        {
            case TICK:
                return std::make_unique<SynchronizerStatePlaybackPaused>(m_pSynchronizer);
            
            default:
                m_logger.warn("Unhandled event from SYNCTOPLAYBACK state: %s", sync_event_name_map.at(event));
                return nullptr;
        }
        
    }
}