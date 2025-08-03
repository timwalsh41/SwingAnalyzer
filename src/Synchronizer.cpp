#include <Synchronizer.h>

#include <algorithm>

namespace VideoPlayback
{
    Synchronizer::Synchronizer(std::shared_ptr<VideoChannel> channel_left, std::shared_ptr<VideoChannel> channel_right) : 
        m_pChannel_left(channel_left), m_pChannel_right(channel_right), m_logger(Logger::get())
    {
    }

    void Synchronizer::playVideos()
    {
        m_pChannel_left->playVideo();
        m_pChannel_right->playVideo();
    }

    void Synchronizer::playVideo(WhichVideoChannel channel)
    {
        switch(channel)
        {
            case LEFT_CHANNEL:
                m_pChannel_left->playVideo();
                break;
            case RIGHT_CHANNEL:
                m_pChannel_right->playVideo();
                break;
            case BOTH_CHANNELS:
                playVideos();
                break;
            default:
                m_logger.warn("Synchronizer::playVideo: Invalid channel %d", static_cast<int>(channel));
                break;
        }
    }

    void Synchronizer::pauseVideo(WhichVideoChannel channel)
    {
        switch(channel)
        {
            case LEFT_CHANNEL:
                m_pChannel_left->pauseVideo();
                break;
            case RIGHT_CHANNEL:
                m_pChannel_right->pauseVideo();
                break;
            case BOTH_CHANNELS:
                pauseVideos();
                break;
            default:
                m_logger.warn("Synchronizer::pauseVideo: Invalid channel %d", static_cast<int>(channel));
                break;
        }
    }

    void Synchronizer::stepVideo(WhichVideoChannel channel, int step_size)
    {
        switch(channel)
        {
            case LEFT_CHANNEL:
                m_pChannel_left->stepFrame(step_size);
                break;
            case RIGHT_CHANNEL:
                m_pChannel_right->stepFrame(step_size);
                break;
            default:
                m_logger.warn("Synchronizer::stepVideo: Invalid channel %d", static_cast<int>(channel));
                break;
        }
    }

    VideoPlaybackStatus Synchronizer::getVideoPlaybackStatus(WhichVideoChannel channel) const
    {
        switch(channel)
        {
            case LEFT_CHANNEL:
                return m_pChannel_left->getPlaybackStatus();
            case RIGHT_CHANNEL:
                return m_pChannel_right->getPlaybackStatus();
            default:
                m_logger.warn("Synchronizer::getVideoPlaybackStatus: Invalid channel %d", static_cast<int>(channel));
                return VideoPlaybackStatus::PAUSED;
        }
    }

    void Synchronizer::setVideoFrameOffset(int offset)
    {
        m_video_frame_offset = offset;
    }

    void Synchronizer::clearStartAndEndFrames()
    {
        m_pChannel_left->setStartFrame(0);
        m_pChannel_left->setEndFrame(-1);
        m_pChannel_right->setStartFrame(0);
        m_pChannel_right->setEndFrame(-1);
    }

    void Synchronizer::setVideosToStartPosition()
    {
        m_pChannel_right->setVideoToStart();
        m_pChannel_left->setVideoToStart();
    }

    void Synchronizer::setupVideoSync()
    {
        m_video_frame_offset = (m_pChannel_left->getActiveFrame() - m_pChannel_left->getStartFrame()) -
                               (m_pChannel_right->getActiveFrame() - m_pChannel_right->getStartFrame());

        if (m_video_frame_offset > 0)
        {
            m_pChannel_left->setStartFrame(m_pChannel_left->getStartFrame() + m_video_frame_offset);
        }
        else
        {
            m_pChannel_right->setStartFrame(m_pChannel_right->getStartFrame() - m_video_frame_offset);
        }

        m_left_length = m_pChannel_left->getEndFrame() - m_pChannel_left->getStartFrame();
        m_right_length = m_pChannel_right->getEndFrame() - m_pChannel_right->getStartFrame();
        m_combined_length = std::min(m_left_length, m_right_length);

        if (m_left_length < m_right_length)
        {
            m_video_position_update_channel = LEFT_CHANNEL;
        }
        else
        {
            m_video_position_update_channel = RIGHT_CHANNEL;
        }
    }

    void Synchronizer::calculateFramerateRatio()
    {
        if (!m_pChannel_left->IsVideoLoaded() or !m_pChannel_right->IsVideoLoaded())
        {
            m_logger.info("Cannot calculate framerate ratio, one or both videos are not loaded");
            return;
        }

        m_framerate_ratio = m_pChannel_left->getRawFramerate_fps() / m_pChannel_right->getRawFramerate_fps();
    }

    void Synchronizer::stepVideos(int step_size)
    {
        // Here we pass the step command on to the video channels, note that the frame
        // updates within the channels will trigger the scrollbar update

        float step_size_left, step_size_right;

        if (m_framerate_ratio > 1)
        {
            step_size_left = step_size* m_framerate_ratio;
            step_size_right = step_size;
        }
        else
        {
            step_size_left = step_size;
            step_size_right = step_size / m_framerate_ratio;
        }

        m_pChannel_left->stepFrame(static_cast<int>(step_size_left));
        m_pChannel_right->stepFrame(static_cast<int>(step_size_right));
    }

    void Synchronizer::pauseVideos()
    {
        m_pChannel_left->pauseVideo();
        m_pChannel_right->pauseVideo();
    }

    void Synchronizer::videoPositionUpdate(WhichVideoChannel channel)
    {
        if (m_new_video_position < 0 || m_new_video_position > 1)
        {
            m_logger.warn("Invalid video position %.3f, valid range is (0.0,1.0)", m_new_video_position);
            return;
        }

        int left_frame{0}, right_frame{0}, left_video_length{0}, right_video_length{0};

        // Which channel we handle, and how we handle it, will depend on the state
        // - In sync state, we'll get left or right frame, and we'll simply set the video
        //   to the frame corresponding to the video position between the start and end frame
        // - In playback mode, both videos must respect the combined length, so we'll calculate
        //   the frame for each based on that
        switch(channel)
        {
            case LEFT_CHANNEL:
                left_video_length = m_pChannel_left->getEndFrame() - m_pChannel_left->getStartFrame();
                left_frame = m_pChannel_left->getStartFrame() + static_cast<int>(m_new_video_position * left_video_length);
                m_pChannel_left->seekToFrame(left_frame);
                break;
            
            case RIGHT_CHANNEL:
                right_video_length = m_pChannel_right->getEndFrame() - m_pChannel_right->getStartFrame();
                right_frame = m_pChannel_right->getStartFrame() + static_cast<int>(m_new_video_position * right_video_length);
                m_pChannel_right->seekToFrame(right_frame);
                break;

            case BOTH_CHANNELS:
                left_frame = m_pChannel_left->getStartFrame() + static_cast<int>(m_new_video_position * m_combined_length);
                right_frame = m_pChannel_right->getStartFrame() + static_cast<int>(m_new_video_position * m_combined_length);
                m_pChannel_left->seekToFrame(left_frame);
                m_pChannel_right->seekToFrame(right_frame);
                break;

            default:
                m_logger.warn("Synchronizer: received video position update without channel selected");
                break;
        }
    }

    void Synchronizer::videoFrameUpdate(WhichVideoChannel channel, bool update_joint_slider)
    {
        // Calculate the new video position based on the current frame and store it
        // for retrieval by the calling component
        float new_position;
        int video_length;
        VideoChannel* pChannel;

        // if this is a call to update the joint slider (e.g. we are in playback mode not sync mode), then
        // discard if this is not for the active channel we use for video position tracking
        auto no_slider_update = ((channel != m_video_position_update_channel) && update_joint_slider);
        if (no_slider_update)
        {
            return;
        }
        
        if (channel == WhichVideoChannel::LEFT_CHANNEL)
        {
            pChannel = m_pChannel_left.get();
        }
        else if (channel == WhichVideoChannel::RIGHT_CHANNEL)
        {
            pChannel = m_pChannel_right.get();
        }

        video_length = pChannel->getEndFrame() - pChannel->getStartFrame();

        if (video_length == 0)
        {
            m_new_video_position = 0.;
            m_logger.warn("Synchronizer::videoFrameUpdate: video length is zero, setting position to 0");
        }
        else
        {
            m_new_video_position = static_cast<float>(pChannel->getActiveFrame() - pChannel->getStartFrame()) 
                            / (pChannel->getEndFrame() - pChannel->getStartFrame());
        }

    }

    WhichVideoChannel Synchronizer::getVideoPositionUpdateChannel() const
    {
        return m_video_position_update_channel;
    }

    float Synchronizer::getNewVideoPosition() const
    {
        return m_new_video_position;
    }

    void Synchronizer::setNewVideoPosition(float position)
    {
        if (0.0 <= position && position <= 1.0)
        {
            m_new_video_position = position;
        }
        else
        {
            m_logger.warn("Synchronizer::setNewVideoPosition: invalid position argument received: %:.3f", position);
        }
    }

    int Synchronizer::getVideoFrameOffset() const
    {
        return m_video_frame_offset;
    }

} // namespace VideoPlayback