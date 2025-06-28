#ifndef IVIDEODECODER_H
#define IVIDEODECODER_H

#include <VideoPlaybackTypes.h>
#include <opencv2/opencv.hpp>
#include <string>

namespace VideoPlayback
{

struct VideoProperties {
    int frame_count = 0;
    float fps = 0.0f;
    int width = 0;
    int height = 0;
    float aspect_ratio = 0.0f;
};

class IVideoDecoder {
public:
    virtual ~IVideoDecoder() = default;
    
    virtual VideoLoadStatus loadVideo(const std::string& path) = 0;
    virtual void unloadVideo() = 0;
    virtual bool getNextFrame(cv::Mat& frame) = 0;
    virtual bool seekToFrame(int frame_number) = 0;
    virtual bool getCurrentFrame(cv::Mat& frame) = 0;
    virtual VideoProperties getProperties() const = 0;
    virtual bool isLoaded() const = 0;
    virtual int getCurrentFrameIndex() const = 0;
};

} // namespace VideoPlayback

#endif // IVIDEODECODER_H
