#include <ThreadSafeFrameBuffer.h>

namespace VideoPlayback
{

ThreadSafeFrameBuffer::ThreadSafeFrameBuffer() : m_logger(Logger::get()) 
{
}

void ThreadSafeFrameBuffer::updateFrame(const cv::Mat& new_frame)
{
    if (new_frame.empty())
    {
        m_logger.warn("Attempted to update frame buffer with empty frame");
        return;
    }

    std::lock_guard<std::mutex> lock(m_frame_mutex);
    m_frame_buffer = new_frame.clone();
}

cv::Mat ThreadSafeFrameBuffer::getDisplayFrame(const cv::Size& target_size)
{
    std::lock_guard<std::mutex> lock(m_frame_mutex);

    if (m_frame_buffer.empty())
    {
        return cv::Mat();
    }

    if (target_size.width <= 0 || target_size.height <= 0)
    {
        return m_frame_buffer.clone();
    }

    cv::resize(m_frame_buffer, m_display_frame, target_size);
    return m_display_frame.clone();
}

bool ThreadSafeFrameBuffer::hasFrame() const
{
    std::lock_guard<std::mutex> lock(m_frame_mutex);
    return !m_frame_buffer.empty();
}

} // namespace VideoPlayback
