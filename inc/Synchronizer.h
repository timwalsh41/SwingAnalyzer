#ifndef SYNCHRONIZER_H
#define SYNCHRONIZER_H

#include <ISynchronizerActions.h>
#include <IVideoChannel.h>
#include <Logger.h>

#include <memory>

namespace VideoPlayback
{

class Synchronizer : public ISynchronizerActions
{
    public:
        Synchronizer(std::shared_ptr<IVideoChannel> channel_left, std::shared_ptr<IVideoChannel> channel_right);
        ~Synchronizer() = default;
        
        // ============ ISynchronizerActions implementation ============
        // Joint video control
        void playVideos() override;
        void pauseVideos() override;
        void setVideosToStartPosition() override;
        void stepVideos(int step_size) override;
        
        // Individual channel control (for sync state)
        void playVideo(WhichVideoChannel channel) override;
        void pauseVideo(WhichVideoChannel channel) override;
        void stepVideo(WhichVideoChannel channel, int step_size) override;
        VideoPlaybackStatus getVideoPlaybackStatus(WhichVideoChannel channel) const override;
        
        // Synchronization actions
        void setupVideoSync() override;
        void calculateFramerateRatio() override;
        void clearStartAndEndFrames() override;
        void setVideoFrameOffset(int offset) override;
        void videoPositionUpdate(WhichVideoChannel channel) override;
        void videoFrameUpdate(WhichVideoChannel channel, bool update_joint_slider) override;
        
        // State accessors
        WhichVideoChannel getVideoPositionUpdateChannel() const override;
        float getNewVideoPosition() const override;
        void setNewVideoPosition(float position) override;
        int getVideoFrameOffset() const override;

        // ============ Other public methods ============
        // (mainly used for testing)
        float getFramerateRatio() const { return m_framerate_ratio; }

    private:
        std::shared_ptr<IVideoChannel> m_pChannel_left;
        std::shared_ptr<IVideoChannel> m_pChannel_right;
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

        // when running in playback mode, we'll use the frame update from only one channel
        // to trigger updates to the slider; we'll set the appropriate channel upon transition
        // from sync to playback mode
        WhichVideoChannel m_video_position_update_channel{NO_CHANNEL};
};

} // namespace VideoPlayback

#endif // SYNCHRONIZERIMPL_H