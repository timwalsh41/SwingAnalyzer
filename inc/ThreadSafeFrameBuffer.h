#ifndef THREADSAFEFRAMEBUFFER_H
#define THREADSAFEFRAMEBUFFER_H

#include <Logger.h>

#include <opencv2/opencv.hpp>

#include <mutex>
namespace VideoPlayback
{

class ThreadSafeFrameBuffer {
public:
    ThreadSafeFrameBuffer();

    void updateFrame(const cv::Mat& new_frame);
    cv::Mat getDisplayFrame(const cv::Size& target_size);
    bool hasFrame() const;

private:
    cv::Mat m_frame_buffer;
    cv::Mat m_display_frame;
    mutable std::mutex m_frame_mutex;
    Logger& m_logger;
};

} // namespace VideoPlayback

#endif // THREADSAFEFRAMEBUFFER_H
