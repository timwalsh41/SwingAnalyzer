#include <ThreadSafeFrameBuffer.h>

namespace VideoPlayback
{

ThreadSafeFrameBuffer::ThreadSafeFrameBuffer() : logger_(Logger::get()) {
}

void ThreadSafeFrameBuffer::updateFrame(const cv::Mat& new_frame) {
    if (new_frame.empty()) {
        logger_.warn("Attempted to update frame buffer with empty frame");
        return;
    }
    
    std::lock_guard<std::mutex> lock(frame_mutex_);
    frame_buffer_ = new_frame.clone();
}

cv::Mat ThreadSafeFrameBuffer::getDisplayFrame(const cv::Size& target_size) {
    std::lock_guard<std::mutex> lock(frame_mutex_);
    
    if (frame_buffer_.empty()) {
        return cv::Mat();
    }
    
    if (target_size.width <= 0 || target_size.height <= 0) {
        return frame_buffer_.clone();
    }
    
    cv::resize(frame_buffer_, display_frame_, target_size);
    return display_frame_.clone();
}

bool ThreadSafeFrameBuffer::hasFrame() const {
    std::lock_guard<std::mutex> lock(frame_mutex_);
    return !frame_buffer_.empty();
}

} // namespace VideoPlayback
