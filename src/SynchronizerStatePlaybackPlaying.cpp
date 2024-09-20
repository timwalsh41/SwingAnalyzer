#include <SynchronizerStatePlaybackPlaying.h>
#include <SynchronizerStatePlaybackPaused.h>
#include <SynchronizerStateSync.h>

namespace VideoPlayback
{
    SynchronizerStatePlaybackPlaying::SynchronizerStatePlaybackPlaying(Synchronizer* synchronizer) : ISynchronizerState("PLAYBACK_PAUSED", synchronizer)
    {
        m_logger.debug("SyncManager: entering state PLAYBACK_PLAYING");
    }

    std::unique_ptr<ISynchronizerState> SynchronizerStatePlaybackPlaying::handleEvent(VideoSyncEvent event, WhichVideoChannel channel)
    {
        switch(event)
        {
            case TICK:
                return nullptr;

            default:
                m_logger.warn("Unhandled event from PLAYBACK_PLAYING state: %s", sync_event_name_map.at(event));
                return nullptr;
        }
    }
}