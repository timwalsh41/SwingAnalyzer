#include <VideoChannel.h>
#include <VideoPlaybackConstants.h>

#include <thread>
#include <algorithm>
#include <iostream>

namespace VideoPlayback
{

VideoChannel::VideoChannel(const WhichVideoChannel channel, std::function<void(int,WhichVideoChannel)> update_video_position_callback)
    : m_channel_id{channel}, m_logger{Logger::get()}, m_update_video_position_callback{update_video_position_callback}
{
    m_pixbuf = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, false, 8, 100, 100);
    m_image = Goocanvas::Image::create(m_pixbuf, 0, 0);

    m_profiler_frame.reset();
    m_profiler_image.reset(true);

    // our dispatcher will trigger the update of the canvas image pixbuf
    // the dispatcher is triggered by the worker thread
    m_frame_update_dispatcher.connect([this]() {
        m_profiler_image.tick();

        // thread safe copy of frame buffer into display frame
        {
            std::lock_guard<std::mutex> lock(m_frameMutex);
            cv::resize(m_frame_buffer, m_display_frame, cv::Size(m_video_width, m_video_height));
        }

        updatePixBuf();

        // update video position slider accordingly
        m_update_video_position_callback(m_active_frame, m_channel_id);

        m_profiler_image.tock();
    });

    m_playback_complete_dispatcher.connect([this]() {
        if (m_video_complete_callback)
        {
            m_video_complete_callback(m_channel_id);
        }
    });
}

VideoChannel::~VideoChannel()
{
}

void VideoChannel::playVideoWorkerThread()
{
    cv::Mat frame;

    m_frame_time_us = 1 / m_framerate_multiplier / m_frames_per_second * 1e6;

    m_logger.info("Starting playback of %s channel video with frame time %.2f ms", channel_name_map.at(m_channel_id),
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

        if (frame.empty())
        {
            m_logger.warn("Frame %d is empty for %s channel, not updating frame", m_active_frame, channel_name_map.at(m_channel_id));
        }
        else
        {
            // thread safe copy of the new frame into the frame buffer
            {
                std::lock_guard<std::mutex> lock(m_frameMutex);
                m_frame_buffer = frame.clone();
            }

            // trigger the dispatcher to update the canvas image
            m_frame_update_dispatcher.emit();
        }

        // TODO: figure out what's happening here...
        try
        {
            m_video_capture >> frame;
        }
        catch (cv::Exception &e)
        {
            m_logger.error("Error reading frame %d from %s channel video: %s", m_active_frame, channel_name_map.at(m_channel_id), e.what());
            std::cout << "Error reading frame " << m_active_frame << " from " << channel_name_map.at(m_channel_id) << ": " << e.what() << "\n";
        }

        m_profiler_frame.tock();

        std::this_thread::sleep_until(m_profiler_frame.getTickTime() + std::chrono::microseconds(static_cast<long int>(m_frame_time_us)));
    }

    m_profiler_frame.dump();
    m_profiler_image.dump();
    m_profiler_frame.reset();
    m_profiler_image.reset(true);

    m_playback_complete_dispatcher.emit();

    m_continue_playback = false;
    m_logger.info("Channel %s finished playback", channel_name_map.at(m_channel_id));
}

void VideoChannel::updatePixBuf()
{
    cv::Mat rgb_frame;

    cv::cvtColor(m_display_frame, rgb_frame, cv::COLOR_BGR2RGB);
    m_pixbuf = Gdk::Pixbuf::create_from_data(rgb_frame.data, Gdk::COLORSPACE_RGB, false, 8, rgb_frame.cols,
                                            rgb_frame.rows, rgb_frame.step);

    m_image->property_pixbuf() = m_pixbuf;
}

void VideoChannel::setVideoSize(int availableHeight, int availableWidth)
{
    if (!m_video_loaded)
    {
        return;
    }

    if (availableHeight < 10 || availableWidth < 10)
    {
        m_logger.error("Invalid video size for %s video: %d x %d", channel_name_map.at(m_channel_id), availableWidth, availableHeight);
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

    m_logger.info("Set %s channel video size to %d x %d", channel_name_map.at(m_channel_id), m_video_width, m_video_height);
}

const VideoSize VideoChannel::getVideoSize()
{
    if (m_video_loaded)
    {
        return {m_video_width, m_video_height, m_video_aspect_ratio};
    }
    else
    {
        return {0, 0, 0};
    }
}

void VideoChannel::playVideo()
{
    if (!m_video_loaded)
    {
        return;
    }

    if (m_active_frame == m_end_frame)
    {
        m_active_frame = m_start_frame;
        m_video_capture.set(cv::CAP_PROP_POS_FRAMES, m_active_frame);
        m_logger.info("Reset %s channel video playback to start", channel_name_map.at(m_channel_id));
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

    m_video_loaded = true;

    // load a frame to get the video size
    m_video_capture >> m_display_frame;
    m_video_aspect_ratio = static_cast<float>(m_display_frame.cols) / static_cast<float>(m_display_frame.rows);
    m_frames_per_second = m_video_capture.get(cv::CAP_PROP_FPS);
    m_frame_count = m_video_capture.get(cv::CAP_PROP_FRAME_COUNT);
    m_start_frame = 0;
    m_end_frame = m_frame_count - 1;

    m_logger.info("Loaded %s channel video %s with size %d x %d (AR=%.2f), framerate %.2f, and %d frames",
                  channel_name_map.at(m_channel_id), videoPath.c_str(), m_display_frame.cols, m_display_frame.rows, m_video_aspect_ratio,
                  m_frames_per_second, m_frame_count);

    // TODO: set actual video size...
    setVideoSize(800, 800);
    setVideoToStart();

    return VIDEO_LOAD_SUCCESS;
}

void VideoChannel::unloadVideo()
{
    m_video_loaded = false;
}

void VideoChannel::stepFrame(int frame_offset)
{
    if (!m_video_loaded)
    {
        return;
    }

    if (frame_offset == 0)
    {
        return;
    }

    int new_frame = m_active_frame + frame_offset;
    if (new_frame < m_start_frame || new_frame > m_end_frame)
    {
        m_logger.info(
            "Clamping step size to not exceed video bounds");
        new_frame = std::clamp(new_frame, m_start_frame, m_end_frame);
    }

    m_logger.debug("Stepping %s channel frame from %d to %d", channel_name_map.at(m_channel_id), m_active_frame, new_frame);

    // stepping the frame will halt playback if active
    m_continue_playback = false;

    setActiveFrame(new_frame);

    // this will require a video position slider update
    m_update_video_position_callback(m_active_frame, m_channel_id);
}

void VideoChannel::setActiveFrame(int frame_index)
{
    if (!m_video_loaded)
    {
        return;
    }

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
    cv::resize(frame, m_display_frame, cv::Size(m_video_width, m_video_height));

    // we now have the new frame loaded into m_display_frame, so trigger the canvas
    // image update
    updatePixBuf();
}

void VideoChannel::setStartFrame(int frame_index)
{
    if (!m_video_loaded)
    {
        return;
    }

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
    if (!m_video_loaded)
    {
        return;
    }

    if (frame_index == -1)
    {
        m_end_frame = m_frame_count - 1;
    }
    else if (frame_index <= m_start_frame || frame_index >= m_frame_count)
    {
        m_logger.warn("Invalid frame index for end frame: %d, valid range is (%d,%d)", frame_index, m_start_frame + 1,
                      m_frame_count - 1);
        return;
    }
    else
    {
        m_end_frame = frame_index;
    }

    // we will stop playback if the frame is set directly
    m_continue_playback = false;
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

void VideoChannel::setVideoToStart()
{
    if (!m_video_loaded)
    {
        return;
    }

    setActiveFrame(m_start_frame);
}

void VideoChannel::setVideoToFrame(int frame_index)
{
    if (!m_video_loaded)
    {
        return;
    }

    // note that we check the value of frame_index in setActiveFrame
    setActiveFrame(frame_index);
}

void VideoChannel::registerVideoPlaybackCompleteCallback(std::function<void(WhichVideoChannel)> callback)
{
    m_video_complete_callback = callback;
}

} // namespace VideoPlayback