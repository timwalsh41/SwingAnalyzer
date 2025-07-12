#include <OpenCVVideoDecoder.h>

#include <filesystem>

namespace VideoPlayback
{

OpenCVVideoDecoder::OpenCVVideoDecoder() 
    : m_is_loaded(false), m_logger(Logger::get()) {
}

VideoLoadStatus OpenCVVideoDecoder::loadVideo(const std::string& path)
{
    if (!std::filesystem::exists(path))
    {
        m_logger.error("Video file not found: %s", path.c_str());
        return VIDEO_LOAD_FILE_NOT_FOUND;
    }

    m_video_capture.open(path);

    if (!m_video_capture.isOpened())
    {
        m_logger.error("Failed to open video file: %s", path.c_str());
        return VIDEO_LOAD_ERROR;
    }

    if (!loadProperties())
    {
        m_logger.error("Failed to load video properties for: %s", path.c_str());
        m_video_capture.release();
        return VIDEO_LOAD_ERROR;
    }

    m_is_loaded = true;
    m_logger.info("Successfully loaded video: %s (%dx%d, %.2f fps, %d frames)", 
                 path.c_str(), m_properties.width, m_properties.height, 
                 m_properties.fps, m_properties.frame_count);

    return VIDEO_LOAD_SUCCESS;
}

bool OpenCVVideoDecoder::unloadVideo()
{
    if (m_is_loaded)
    {
        m_video_capture.release();
        m_is_loaded = false;
        m_properties = VideoProperties{};
        m_logger.debug("Video unloaded");
    }

    return true;
}

bool OpenCVVideoDecoder::getNextFrame(cv::Mat& frame)
{
    if (!m_is_loaded)
    {
        return false;
    }

    try
    {
        m_video_capture >> frame;
        return !frame.empty();
    } 
    catch (const cv::Exception& e) 
    {
        m_logger.error("Error reading next frame: %s", e.what());
        return false;
    }
}

bool OpenCVVideoDecoder::seekToFrame(int frame_number) 
{
    if (!m_is_loaded)
    {
        return false;
    }

    if (frame_number < 0 || frame_number >= m_properties.frame_count)
    {
        m_logger.warn("Invalid frame number: %d (valid range: 0-%d)", 
                     frame_number, m_properties.frame_count - 1);
        return false;
    }

    return m_video_capture.set(cv::CAP_PROP_POS_FRAMES, frame_number);
}

bool OpenCVVideoDecoder::getCurrentFrame(cv::Mat& frame)
{
    if (!m_is_loaded)
    {
        return false;
    }

    // Get current frame without advancing
    int current_pos = static_cast<int>(m_video_capture.get(cv::CAP_PROP_POS_FRAMES));
    bool success = getNextFrame(frame);
    if (success)
    {
        // Restore position
        m_video_capture.set(cv::CAP_PROP_POS_FRAMES, current_pos);
    }
    return success;
}

VideoProperties OpenCVVideoDecoder::getProperties() const
{
    return m_properties;
}

bool OpenCVVideoDecoder::isLoaded() const
{
    return m_is_loaded;
}

int OpenCVVideoDecoder::getCurrentFrameIndex() const
{
    if (!m_is_loaded)
    {
        return -1;
    }

    return static_cast<int>(m_video_capture.get(cv::CAP_PROP_POS_FRAMES));
}

bool OpenCVVideoDecoder::loadProperties()
{
    if (!m_video_capture.isOpened())
    {
        return false;
    }

    m_properties.frame_count = static_cast<int>(m_video_capture.get(cv::CAP_PROP_FRAME_COUNT));
    m_properties.fps = m_video_capture.get(cv::CAP_PROP_FPS);
    m_properties.width = static_cast<int>(m_video_capture.get(cv::CAP_PROP_FRAME_WIDTH));
    m_properties.height = static_cast<int>(m_video_capture.get(cv::CAP_PROP_FRAME_HEIGHT));

    if (m_properties.height > 0)
    {
        m_properties.aspect_ratio = static_cast<float>(m_properties.width) / static_cast<float>(m_properties.height);
    }

    return m_properties.frame_count > 0 && m_properties.fps > 0 && 
           m_properties.width > 0 && m_properties.height > 0;
}

} // namespace VideoPlayback
