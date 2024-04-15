#include <VideoChannel.h>
#include <thread>

VideoChannel::VideoChannel() :
    m_logger{Logger::get()}
{
    m_pixbuf = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, false, 8, 100, 100);
    m_image = Goocanvas::Image::create(m_pixbuf, 0, 0);

    // our dispatcher will trigger the update of the canvas image pixbuf
    // the dispatcher is triggered by the worker thread
    m_dispatcher.connect([this]() {
        std::lock_guard<std::mutex> lock(m_pixBufMutex);
        m_image->property_pixbuf() = m_pixbuf;
        // m_canvas->queue_draw();
    });
}

VideoChannel::~VideoChannel()
{

}

void VideoChannel::playVideoWorkerThread()
{
    while (!m_frame.empty())
    {
        cv::Mat resized_frame;
        cv::resize(m_frame, resized_frame, cv::Size(m_videoWidth, m_videoHeight));

        cv::Mat rgb_frame;
        cv::cvtColor(resized_frame, rgb_frame, cv::COLOR_BGR2RGB);

        {
            std::lock_guard<std::mutex> lock(m_pixBufMutex);
            m_pixbuf = Gdk::Pixbuf::create_from_data(rgb_frame.data, Gdk::COLORSPACE_RGB, false, 8, rgb_frame.cols,
                                                     rgb_frame.rows, rgb_frame.step);
        }

        m_dispatcher.emit();
        m_videoCapture >> m_frame;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void VideoChannel::setVideoSize(int availableHeight, int availableWidth)
{
    // TODO: check height/width inputs

    if (static_cast<float>(availableWidth) / m_videoAspectRatio <= availableHeight)
    {
        m_videoWidth = availableWidth;
        m_videoHeight = static_cast<float>(availableWidth) / m_videoAspectRatio;
    }
    else
    {
        m_videoHeight = availableHeight;
        m_videoWidth = static_cast<float>(availableHeight) * m_videoAspectRatio;
    }

    m_logger.logString(INFO, "Set video size to %d x %d", m_videoWidth, m_videoHeight);
}

const VideoSize VideoChannel::getVideoSize()
{
    return {m_videoWidth, m_videoHeight, m_videoAspectRatio};
}

void VideoChannel::playVideo()
{
    // start our worker thread to process the video frames
    std::thread workerThread(&VideoChannel::playVideoWorkerThread, this);
    workerThread.detach();
}

VideoLoadStatus VideoChannel::loadVideo(const std::string &videoPath)
{
    // TODO: check if path exists, if not throw VIDEO_LOAD_FILE_NOT_FOUND
    // TODO: check if file format is supported, if not throw VIDEO_LOAD_INVALID_TYPE
    m_videoCapture.open(videoPath);
    if (!m_videoCapture.isOpened())
    {
        return VIDEO_LOAD_UNKNOWN_ERROR;
    }

    m_videoCapture >> m_frame;
    m_videoAspectRatio = static_cast<float>(m_frame.cols) / static_cast<float>(m_frame.rows);
    m_fps = m_videoCapture.get(cv::CAP_PROP_FPS);
    m_frame_count = m_videoCapture.get(cv::CAP_PROP_FRAME_COUNT);

    m_logger.logString(INFO, "Loaded video %s with size %d x %d (AR=%.2f), framerate %.2f, and %d frames", 
        videoPath.c_str(), m_frame.cols, m_frame.rows, m_videoAspectRatio, m_fps, m_frame_count);

    // TODO: set actual video size...
    setVideoSize(800, 800);

    return VIDEO_LOAD_SUCCESS;
}

void VideoChannel::unloadVideo()
{

}