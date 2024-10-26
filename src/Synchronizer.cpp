#include <Synchronizer.h>
#include <Analyzer.h>

#include <algorithm>
#include <iostream>

namespace VideoPlayback
{
    Synchronizer::Synchronizer(Analyzer* analyzer, VideoChannel *channel_left, VideoChannel *channel_right) : 
        m_pAnalyzer{analyzer}, m_pChannel_left(channel_left), m_pChannel_right(channel_right), m_logger(Logger::get())
    {

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

        int left_frame, right_frame;

        left_frame = m_pChannel_left->getStartFrame() + static_cast<int>(m_new_video_position * m_combined_length);
        right_frame = m_pChannel_right->getStartFrame() + static_cast<int>(m_new_video_position * m_combined_length);

        switch(channel)
        {
            case LEFT_CHANNEL:
                m_pChannel_left->setVideoToFrame(left_frame);
                break;
            
            case RIGHT_CHANNEL:
                m_pChannel_right->setVideoToFrame(right_frame);
                break;

            case BOTH_CHANNELS:
                m_pChannel_left->setVideoToFrame(left_frame);
                m_pChannel_right->setVideoToFrame(right_frame);
                break;

            default:
                m_logger.warn("Synchronizer: received video position update without channel selected");
                break;
        }
    }

    void Synchronizer::videoFrameUpdate(WhichVideoChannel channel, bool update_joint_slider)
    {
        float new_position;
        int video_length;
        VideoChannel* pChannel;

        if (channel != m_video_position_update_channel && update_joint_slider)
        {
            // we only update the slider for one channel in playback mode (e.g. when updating the
            // joint slider)
            return;
        }
        
        if (channel == WhichVideoChannel::LEFT_CHANNEL)
        {
            pChannel = m_pChannel_left;
        }
        else if (channel == WhichVideoChannel::RIGHT_CHANNEL)
        {
            pChannel = m_pChannel_right;
        }

        video_length = pChannel->getEndFrame() - pChannel->getStartFrame();

        if (video_length == 0)
        {
            new_position = 0.;
            m_logger.warn("Synchronizer::videoFrameUpdate: video length is zero, setting position to 0");
            
        }
        else
        {
            new_position = static_cast<float>(pChannel->getActiveFrame() - pChannel->getStartFrame()) 
                            / (pChannel->getEndFrame() - pChannel->getStartFrame());
        }

        if  (update_joint_slider)
        {
            std::cout << "Slider update " << channel_name_map.at(channel) << " " << pChannel->getActiveFrame() << "/" << pChannel->getEndFrame() << " = " << new_position << "\n";
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