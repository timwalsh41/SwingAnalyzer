#include <SynchronizerStateSync.h>
#include <SynchronizerStateSyncToPlayback.h>

namespace VideoPlayback
{
    SynchronizerStateSync::SynchronizerStateSync(Synchronizer* synchronizer) : ISynchronizerState("SYNC", synchronizer)
    {
        m_logger.debug("SyncManager: entering state SYNC");
    }

    std::unique_ptr<ISynchronizerState> SynchronizerStateSync::handleEvent(VideoSyncEvent event, WhichVideoChannel channel)
    {
        switch(event)
        {
            case SYNC_ON:
            case TICK:
                return nullptr;

            case SYNC_OFF:
                return std::make_unique<SynchronizerStateSyncToPlayback>(m_pSynchronizer);

            case PLAY:
            case PLAY_OR_PAUSE:
                // if we're in sync state and we get a play event, we'll just play both videos
                if (m_pSynchronizer->m_pChannel_left->getPlaybackStatus() == VideoPlaybackStatus::PAUSED &&
                    m_pSynchronizer->m_pChannel_right->getPlaybackStatus() == VideoPlaybackStatus::PAUSED)
                {
                    m_pSynchronizer->m_pChannel_left->playVideo();
                    m_pSynchronizer->m_pChannel_right->playVideo();
                }
                else
                {
                    m_pSynchronizer->m_pChannel_left->pauseVideo();
                    m_pSynchronizer->m_pChannel_right->pauseVideo();
                }

                return nullptr;

            case PAUSE:
                m_pSynchronizer->m_pChannel_left->pauseVideo();
                m_pSynchronizer->m_pChannel_right->pauseVideo();

                return nullptr;

            default:
                m_logger.warn("Unhandled event from SYNC state on channel(s) %s: %s", 
                    channel_name_map.at(channel), sync_event_name_map.at(event));
                return nullptr;
        }
    }
}