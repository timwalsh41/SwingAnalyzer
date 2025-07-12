#include <VideoPlaybackController.h>

namespace VideoPlayback
{

VideoPlaybackController::VideoPlaybackController() 
    : m_should_continue(false), m_frame_time_us(0), m_logger(Logger::get())
{
}

VideoPlaybackController::~VideoPlaybackController()
{
    stopPlayback();
}

void VideoPlaybackController::startPlayback(std::function<void()> playback_function)
{
    if (isPlaying())
    {
        m_logger.warn("Playback already active, stopping previous playback");
        stopPlayback();
    }

    m_should_continue.store(true);
    m_playback_thread = std::thread([this, playback_function]()
    {
        m_logger.debug("Playback thread started");
        playback_function();
        m_logger.debug("Playback thread finished");
    });
}

void VideoPlaybackController::stopPlayback()
{
    m_should_continue.store(false);
    cleanup();
}

bool VideoPlaybackController::isPlaying() const
{
    return m_should_continue.load() && m_playback_thread.joinable();
}

void VideoPlaybackController::setFramerate(float fps, float multiplier)
{
    if (fps <= 0 || multiplier <= 0)
    {
        m_logger.warn("Invalid framerate parameters: fps=%.2f, multiplier=%.2f", fps, multiplier);
        return;
    }

    if (multiplier > MAX_FRAMERATE_MULTIPLIER || multiplier < MIN_FRAMERATE_MULTIPLIER)
    {
        m_logger.warn("Framerate multiplier %.3f out of range (%.3f-%.3f)", 
                     multiplier, MIN_FRAMERATE_MULTIPLIER, MAX_FRAMERATE_MULTIPLIER);
        return;
    }

    m_frame_time_us = 1.0f / multiplier / fps * 1e6f;
    m_logger.debug("Frame time set to %.2f ms (fps=%.2f, multiplier=%.2f)", 
                  m_frame_time_us / 1000.0f, fps, multiplier);
}

std::chrono::microseconds VideoPlaybackController::getFrameInterval() const
{
    return std::chrono::microseconds(static_cast<long int>(m_frame_time_us));
}

void VideoPlaybackController::cleanup()
{
    if (m_playback_thread.joinable())
    {
        m_playback_thread.join();
    }
}

} // namespace VideoPlayback
