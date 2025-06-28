#ifndef THREADSAFEFRAMEBUFFER_H
#define THREADSAFEFRAMEBUFFER_H

#include <opencv2/opencv.hpp>
#include <mutex>
#include <Logger.h>

namespace VideoPlayback
{

class ThreadSafeFrameBuffer {
public:
    ThreadSafeFrameBuffer();
    
    void updateFrame(const cv::Mat& new_frame);
    cv::Mat getDisplayFrame(const cv::Size& target_size);
    bool hasFrame() const;
    
private:
    cv::Mat frame_buffer_;
    cv::Mat display_frame_;
    mutable std::mutex frame_mutex_;
    Logger& logger_;
};

} // namespace VideoPlayback

#endif // THREADSAFEFRAMEBUFFER_H
