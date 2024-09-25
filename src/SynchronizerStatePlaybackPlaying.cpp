#include <SynchronizerStatePlaybackPlaying.h>
#include <SynchronizerStatePlaybackPaused.h>
#include <SynchronizerStateSync.h>
#include <VideoPlaybackConstants.h>
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
                // Do nothing
                return nullptr;
            
            case PLAY_OR_PAUSE:
                // Videos are playing, so we'll pause them
                m_pSynchronizer->m_pChannel_left->pauseVideo();
                m_pSynchronizer->m_pChannel_right->pauseVideo();

                // and enter the PLAYBACK_PAUSED state
                return std::make_unique<SynchronizerStatePlaybackPaused>(m_pSynchronizer);

            case PLAYBACK_COMPLETE:
                // One video should still be playing, make sure it's paused
                m_pSynchronizer->pauseVideos();
                m_pSynchronizer->setVideosToStartPosition();
                return std::make_unique<SynchronizerStatePlaybackPaused>(m_pSynchronizer);

            case STEP_FORWARD_LARGE:
                m_pSynchronizer->pauseVideos();
                m_pSynchronizer->stepVideos(VideoPlayback::FRAME_STEP_LARGE);
                return std::make_unique<SynchronizerStatePlaybackPaused>(m_pSynchronizer);

            case STEP_BACKWARD_LARGE:
                m_pSynchronizer->pauseVideos();
                m_pSynchronizer->stepVideos(-VideoPlayback::FRAME_STEP_LARGE);
                return std::make_unique<SynchronizerStatePlaybackPaused>(m_pSynchronizer);

            case STEP_FORWARD_SMALL:
                m_pSynchronizer->pauseVideos();
                m_pSynchronizer->stepVideos(VideoPlayback::FRAME_STEP_SMALL);
                return std::make_unique<SynchronizerStatePlaybackPaused>(m_pSynchronizer);

            case STEP_BACKWARD_SMALL:
                m_pSynchronizer->pauseVideos();
                m_pSynchronizer->stepVideos(-VideoPlayback::FRAME_STEP_SMALL);
                return std::make_unique<SynchronizerStatePlaybackPaused>(m_pSynchronizer);

            case SET_VIDEO_TO_START:
                m_pSynchronizer->pauseVideos();
                m_pSynchronizer->setVideosToStartPosition();
                return std::make_unique<SynchronizerStatePlaybackPaused>(m_pSynchronizer);

            case VIDEO_POSITION_UPDATE:
                return std::make_unique<SynchronizerStatePlaybackPaused>(m_pSynchronizer);

            case PLAYBACK_RATE_UPDATE:
                return nullptr;

            case LOAD_VIDEO:
                m_pSynchronizer->calculateFramerateRatio();
                return nullptr;

            case SYNC_ON:
                // Videos are playing, so we'll pause them
                m_pSynchronizer->m_pChannel_left->pauseVideo();
                m_pSynchronizer->m_pChannel_right->pauseVideo();

                // And enter the SYNC state
                return std::make_unique<SynchronizerStateSync>(m_pSynchronizer);

            default:
                // We shouldn't get any events other than the above
                m_logger.warn("Unhandled event from PLAYBACK_PLAYING state: %s", sync_event_name_map.at(event));
                return nullptr;
        }
    }
}