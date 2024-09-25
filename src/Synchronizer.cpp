#include <Synchronizer.h>

namespace VideoPlayback
{
    Synchronizer::Synchronizer(VideoChannel *channel_left, VideoChannel *channel_right) : 
        m_pChannel_left(channel_left), m_pChannel_right(channel_right), m_logger(Logger::get())
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
    
} // namespace VideoPlayback