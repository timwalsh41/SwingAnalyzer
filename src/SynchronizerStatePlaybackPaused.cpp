#include <SynchronizerStatePlaybackPaused.h>
#include <SynchronizerStatePlaybackPlaying.h>
#include <SynchronizerStateSync.h>

namespace VideoPlayback
{
    SynchronizerStatePlaybackPaused::SynchronizerStatePlaybackPaused(Synchronizer* synchronizer) : ISynchronizerState("PLAYBACK_PAUSED", synchronizer)
    {
        m_logger.debug("SyncManager: entering state PLAYBACK_PAUSED");
    }

    std::unique_ptr<ISynchronizerState> SynchronizerStatePlaybackPaused::handleEvent(VideoSyncEvent event, WhichVideoChannel channel)
    {
        switch(event)
        {
            case TICK:
                return nullptr;

            default:
                m_logger.warn("Unhandled event from PLAYBACK_PAUSED state: %s", sync_event_name_map.at(event));
                return nullptr;
        }
    }
}