#include <AnalyzerController.h>
#include <AnalyzerGui.h>
#include <VideoPlaybackConstants.h>

AnalyzerController::AnalyzerController(AnalyzerGui* gui) 
    : m_gui(gui), m_logger(Logger::get())
{
    m_logger.info("AnalyzerController constructed");
}

void AnalyzerController::initialize()
{
    m_engine = std::make_shared<VideoPlayback::VideoSynchronizationEngine>();
    m_engine->addObserver(std::weak_ptr<VideoPlayback::IVideoSynchronizationObserver>(shared_from_this()));
    m_engine->start();

    // TODO: TEMPORARY TEST CODE TO LOAD AND PLAY VIDEO CHANNELS
    m_engine->loadVideo(VideoPlayback::WhichVideoChannel::LEFT_CHANNEL, "/home/tim/Downloads/Golf/2025-05-27 4i DTL.mp4");
    m_engine->loadVideo(VideoPlayback::WhichVideoChannel::RIGHT_CHANNEL, "/home/tim/Downloads/Golf/2025-05-27 4i FO.mp4");
    
    m_logger.info("AnalyzerController initialized");
}

void AnalyzerController::playVideo(VideoPlayback::WhichVideoChannel channel)
{
    m_logger.debug("AnalyzerController::playVideo(%d)", static_cast<int>(channel));
    m_engine->playVideo(channel);
}

void AnalyzerController::pauseVideo(VideoPlayback::WhichVideoChannel channel)
{
    m_logger.debug("AnalyzerController::pauseVideo(%d)", static_cast<int>(channel));
    m_engine->pauseVideo(channel);
}

void AnalyzerController::stepVideo(VideoPlayback::WhichVideoChannel channel, int frames)
{
    m_logger.debug("AnalyzerController::stepVideo(%d, %d)", static_cast<int>(channel), frames);
    m_engine->stepVideo(channel, frames);
}

void AnalyzerController::setVideoToStart(VideoPlayback::WhichVideoChannel channel)
{
    m_logger.debug("AnalyzerController::setVideoToStart(%d)", static_cast<int>(channel));
    m_engine->setVideoToStart(channel);
}

void AnalyzerController::setVideoPosition(VideoPlayback::WhichVideoChannel channel, float position)
{
    m_logger.debug("AnalyzerController::setVideoPosition(%d, %.3f)", static_cast<int>(channel), position);
    m_engine->setVideoPosition(channel, position);
}

void AnalyzerController::loadVideo(VideoPlayback::WhichVideoChannel channel, const std::string& path)
{
    m_logger.info("AnalyzerController::loadVideo(%d, %s)", static_cast<int>(channel), path.c_str());
    m_engine->loadVideo(channel, path);
}

void AnalyzerController::unloadVideo(VideoPlayback::WhichVideoChannel channel)
{
    m_logger.info("AnalyzerController::unloadVideo(%d)", static_cast<int>(channel));
    m_engine->unloadVideo(channel);
}

void AnalyzerController::setSyncMode(bool enabled)
{
    m_logger.info("AnalyzerController::setSyncMode(%s)", enabled ? "true" : "false");
    m_engine->setSyncMode(enabled);
}

void AnalyzerController::setPlaybackRate(float multiplier)
{
    m_logger.info("AnalyzerController::setPlaybackRate(%.3f)", multiplier);
    m_engine->setPlaybackRate(multiplier);
}

void AnalyzerController::onVideoPositionChanged(VideoPlayback::WhichVideoChannel channel, float position)
{
    m_logger.debug("AnalyzerController::onVideoPositionChanged(%d, %.3f)", static_cast<int>(channel), position);
    m_gui->updateVideoPosition(channel, position);
}

void AnalyzerController::onPlaybackStateChanged(VideoPlayback::WhichVideoChannel channel, VideoPlayback::VideoPlaybackStatus status)
{
    m_logger.debug("AnalyzerController::onPlaybackStateChanged(%d, %d)", static_cast<int>(channel), static_cast<int>(status));
    m_gui->updatePlaybackState(channel, status);
}

void AnalyzerController::onSyncModeChanged(bool syncMode)
{
    m_logger.info("AnalyzerController::onSyncModeChanged(%s)", syncMode ? "true" : "false");
    m_gui->updateSyncModeControls(syncMode);
}

void AnalyzerController::onVideoLoaded(VideoPlayback::WhichVideoChannel channel, bool success)
{
    m_logger.info("AnalyzerController::onVideoLoaded(%d, %s)", static_cast<int>(channel), success ? "success" : "failed");
    m_gui->updateVideoLoadedState(channel, success);

    // If video loading was successful, set up the video renderer to connect the video to the GUI canvas
    if (success)
    {
        VideoPlayback::VideoChannel* videoChannel = m_engine->getVideoChannel(channel);
        if (videoChannel)
        {
            m_gui->setVideoRenderer(channel, videoChannel);
            m_logger.info("Video renderer set up for channel %d", static_cast<int>(channel));
        }
        else
        {
            m_logger.error("Failed to get video channel for renderer setup");
        }
    }
}

void AnalyzerController::onVideoUnloaded(VideoPlayback::WhichVideoChannel channel)
{
    m_logger.info("AnalyzerController::onVideoUnloaded(%d)", static_cast<int>(channel));
    m_gui->updateVideoUnloadedState(channel);
}

VideoPlayback::VideoChannel* AnalyzerController::getVideoChannel(VideoPlayback::WhichVideoChannel channel)
{
    return m_engine->getVideoChannel(channel);
}
