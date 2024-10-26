#include <SynchronizerStatePlaybackPaused.h>
#include <SynchronizerStatePlaybackPlaying.h>
#include <SynchronizerStateSync.h>
#include <VideoPlaybackConstants.h>

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
                // Do nothing
                return nullptr;

            case PLAY_OR_PAUSE:
                // In this state the videos should be paused, so we'll start playback
                m_pSynchronizer->m_pChannel_left->playVideo();
                m_pSynchronizer->m_pChannel_right->playVideo();
                
                // and enter the PLAYBACK_PLAYING
                return std::make_unique<SynchronizerStatePlaybackPlaying>(m_pSynchronizer);

            case STEP_FORWARD_LARGE:
                m_pSynchronizer->stepVideos(VideoPlayback::FRAME_STEP_LARGE);
                return nullptr;

            case STEP_BACKWARD_LARGE:
                m_pSynchronizer->stepVideos(-VideoPlayback::FRAME_STEP_LARGE);
                return nullptr;

            case STEP_FORWARD_SMALL:
                m_pSynchronizer->stepVideos(VideoPlayback::FRAME_STEP_SMALL);
                return nullptr;

            case STEP_BACKWARD_SMALL:
                m_pSynchronizer->stepVideos(-VideoPlayback::FRAME_STEP_SMALL);
                return nullptr;

            case SET_VIDEO_TO_START:
                m_pSynchronizer->setVideosToStartPosition();
                return nullptr;

            case VIDEO_POSITION_UPDATE:
                // Trigger the video position update, note that the video position is loaded
                // by the sync manager from the GUI callback
                m_pSynchronizer->videoPositionUpdate(BOTH_CHANNELS);
                return nullptr;

            case VIDEO_FRAME_UPDATE:
                // This event occurs when a video channel updates the currently displayed frame
                // during playback; we need to pass this through to the synchronizer to update the
                // scollbar position on the analyzer
                m_pSynchronizer->videoFrameUpdate(channel, true);
                return nullptr;

            case PLAYBACK_RATE_UPDATE:
                // User adjusts playback rate

                return nullptr;

            case LOAD_VIDEO:
                m_pSynchronizer->calculateFramerateRatio();
                return nullptr;

            case SYNC_ON:
                // Videos should already be paused, so we can go right into SYNC state
                return std::make_unique<SynchronizerStateSync>(m_pSynchronizer);

            default:
                // We shouldn't get any events other than the above 
                m_logger.warn("Unhandled event from PLAYBACK_PAUSED state: %s", sync_event_name_map.at(event));
                return nullptr;
        }
    }
}