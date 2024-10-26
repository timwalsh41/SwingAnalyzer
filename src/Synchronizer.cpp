#include <Synchronizer.h>
#include <Analyzer.h>

#include <algorithm>

namespace VideoPlayback
{
    Synchronizer::Synchronizer(Analyzer* analyzer, VideoChannel *channel_left, VideoChannel *channel_right) : 
        m_pAnalyzer{analyzer}, m_pChannel_left(channel_left), m_pChannel_right(channel_right), m_logger(Logger::get())
    {

    }

    void Synchronizer::setVideosToStartPosition()
    {
        if (m_video_frame_offset > 0)
        {
            m_pChannel_left->setVideoToFrame(m_pChannel_left->getStartFrame() + m_video_frame_offset);
            m_pChannel_right->setVideoToStart();
        }
        else
        {
            m_pChannel_left->setVideoToStart();
            m_pChannel_right->setVideoToFrame(m_pChannel_right->getStartFrame() - m_video_frame_offset);
        }
    }

    void Synchronizer::setVideoChannelOffset()
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

    void Synchronizer::videoPositionUpdate()
    {
        if (m_new_video_position < 0 || m_new_video_position > 1)
        {
            m_logger.warn("Invalid video position %.3f, valid range is (0.0,1.0)", m_new_video_position);
            return;
        }

        int left_length, right_length, combined_length, left_frame, right_frame;

        if (m_video_frame_offset > 0)
        {
            left_length = m_pChannel_left->getEndFrame() - m_pChannel_left->getStartFrame() - m_video_frame_offset;
            right_length = m_pChannel_right->getEndFrame() - m_pChannel_right->getStartFrame();
            combined_length = std::min(left_length, right_length);
            left_frame = m_pChannel_left->getStartFrame() + m_video_frame_offset + static_cast<int>(m_new_video_position * combined_length);
            right_frame = m_pChannel_right->getStartFrame() + static_cast<int>(m_new_video_position * combined_length);
        }
        else
        {
            left_length = m_pChannel_left->getEndFrame() - m_pChannel_left->getStartFrame();
            right_length = m_pChannel_right->getEndFrame() - m_pChannel_right->getStartFrame() + m_video_frame_offset;
            combined_length = std::min(left_length, right_length);
            left_frame = m_pChannel_left->getStartFrame() + static_cast<int>(m_new_video_position * combined_length);
            right_frame = m_pChannel_right->getStartFrame() - m_video_frame_offset + static_cast<int>(m_new_video_position * combined_length);
        }

        m_pChannel_left->setVideoToFrame(left_frame);
        m_pChannel_right->setVideoToFrame(right_frame);
    }

    void Synchronizer::videoFrameUpdate(WhichVideoChannel channel, bool update_joint_slider)
    {
        float new_position;
        VideoChannel* pChannel;
        
        if (channel == WhichVideoChannel::LEFT_CHANNEL)
        {
            pChannel = m_pChannel_left;
        }
        else if (channel == WhichVideoChannel::RIGHT_CHANNEL)
        {
            pChannel = m_pChannel_right;
        }

        new_position = static_cast<float>(pChannel->getActiveFrame() - pChannel->getStartFrame()) 
                        / (pChannel->getEndFrame() - pChannel->getStartFrame());

        if  (update_joint_slider)
        {
            m_pAnalyzer->updateCombinedVideoPosition(new_position);
        }
        else
        {
            if (channel == LEFT_CHANNEL)
            {
                m_pAnalyzer->updateLeftVideoPosition(new_position);
            }
            else if (channel == RIGHT_CHANNEL)
            {
                m_pAnalyzer->updateRightVideoPosition(new_position);
            }
        }
    }

} // namespace VideoPlayback