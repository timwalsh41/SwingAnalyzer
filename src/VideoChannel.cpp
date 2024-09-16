#include <VideoChannel.h>
#include <VideoPlaybackConstants.h>

#include <thread>

namespace VideoPlayback
{

VideoChannel::VideoChannel(const std::string name, std::function<void(float)> m_update_video_position_callback)
    : m_name{name}, m_logger{Logger::get()}, m_update_video_position_callback{m_update_video_position_callback}
{
    m_pixbuf = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, false, 8, 100, 100);
    m_image = Goocanvas::Image::create(m_pixbuf, 0, 0);

    m_profiler_frame.reset();
    m_profiler_image.reset(true);

    // our dispatcher will trigger the update of the canvas image pixbuf
    // the dispatcher is triggered by the worker thread
    m_dispatcher.connect([this]() {
        cv::Mat resized_frame;
        cv::Mat rgb_frame;

        m_profiler_image.tick();

        {
            std::lock_guard<std::mutex> lock(m_frameMutex);
            cv::resize(m_frame, resized_frame, cv::Size(m_video_width, m_video_height));
        }

        cv::cvtColor(resized_frame, rgb_frame, cv::COLOR_BGR2RGB);
        m_pixbuf = Gdk::Pixbuf::create_from_data(rgb_frame.data, Gdk::COLORSPACE_RGB, false, 8, rgb_frame.cols,
                                                rgb_frame.rows, rgb_frame.step);


        m_image->property_pixbuf() = m_pixbuf;

        if (m_update_video_position)
        {
            // update video position accordingly
            updateVideoPosition(static_cast<float>(m_active_frame - m_start_frame) / (m_end_frame - m_start_frame),
                                false);
            m_update_video_position = false;
        }

        m_profiler_image.tock();
    });
}

VideoChannel::~VideoChannel()
{
}

void VideoChannel::playVideoWorkerThread()
{
    cv::Mat frame;

    m_frame_time_us = 1 / m_framerate_multiplier / m_frames_per_second * 1e6;

    m_logger.info("Starting playback of %s channel video with frame time %.2f ms", m_name.c_str(),
                  m_frame_time_us / 1000);

    // grab the first frame
    m_video_capture >> frame;

    while (m_active_frame < m_end_frame && m_continue_playback)
    {
        m_profiler_frame.tick();

        // set the frame index, since the video capture returns the index of the
        // frame to be captured next, we will subtract one here to reflect the frame
        // we are currently processing
        m_active_frame = m_video_capture.get(cv::CAP_PROP_POS_FRAMES) - 1;
        m_video_position = static_cast<float>(m_active_frame - m_start_frame) / (m_end_frame - m_start_frame);

        // set the internal flag to update video position
        m_update_video_position = true;

        if (frame.empty())
        {
            m_logger.warn("Frame %d is empty for %s channel, not updating frame", m_active_frame, m_name.c_str());
        }
        else
        {
            updateDisplayedFrame(frame);
        }

        m_video_capture >> frame;

        m_profiler_frame.tock();

        std::this_thread::sleep_until(m_profiler_frame.getTickTime() + std::chrono::microseconds(static_cast<long int>(m_frame_time_us)));
    }

    m_profiler_frame.dump();
    m_profiler_image.dump();
    m_profiler_frame.reset();
    m_profiler_image.reset(true);

    m_continue_playback = false;
    m_logger.info("Channel %s finished playback", m_name.c_str());
}

void VideoChannel::setVideoSize(int availableHeight, int availableWidth)
{
    if (availableHeight < 10 || availableWidth < 10)
    {
        m_logger.error("Invalid video size for %s video: %d x %d", m_name.c_str(), availableWidth, availableHeight);
        return;
    }

    if (static_cast<float>(availableWidth) / m_video_aspect_ratio <= availableHeight)
    {
        m_video_width = availableWidth;
        m_video_height = static_cast<float>(availableWidth) / m_video_aspect_ratio;
    }
    else
    {
        m_video_height = availableHeight;
        m_video_width = static_cast<float>(availableHeight) * m_video_aspect_ratio;
    }

    m_logger.info("Set %s channel video size to %d x %d", m_name.c_str(), m_video_width, m_video_height);
}

const VideoSize VideoChannel::getVideoSize()
{
    return {m_video_width, m_video_height, m_video_aspect_ratio};
}

void VideoChannel::playVideo()
{
    if (m_active_frame == m_end_frame)
    {
        m_active_frame = m_start_frame;
        m_video_capture.set(cv::CAP_PROP_POS_FRAMES, m_active_frame);
        m_logger.info("Reset %s channel video playback to start", m_name.c_str());
    }

    // start our worker thread to process the video frames
    m_continue_playback = true;
    std::thread workerThread(&VideoChannel::playVideoWorkerThread, this);
    workerThread.detach();
}

void VideoChannel::pauseVideo()
{
    m_continue_playback = false;
}

VideoLoadStatus VideoChannel::loadVideo(const std::string &videoPath)
{
    // TODO: check if path exists, if not return VIDEO_LOAD_FILE_NOT_FOUND
    // TODO: check if file format is supported, if not return VIDEO_LOAD_INVALID_TYPE
    m_video_capture.open(videoPath);
    if (!m_video_capture.isOpened())
    {
        return VIDEO_LOAD_UNKNOWN_ERROR;
    }

    m_video_capture >> m_frame;
    m_video_aspect_ratio = static_cast<float>(m_frame.cols) / static_cast<float>(m_frame.rows);
    m_frames_per_second = m_video_capture.get(cv::CAP_PROP_FPS);
    m_frame_count = m_video_capture.get(cv::CAP_PROP_FRAME_COUNT);
    m_start_frame = 0;
    m_end_frame = m_frame_count - 1;

    m_logger.info("Loaded %s channel video %s with size %d x %d (AR=%.2f), framerate %.2f, and %d frames",
                  m_name.c_str(), videoPath.c_str(), m_frame.cols, m_frame.rows, m_video_aspect_ratio,
                  m_frames_per_second, m_frame_count);

    // TODO: set actual video size...
    setVideoSize(800, 800);

    return VIDEO_LOAD_SUCCESS;
}

void VideoChannel::unloadVideo()
{
}

void VideoChannel::playOrPauseVideo()
{
    if (m_continue_playback)
    {
        m_continue_playback = false;
    }
    else
    {
        playVideo();
    }
}

void VideoChannel::stepFrame(int frame_offset)
{
    if (frame_offset == 0)
    {
        return;
    }

    int new_frame = m_active_frame + frame_offset;
    if (new_frame < m_start_frame || new_frame > m_end_frame)
    {
        m_logger.warn(
            "Invalid frame step, current active frame is %d, step %d results in frame index %d, valid range is (%d,%d)",
            m_active_frame, frame_offset, new_frame, m_start_frame, m_end_frame);
        return;
    }

    m_logger.debug("Stepping %s channel frame from %d to %d", m_name.c_str(), m_active_frame, new_frame);

    // stepping the frame will halt playback if active
    m_continue_playback = false;

    // this will require a video position update
    m_update_video_position = true;

    setActiveFrame(new_frame);
}

void VideoChannel::setActiveFrame(int frame_index)
{
    if (frame_index < m_start_frame || frame_index > m_end_frame)
    {
        m_logger.warn("Invalid frame index for active frame: %d, valid range is (%d,%d)", frame_index, m_start_frame,
                      m_end_frame);
        return;
    }

    // we will stop playback if the frame is set directly
    m_continue_playback = false;

    m_active_frame = frame_index;

    // load the frame for display
    cv::Mat frame;
    m_video_capture.set(cv::CAP_PROP_POS_FRAMES, m_active_frame);
    m_video_capture >> frame;
    updateDisplayedFrame(frame);
}

void VideoChannel::setStartFrame(int frame_index)
{
    if (frame_index < 0 || frame_index >= m_end_frame)
    {
        m_logger.debug("Invalid frame index for start frame: %d, valid range is (%d,%d)", frame_index, 0,
                       m_end_frame - 1);
        return;
    }

    // we will stop playback if the frame is set directly
    m_continue_playback = false;

    m_start_frame = frame_index;
}

void VideoChannel::setEndFrame(int frame_index)
{
    if (frame_index <= m_start_frame || frame_index >= m_frame_count)
    {
        m_logger.warn("Invalid frame index for end frame: %d, valid range is (%d,%d)", frame_index, m_start_frame + 1,
                      m_frame_count - 1);
        return;
    }

    // we will stop playback if the frame is set directly
    m_continue_playback = false;

    m_end_frame = frame_index;
}

void VideoChannel::setVideoPosition(float video_position)
{
    if (video_position < 0 || video_position > 1)
    {
        m_logger.warn("Invalid video position %.3f, valid range is (0.0,1.0)", video_position);
        return;
    }

    updateVideoPosition(video_position, true);
}

void VideoChannel::updateDisplayedFrame(const cv::Mat &frame)
{
    // we lock here as this function can be called from the worker thread
    {
        std::lock_guard<std::mutex> lock(m_frameMutex);
        m_frame = frame.clone();
    }

    // emit this dispatcher signal to trigger the update of the canvas image
    m_dispatcher.emit();
}

void VideoChannel::setFramerateMultiplier(float framerate_multiplier)
{
    if (framerate_multiplier > MAX_FRAMERATE_MULTIPLIER || framerate_multiplier < MIN_FRAMERATE_MULTIPLIER)
    {
        m_logger.warn("Invalid framerate multiplier %.3f, valid range is (%.3f,%.3f)", framerate_multiplier,
                      MIN_FRAMERATE_MULTIPLIER, MAX_FRAMERATE_MULTIPLIER);
        return;
    }

    // stop playback if we update the framerate
    m_continue_playback = false;
    m_framerate_multiplier = framerate_multiplier;
}

void VideoChannel::updateVideoPosition(float video_position, bool update_frame)
{
    m_video_position = video_position;

    if (update_frame)
    {
        // we will stop playback if the frame is set directly
        m_continue_playback = false;
        int new_frame = m_start_frame + static_cast<int>(m_video_position * (m_end_frame - m_start_frame));

        // make sure we don't trigger a subsequent video position update
        m_update_video_position = false;
        setActiveFrame(new_frame);
    }
    else
    {
        // trigger update of slider position in GUI
        m_update_video_position_callback(m_video_position);
    }
}

} // namespace VideoPlayback