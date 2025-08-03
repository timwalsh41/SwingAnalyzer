#include <SynchronizerStateSync.h>
#include <SynchronizerStateSyncToPlayback.h>
#include <VideoPlaybackConstants.h>

/*
In the SYNC state, the user can control the videos individually to adjust the timing for synchronized
playback. There are individual adjustment controls (frame step & slider), and the single play/pause
button for both channels. The individual controls are routed directly to the video channels since no
synchronization is required. The play/pause button is handled by the synchronizer, as well as the 
transition to the SYNC_TO_PLAYBACK state when sync mode is deselected.
*/

namespace VideoPlayback
{
    SynchronizerStateSync::SynchronizerStateSync(ISynchronizerActions* synchronizer) : ISynchronizerState("SYNC", synchronizer)
    {
        // we'll clear out the offset once we go back into SYNC mode
        m_pSynchronizer->setVideoFrameOffset(0);
        m_pSynchronizer->clearStartAndEndFrames();

        m_logger.debug("SyncManager: entering state SYNC");
    }

    std::unique_ptr<ISynchronizerState> SynchronizerStateSync::handleEvent(VideoSyncEvent event, WhichVideoChannel channel)
    {
        switch(event)
        {
            case TICK:
                // on tick, we'll trigger a video position update to make sure the scrollbars get updated
                // we need to do this on the tick rather than in the constructor (e.g. on state entry) because
                // the video position update is handled differently depending on state; we need to trigger this
                // update while the Synchronizer sees we are in the Sync state (e.g. the constructor has finished)
                // to ensure the individual channel scrollbars are updated rather than the joint scrollbar
                m_pSynchronizer->videoPositionUpdate(BOTH_CHANNELS); 

                return nullptr;

            case SYNC_ON:
            case PLAYBACK_COMPLETE:
            case SET_VIDEO_TO_START:
                // no action
                return nullptr;

            case SYNC_OFF:
                // move to SYNC_TO_PLAYBACK state
                return std::make_unique<SynchronizerStateSyncToPlayback>(m_pSynchronizer);

            case PLAY:
                m_pSynchronizer->playVideo(LEFT_CHANNEL);
                m_pSynchronizer->playVideo(RIGHT_CHANNEL);

                return nullptr;

            case PLAY_OR_PAUSE:
                // if we're in sync state and we get a play event, we'll just play both videos
                if (m_pSynchronizer->getVideoPlaybackStatus(LEFT_CHANNEL) == VideoPlaybackStatus::PAUSED &&
                    m_pSynchronizer->getVideoPlaybackStatus(RIGHT_CHANNEL) == VideoPlaybackStatus::PAUSED)
                {
                    m_pSynchronizer->playVideo(LEFT_CHANNEL);
                    m_pSynchronizer->playVideo(RIGHT_CHANNEL);
                }
                else
                {
                    m_pSynchronizer->pauseVideo(LEFT_CHANNEL);
                    m_pSynchronizer->pauseVideo(RIGHT_CHANNEL);
                }

                return nullptr;

            case PAUSE:
                // Pause both channels regardless of playback status
                m_pSynchronizer->pauseVideo(LEFT_CHANNEL);
                m_pSynchronizer->pauseVideo(RIGHT_CHANNEL);

                return nullptr;

            case STEP_FORWARD_SMALL:
                if (LEFT_CHANNEL == channel)
                    m_pSynchronizer->stepVideo(LEFT_CHANNEL, FRAME_STEP_SMALL);
                else if (RIGHT_CHANNEL == channel)
                    m_pSynchronizer->stepVideo(RIGHT_CHANNEL, FRAME_STEP_SMALL);
                else
                    m_logger.warn("SynchronizerStateSync: Invalid channel selected for frame step %d", channel);

                return nullptr;

            case STEP_FORWARD_LARGE:
                if (LEFT_CHANNEL == channel)
                    m_pSynchronizer->stepVideo(LEFT_CHANNEL, FRAME_STEP_LARGE);
                else if (RIGHT_CHANNEL == channel)
                    m_pSynchronizer->stepVideo(RIGHT_CHANNEL, FRAME_STEP_LARGE);
                else
                    m_logger.warn("SynchronizerStateSync: Invalid channel selected for frame step %d", channel);

                return nullptr;

            case STEP_BACKWARD_SMALL:
                if (LEFT_CHANNEL == channel)
                    m_pSynchronizer->stepVideo(LEFT_CHANNEL, -FRAME_STEP_SMALL);
                else if (RIGHT_CHANNEL == channel)
                    m_pSynchronizer->stepVideo(RIGHT_CHANNEL, -FRAME_STEP_SMALL);
                else
                    m_logger.warn("SynchronizerStateSync: Invalid channel selected for frame step %d", channel);

                return nullptr;

            case STEP_BACKWARD_LARGE:
                if (LEFT_CHANNEL == channel)
                    m_pSynchronizer->stepVideo(LEFT_CHANNEL, -FRAME_STEP_LARGE);
                else if (RIGHT_CHANNEL == channel)
                    m_pSynchronizer->stepVideo(RIGHT_CHANNEL, -FRAME_STEP_LARGE);
                else
                    m_logger.warn("SynchronizerStateSync: Invalid channel selected for frame step %d", channel);

                return nullptr;

            case VIDEO_POSITION_UPDATE:
                // This event occurs when the user moves the video position slider in the GUI. Trigger 
                // the video position update, note that the video position is loaded by the sync manager
                // from the GUI callback
                m_pSynchronizer->videoPositionUpdate(channel);

                return nullptr;

            case VIDEO_FRAME_UPDATE:
                // This event occurs when a video channel updates the currently displayed frame
                // during playback; we need to pass this through to the synchronizer to update the
                // scollbar position on the analyzer
                m_pSynchronizer->videoFrameUpdate(channel, false);
                return nullptr;

            case PLAYBACK_RATE_UPDATE:

                return nullptr;

            case LOAD_VIDEO:
                m_pSynchronizer->calculateFramerateRatio();
                return nullptr;

            default:
                // we shouldn't get any events other than the above
                m_logger.warn("Unhandled event from SYNC state on channel(s) %s: %s", 
                    channel_name_map.at(channel), sync_event_name_map.at(event));
                return nullptr;
        }
    }
}