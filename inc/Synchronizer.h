#ifndef SYNCHRONIZER_H
#define SYNCHRONIZER_H

#include <ISynchronizerActions.h>
#include <IVideoSynchronizationObserver.h>
#include <Logger.h>
#include <VideoChannel.h>

#include <memory>

namespace VideoPlayback
{

class Synchronizer : public ISynchronizerActions
{
    // The SyncManager and its states are friend classes that can manipulate
    // synchronizer data as needed
    friend class SyncManager;
    friend class SynchronizerStateSyncToPlayback;
    friend class SynchronizerStateSync;
    friend class SynchronizerStatePlaybackPlaying;
    friend class SynchronizerStatePlaybackPaused;

    public:
        Synchronizer(std::shared_ptr<VideoChannel> channel_left, std::shared_ptr<VideoChannel> channel_right);
        ~Synchronizer() = default;
        
        // Observer management
        void setObserver(std::weak_ptr<IVideoSynchronizationObserver> observer);
        
        // ISynchronizerActions implementation
        void playVideos() override;
        void pauseVideos() override;
        void setVideosToStartPosition() override;
        void stepVideos(int step_size) override;
        
        // Individual channel control (for sync state)
        void playVideo(WhichVideoChannel channel) override;
        void pauseVideo(WhichVideoChannel channel) override;
        void stepVideo(WhichVideoChannel channel, int step_size) override;
        VideoPlaybackStatus getVideoPlaybackStatus(WhichVideoChannel channel) const override;
        
        void setupVideoSync() override;
        void calculateFramerateRatio() override;
        void clearStartAndEndFrames() override;
        void setVideoFrameOffset(int offset) override;
        void videoPositionUpdate(WhichVideoChannel channel) override;
        void videoFrameUpdate(WhichVideoChannel channel, bool update_joint_slider) override;
        
        WhichVideoChannel getVideoPositionUpdateChannel() const override;
        float getNewVideoPosition() const override;
        void setNewVideoPosition(float position) override;
        int getNewActiveFrame() const override;
        void setNewActiveFrame(int frame) override;
        void notifyVideoPositionChanged(WhichVideoChannel channel, float position) override;
        int getVideoFrameOffset() const override;

    private:
        std::weak_ptr<IVideoSynchronizationObserver> m_observer;
        std::shared_ptr<VideoChannel> m_pChannel_left, m_pChannel_right;
        Logger &m_logger;

        // synchronized video offset (left channel position - right channel position)
        int m_video_frame_offset{0};

        // framerate ratio, left video framerate divided by right video framerate
        float m_framerate_ratio{1.0};

        // effective length of the left and right video channels, including user selected
        // start/end frame and offset
        int m_left_length{0}, m_right_length{0};
        
        // the combined length is the shorter of the right and left channels
        int m_combined_length{0};

        // flag for continuous playback mode, as selected by user
        bool m_continuous_playback{false};

        // member variable to hold the new position setpoint from the GUI callback
        // for the joint slider
        float m_new_video_position{0};

        // member variable to hold the new active frame from the video channel frame
        // update callback
        int m_new_active_frame{0};

        // when running in playback mode, we'll use the frame update from only one channel
        // to trigger updates to the slider; we'll set the appropriate channel upon transition
        // from sync to playback mode
        WhichVideoChannel m_video_position_update_channel{NO_CHANNEL};
};

} // namespace VideoPlayback

#endif // SYNCHRONIZERIMPL_H