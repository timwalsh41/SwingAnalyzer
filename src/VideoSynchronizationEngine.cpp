#include <VideoSynchronizationEngine.h>
#include <VideoPlaybackConstants.h>

#include <algorithm>

namespace VideoPlayback
{

VideoSynchronizationEngine::VideoSynchronizationEngine() : m_logger(Logger::get())
{
    m_syncManager = std::make_unique<SyncManager>();
    m_logger.info("VideoSynchronizationEngine initialized");
}

void VideoSynchronizationEngine::setObserver(std::weak_ptr<IVideoSynchronizationObserver> observer)
{
    m_observer = observer;
    m_syncManager->setObserver(observer);
    m_logger.debug("Observer set on VideoSynchronizationEngine and forwarded to SyncManager");
}

void VideoSynchronizationEngine::playVideo(WhichVideoChannel channel)
{
    m_logger.debug("VideoSynchronizationEngine::playVideo(%d)", static_cast<int>(channel));
    m_syncManager->handleEvent(VideoSyncEvent::PLAY_OR_PAUSE, channel);
}

void VideoSynchronizationEngine::pauseVideo(WhichVideoChannel channel)
{
    m_logger.debug("VideoSynchronizationEngine::pauseVideo(%d)", static_cast<int>(channel));
    m_syncManager->handleEvent(VideoSyncEvent::PAUSE, channel);
}

void VideoSynchronizationEngine::stepVideo(WhichVideoChannel channel, int frames)
{
    m_logger.debug("VideoSynchronizationEngine::stepVideo(%d, %d)", static_cast<int>(channel), frames);
    
    VideoSyncEvent event;
    if (frames > 0)
    {
        event = (frames > 1) ? VideoSyncEvent::STEP_FORWARD_LARGE : VideoSyncEvent::STEP_FORWARD_SMALL;
    }
    else
    {
        event = (frames < -1) ? VideoSyncEvent::STEP_BACKWARD_LARGE : VideoSyncEvent::STEP_BACKWARD_SMALL;
    }
    
    m_syncManager->handleEvent(event, channel);
}

void VideoSynchronizationEngine::setVideoToStart(WhichVideoChannel channel)
{
    m_logger.debug("VideoSynchronizationEngine::setVideoToStart(%d)", static_cast<int>(channel));
    m_syncManager->handleEvent(VideoSyncEvent::SET_VIDEO_TO_START, channel);
}

void VideoSynchronizationEngine::setVideoPosition(WhichVideoChannel channel, float position)
{
    m_logger.debug("VideoSynchronizationEngine::setVideoPosition(%d, %.3f)", static_cast<int>(channel), position);
    m_syncManager->videoPositionUpdate(position, channel);
}

void VideoSynchronizationEngine::loadVideo(WhichVideoChannel channel, const std::string& path)
{
    m_logger.info("VideoSynchronizationEngine::loadVideo(%d, %s)", static_cast<int>(channel), path.c_str());
    
    // Get the appropriate video channel
    VideoChannel* videoChannel = getVideoChannel(channel);
    if (videoChannel)
    {
        VideoLoadStatus status = videoChannel->loadVideo(path);
        bool success = (status == VIDEO_LOAD_SUCCESS);
        
        if (success)
        {
            m_syncManager->handleEvent(VideoSyncEvent::LOAD_VIDEO, channel);
        }
        
        notifyVideoLoaded(channel, success);
    }
}

void VideoSynchronizationEngine::unloadVideo(WhichVideoChannel channel)
{
    m_logger.info("VideoSynchronizationEngine::unloadVideo(%d)", static_cast<int>(channel));
    
    VideoChannel* videoChannel = getVideoChannel(channel);
    if (videoChannel)
    {
        videoChannel->unloadVideo();
        notifyVideoUnloaded(channel);
    }
}

void VideoSynchronizationEngine::setSyncMode(bool enabled)
{
    m_logger.info("VideoSynchronizationEngine::setSyncMode(%s)", enabled ? "true" : "false");
    
    VideoSyncEvent event = enabled ? VideoSyncEvent::SYNC_ON : VideoSyncEvent::SYNC_OFF;
    m_syncManager->handleEvent(event, WhichVideoChannel::NO_CHANNEL);
    
    notifySyncModeChanged(enabled);
}

void VideoSynchronizationEngine::setPlaybackRate(float multiplier)
{
    m_logger.info("VideoSynchronizationEngine::setPlaybackRate(%.3f)", multiplier);
    
    // Set framerate multiplier for both channels
    if (auto* leftChannel = getVideoChannel(WhichVideoChannel::LEFT_CHANNEL))
    {
        leftChannel->setFramerateMultiplier(multiplier);
    }
    
    if (auto* rightChannel = getVideoChannel(WhichVideoChannel::RIGHT_CHANNEL))
    {
        rightChannel->setFramerateMultiplier(multiplier);
    }
    
    m_syncManager->handleEvent(VideoSyncEvent::PLAYBACK_RATE_UPDATE, WhichVideoChannel::BOTH_CHANNELS);
}

VideoChannel* VideoSynchronizationEngine::getVideoChannel(WhichVideoChannel channel)
{
    switch (channel)
    {
        case WhichVideoChannel::LEFT_CHANNEL:
            return m_syncManager->getLeftChannel();
        case WhichVideoChannel::RIGHT_CHANNEL:
            return m_syncManager->getRightChannel();
        case WhichVideoChannel::BOTH_CHANNELS:
        case WhichVideoChannel::NO_CHANNEL:
        default:
            return nullptr;
    }
}

void VideoSynchronizationEngine::start()
{
    m_logger.info("Starting VideoSynchronizationEngine");
    m_syncManager->start();
}

void VideoSynchronizationEngine::notifyVideoPositionChanged(WhichVideoChannel channel, float position)
{
    if (auto observer = m_observer.lock())
    {
        observer->onVideoPositionChanged(channel, position);
    }
}

void VideoSynchronizationEngine::notifyPlaybackStateChanged(WhichVideoChannel channel, VideoPlaybackStatus status)
{
    if (auto observer = m_observer.lock())
    {
        observer->onPlaybackStateChanged(channel, status);
    }
}

void VideoSynchronizationEngine::notifySyncModeChanged(bool syncMode)
{
    if (auto observer = m_observer.lock())
    {
        observer->onSyncModeChanged(syncMode);
    }
}

void VideoSynchronizationEngine::notifyVideoLoaded(WhichVideoChannel channel, bool success)
{
    if (auto observer = m_observer.lock())
    {
        observer->onVideoLoaded(channel, success);
    }
}

void VideoSynchronizationEngine::notifyVideoUnloaded(WhichVideoChannel channel)
{
    if (auto observer = m_observer.lock())
    {
        observer->onVideoUnloaded(channel);
    }
}

} // namespace VideoPlayback
