#include <VideoFrameManager.h>

namespace VideoPlayback
{

VideoFrameManager::VideoFrameManager() 
    : m_start_frame(0), m_end_frame(0), m_total_frames(0), m_logger(Logger::get())
{
}

void VideoFrameManager::setFrameRange(int start, int end, int total_frames)
{
    m_total_frames = total_frames;
    m_start_frame = std::max(0, start);
    m_end_frame = (end == -1) ? m_total_frames - 1 : std::min(end, m_total_frames - 1);
    validateFrameRange();
}

void VideoFrameManager::setStartFrame(int start)
{
    if (start < 0 || start >= m_end_frame)
    {
        m_logger.warn("Invalid start frame: %d (valid range: 0-%d)", start, m_end_frame - 1);
        return;
    }
    m_start_frame = start;
}

void VideoFrameManager::setEndFrame(int end)
{
    if (end == -1)
    {
        m_end_frame = m_total_frames - 1;
    }
    else if (end <= m_start_frame || end >= m_total_frames)
    {
        m_logger.warn("Invalid end frame: %d (valid range: %d-%d)", 
                     end, m_start_frame + 1, m_total_frames - 1);
        return;
    }
    else
    {
        m_end_frame = end;
    }
}

void VideoFrameManager::setTotalFrames(int total)
{
    if (total <= 0)
    {
        m_logger.warn("Invalid total frames: %d", total);
        return;
    }

    m_total_frames = total;
    m_start_frame = 0;
    m_end_frame = m_total_frames - 1;
}

bool VideoFrameManager::isValidFrame(int frame) const
{
    return frame >= m_start_frame && frame <= m_end_frame;
}

int VideoFrameManager::clampFrame(int frame) const
{
    return std::clamp(frame, m_start_frame, m_end_frame);
}

int VideoFrameManager::getEffectiveLength() const
{
    return m_end_frame - m_start_frame + 1;
}

int VideoFrameManager::getNextFrame(int current_frame) const
{
    if (current_frame >= m_end_frame)
    {
        return m_end_frame; // Already at end
    }
    return current_frame + 1;
}

int VideoFrameManager::getPreviousFrame(int current_frame) const
{
    if (current_frame <= m_start_frame)
    {
        return m_start_frame; // Already at start
    }
    return current_frame - 1;
}

int VideoFrameManager::stepFrame(int current_frame, int offset) const
{
    int new_frame = current_frame + offset;
    return clampFrame(new_frame);
}

void VideoFrameManager::validateFrameRange()
{
    if (m_start_frame >= m_end_frame)
    {
        m_logger.warn("Invalid frame range: start=%d, end=%d. Resetting to full range.", 
                     m_start_frame, m_end_frame);
        m_start_frame = 0;
        m_end_frame = std::max(0, m_total_frames - 1);
    }
}

} // namespace VideoPlayback
