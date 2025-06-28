#include <OpenCVVideoDecoder.h>

namespace VideoPlayback
{

OpenCVVideoDecoder::OpenCVVideoDecoder() 
    : is_loaded_(false), logger_(Logger::get()) {
}

VideoLoadStatus OpenCVVideoDecoder::loadVideo(const std::string& path) {
    video_capture_.open(path);
    if (!video_capture_.isOpened()) {
        logger_.error("Failed to open video file: %s", path.c_str());
        return VIDEO_LOAD_UNKNOWN_ERROR;
    }
    
    if (!loadProperties()) {
        logger_.error("Failed to load video properties for: %s", path.c_str());
        video_capture_.release();
        return VIDEO_LOAD_UNKNOWN_ERROR;
    }
    
    is_loaded_ = true;
    logger_.info("Successfully loaded video: %s (%dx%d, %.2f fps, %d frames)", 
                 path.c_str(), properties_.width, properties_.height, 
                 properties_.fps, properties_.frame_count);
    
    return VIDEO_LOAD_SUCCESS;
}

void OpenCVVideoDecoder::unloadVideo() {
    if (is_loaded_) {
        video_capture_.release();
        is_loaded_ = false;
        properties_ = VideoProperties{};
        logger_.debug("Video unloaded");
    }
}

bool OpenCVVideoDecoder::getNextFrame(cv::Mat& frame) {
    if (!is_loaded_) {
        return false;
    }
    
    try {
        video_capture_ >> frame;
        return !frame.empty();
    } catch (const cv::Exception& e) {
        logger_.error("Error reading next frame: %s", e.what());
        return false;
    }
}

bool OpenCVVideoDecoder::seekToFrame(int frame_number) {
    if (!is_loaded_) {
        return false;
    }
    
    if (frame_number < 0 || frame_number >= properties_.frame_count) {
        logger_.warn("Invalid frame number: %d (valid range: 0-%d)", 
                     frame_number, properties_.frame_count - 1);
        return false;
    }
    
    return video_capture_.set(cv::CAP_PROP_POS_FRAMES, frame_number);
}

bool OpenCVVideoDecoder::getCurrentFrame(cv::Mat& frame) {
    if (!is_loaded_) {
        return false;
    }
    
    // Get current frame without advancing
    int current_pos = static_cast<int>(video_capture_.get(cv::CAP_PROP_POS_FRAMES));
    bool success = getNextFrame(frame);
    if (success) {
        // Restore position
        video_capture_.set(cv::CAP_PROP_POS_FRAMES, current_pos);
    }
    return success;
}

VideoProperties OpenCVVideoDecoder::getProperties() const {
    return properties_;
}

bool OpenCVVideoDecoder::isLoaded() const {
    return is_loaded_;
}

int OpenCVVideoDecoder::getCurrentFrameIndex() const {
    if (!is_loaded_) {
        return -1;
    }
    
    return static_cast<int>(video_capture_.get(cv::CAP_PROP_POS_FRAMES));
}

bool OpenCVVideoDecoder::loadProperties() {
    if (!video_capture_.isOpened()) {
        return false;
    }
    
    properties_.frame_count = static_cast<int>(video_capture_.get(cv::CAP_PROP_FRAME_COUNT));
    properties_.fps = video_capture_.get(cv::CAP_PROP_FPS);
    properties_.width = static_cast<int>(video_capture_.get(cv::CAP_PROP_FRAME_WIDTH));
    properties_.height = static_cast<int>(video_capture_.get(cv::CAP_PROP_FRAME_HEIGHT));
    
    if (properties_.height > 0) {
        properties_.aspect_ratio = static_cast<float>(properties_.width) / static_cast<float>(properties_.height);
    }
    
    return properties_.frame_count > 0 && properties_.fps > 0 && 
           properties_.width > 0 && properties_.height > 0;
}

} // namespace VideoPlayback
