#ifndef OPENCVVIDEODECODER_H
#define OPENCVVIDEODECODER_H

#include <IVideoDecoder.h>
#include <Logger.h>

#include <opencv2/opencv.hpp>

namespace VideoPlayback
{

class OpenCVVideoDecoder : public IVideoDecoder {
public:
    OpenCVVideoDecoder();
    ~OpenCVVideoDecoder() override = default;

    VideoLoadStatus loadVideo(const std::string& path) override;
    void unloadVideo() override;
    bool getNextFrame(cv::Mat& frame) override;
    bool seekToFrame(int frame_number) override;
    bool getCurrentFrame(cv::Mat& frame) override;
    VideoProperties getProperties() const override;
    bool isLoaded() const override;
    int getCurrentFrameIndex() const override;

private:
    cv::VideoCapture m_video_capture;
    VideoProperties m_properties;
    bool m_is_loaded;
    Logger& m_logger;

    bool loadProperties();
};

} // namespace VideoPlayback

#endif // OPENCVVIDEODECODER_H
