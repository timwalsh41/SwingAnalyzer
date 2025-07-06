#include <VideoChannel.h>
#include <GTKImageUpdater.h>
#include <OpenCVVideoDecoder.h>
#include <VideoPlaybackConstants.h>

#include <algorithm>
#include <thread>
#include <chrono>

namespace VideoPlayback
{

VideoChannel::VideoChannel(WhichVideoChannel channel, 
                                 std::function<void(int, WhichVideoChannel)> update_callback)
    : m_channel_id(channel)
    , m_position_callback(update_callback)
    , m_current_frame(0)
    , m_framerate_multiplier(1.0f)
    , m_video_width(100)
    , m_video_height(100)
    , m_profiler_frame(std::string(channel_name_map.at(m_channel_id)) + "VideoChannelFrameProfiler")
    , m_profiler_image(std::string(channel_name_map.at(m_channel_id)) + "VideoChannelImageProfiler")
    , m_logger(Logger::get()) 
{    
    // Initialize components
    m_state_machine = std::make_unique<VideoChannelStateMachine>();
    m_video_decoder = std::make_unique<OpenCVVideoDecoder>();
    m_gui_updater = std::make_unique<GTKImageUpdater>();
    m_frame_manager = std::make_unique<VideoFrameManager>();
    m_playback_controller = std::make_unique<VideoPlaybackController>();
    m_frame_buffer = std::make_unique<ThreadSafeFrameBuffer>();
    
    // Setup dispatchers
    m_frame_update_dispatcher.connect([this]() { onFrameUpdate(); });
    m_playback_complete_dispatcher.connect([this]() { onPlaybackComplete(); });
    
    // Reset profilers
    m_profiler_frame.reset();
    m_profiler_image.reset(true);
    
    m_logger.info("VideoChannel %s initialized", channel_name_map.at(m_channel_id));
}

VideoChannel::~VideoChannel()
{
    if (isPlaying())
    {
        pauseVideo();
    }
    m_logger.debug("VideoChannel %s destroyed", channel_name_map.at(m_channel_id));
}

bool VideoChannel::isLoaded() const
{
    return m_state_machine->isLoaded();
}

bool VideoChannel::isPlaying() const
{
    return m_state_machine->isPlaying();
}

VideoChannelState VideoChannel::getState() const
{
    return m_state_machine->getCurrentState();
}

VideoLoadStatus VideoChannel::loadVideo(const std::string& path)
{
    if (!transitionState(VideoChannelEvent::LOAD_VIDEO))
    {
        m_logger.warn("Cannot load video in current state: %s", m_state_machine->getStateName());
        return VIDEO_LOAD_ERROR;
    }
    
    VideoLoadStatus status = m_video_decoder->loadVideo(path);
    if (status != VIDEO_LOAD_SUCCESS)
    {
        // Revert state transition
        m_state_machine->transition(VideoChannelEvent::ERROR_OCCURRED);
        return status;
    }
    
    // Setup frame management
    VideoProperties props = m_video_decoder->getProperties();
    m_frame_manager->setTotalFrames(props.frame_count);
    
    // Update playback controller
    m_playback_controller->setFramerate(props.fps, m_framerate_multiplier);
    
    // Update video size
    updateVideoSize();
    
    // Set to start frame
    m_current_frame = m_frame_manager->getStartFrame();
    
    m_logger.info("Loaded %s channel video: %dx%d, %.2f fps, %d frames", 
                 channel_name_map.at(m_channel_id), props.width, props.height, 
                 props.fps, props.frame_count);
    
    setVideoToStart();
    return VIDEO_LOAD_SUCCESS;
}

void VideoChannel::unloadVideo()
{
    if (isPlaying())
    {
        pauseVideo();
    }
    
    if (transitionState(VideoChannelEvent::UNLOAD_VIDEO))
    {
        m_video_decoder->unloadVideo();
        m_current_frame = 0;
        m_logger.info("Unloaded %s channel video", channel_name_map.at(m_channel_id));
    }
}

void VideoChannel::playVideo()
{
    if (!transitionState(VideoChannelEvent::PLAY))
    {
        m_logger.warn("Cannot play video in current state: %s", m_state_machine->getStateName());
        return;
    }
    
    // Reset to start if at end
    if (m_current_frame >= m_frame_manager->getEndFrame())
    {
        m_current_frame = m_frame_manager->getStartFrame();
        m_video_decoder->seekToFrame(m_current_frame);
        m_logger.info("Reset %s channel video playback to start", channel_name_map.at(m_channel_id));
    }
    
    // Start playback
    m_playback_controller->startPlayback([this]() { playbackWorkerFunction(); });
    
    m_logger.info("Started playback of %s channel video", channel_name_map.at(m_channel_id));
}

void VideoChannel::pauseVideo()
{
    if (transitionState(VideoChannelEvent::PAUSE))
    {
        m_playback_controller->stopPlayback();
        m_logger.info("Paused %s channel video", channel_name_map.at(m_channel_id));
    }
}

void VideoChannel::stepFrame(int offset)
{
    if (!m_state_machine->canStep())
    {
        m_logger.warn("Cannot step frame in current state: %s", m_state_machine->getStateName());
        return;
    }
    
    if (offset == 0) return;
    
    // Stop playback if active
    if (isPlaying())
    {
        transitionState(VideoChannelEvent::STEP_FRAME);
        m_playback_controller->stopPlayback();
    }
    
    int new_frame = m_frame_manager->stepFrame(m_current_frame, offset);
    
    m_logger.debug("Stepping %s channel frame from %d to %d", 
                  channel_name_map.at(m_channel_id), m_current_frame, new_frame);
    
    seekToFrame(new_frame);
}

void VideoChannel::seekToFrame(int frame)
{
    if (!m_state_machine->canSeek())
    {
        m_logger.warn("Cannot seek in current state: %s", m_state_machine->getStateName());
        return;
    }
    
    if (!m_frame_manager->isValidFrame(frame))
    {
        m_logger.warn("Invalid frame index: %d (valid range: %d-%d)", 
                     frame, m_frame_manager->getStartFrame(), m_frame_manager->getEndFrame());
        return;
    }
    
    // Stop playback if active
    if (isPlaying())
    {
        transitionState(VideoChannelEvent::SEEK_FRAME);
        m_playback_controller->stopPlayback();
    }
    
    m_current_frame = frame;
    
    // Load and display the frame
    if (m_video_decoder->seekToFrame(m_current_frame))
    {
        cv::Mat frame_data;
        if (m_video_decoder->getCurrentFrame(frame_data))
        {
            m_frame_buffer->updateFrame(frame_data);
            cv::Mat display_frame = m_frame_buffer->getDisplayFrame(cv::Size(m_video_width, m_video_height));
            if (!display_frame.empty())
            {
                m_gui_updater->updateImage(display_frame);
            }
        }
    }
    
    // Notify position update
    if (m_position_callback)
    {
        m_position_callback(m_current_frame, m_channel_id);
    }
}

void VideoChannel::setVideoToStart()
{
    if (isLoaded())
    {
        seekToFrame(m_frame_manager->getStartFrame());
    }
}

void VideoChannel::setStartFrame(int start)
{
    if (!isLoaded()) return;
    
    if (isPlaying())
    {
        transitionState(VideoChannelEvent::PAUSE);
        m_playback_controller->stopPlayback();
    }
    
    m_frame_manager->setStartFrame(start);
}

void VideoChannel::setEndFrame(int end)
{
    if (!isLoaded()) return;
    
    if (isPlaying())
    {
        transitionState(VideoChannelEvent::PAUSE);
        m_playback_controller->stopPlayback();
    }
    
    m_frame_manager->setEndFrame(end);
}

int VideoChannel::getStartFrame() const
{
    return isLoaded() ? m_frame_manager->getStartFrame() : 0;
}

int VideoChannel::getEndFrame() const
{
    return isLoaded() ? m_frame_manager->getEndFrame() : 0;
}

int VideoChannel::getEffectiveLength() const
{
    return isLoaded() ? m_frame_manager->getEffectiveLength() : 0;
}

int VideoChannel::getCurrentFrame() const
{
    return isLoaded() ? m_current_frame : 0;
}

int VideoChannel::getTotalFrames() const
{
    return isLoaded() ? m_frame_manager->getTotalFrames() : 0;
}

void VideoChannel::setFramerateMultiplier(float multiplier)
{
    if (multiplier > MAX_FRAMERATE_MULTIPLIER || multiplier < MIN_FRAMERATE_MULTIPLIER)
    {
        m_logger.warn("Invalid framerate multiplier %.3f, valid range is (%.3f,%.3f)", 
                     multiplier, MIN_FRAMERATE_MULTIPLIER, MAX_FRAMERATE_MULTIPLIER);
        return;
    }
    
    // Stop playback if active
    if (isPlaying())
    {
        pauseVideo();
    }
    
    m_framerate_multiplier = multiplier;
    
    if (isLoaded())
    {
        VideoProperties props = m_video_decoder->getProperties();
        m_playback_controller->setFramerate(props.fps, m_framerate_multiplier);
    }
}

float VideoChannel::getFramerateMultiplier() const
{
    return isLoaded() ? m_framerate_multiplier : 0.0f;
}

float VideoChannel::getFramerate() const
{
    if (!isLoaded()) return 0.0f;
    
    VideoProperties props = m_video_decoder->getProperties();
    return props.fps;
}

VideoSize VideoChannel::getVideoSize() const
{
    if (!isLoaded())
    {
        return {0, 0, 0.0f};
    }
    
    VideoProperties props = m_video_decoder->getProperties();
    return {m_video_width, m_video_height, props.aspect_ratio};
}

void VideoChannel::setVideoSize(int width, int height)
{
    if (!isLoaded()) return;
    
    if (width < 10 || height < 10)
    {
        m_logger.error("Invalid video size: %dx%d", width, height);
        return;
    }
    
    VideoProperties props = m_video_decoder->getProperties();
    
    // Maintain aspect ratio
    if (static_cast<float>(width) / props.aspect_ratio <= height)
    {
        m_video_width = width;
        m_video_height = static_cast<int>(static_cast<float>(width) / props.aspect_ratio);
    }
    else
    {
        m_video_height = height;
        m_video_width = static_cast<int>(static_cast<float>(height) * props.aspect_ratio);
    }
    
    m_gui_updater->setImageSize(m_video_width, m_video_height);
    
    m_logger.info("Set %s channel video size to %dx%d", 
                 channel_name_map.at(m_channel_id), m_video_width, m_video_height);
}

void* VideoChannel::getImageObject()
{
    return m_gui_updater->getImageObject();
}

void VideoChannel::registerPlaybackCompleteCallback(std::function<void(WhichVideoChannel)> callback)
{
    m_complete_callback = callback;
}

// Compatibility methods with old VideoChannel API
const bool VideoChannel::IsVideoLoaded() const
{
    return isLoaded();
}

const int VideoChannel::getActiveFrame() const
{
    return getCurrentFrame();
}

const int VideoChannel::getFrameCount() const
{
    return getTotalFrames();
}

const float VideoChannel::getRawFramerate_fps() const
{
    return getFramerate();
}

const VideoPlaybackStatus VideoChannel::getPlaybackStatus() const
{
    return isPlaying() ? PLAYING : PAUSED;
}

const int VideoChannel::getEffectiveVideoLengthFrames() const
{
    return getEffectiveLength();
}

void VideoChannel::registerVideoPlaybackCompleteCallback(std::function<void(WhichVideoChannel)> callback)
{
    registerPlaybackCompleteCallback(callback);
}

const Glib::RefPtr<Goocanvas::Image>& VideoChannel::GetImage()
{
    // Return the Goocanvas::Image from the GTK updater
    GTKImageUpdater* gtk_updater = static_cast<GTKImageUpdater*>(m_gui_updater.get());
    return gtk_updater->getImage();
}

void VideoChannel::enableProfiler()
{
    m_profiler_frame.enableProfiler();
    m_profiler_image.enableProfiler();
}

void VideoChannel::disableProfiler()
{
    m_profiler_frame.disableProfiler();
    m_profiler_image.disableProfiler();
}

// Private methods

void VideoChannel::playbackWorkerFunction()
{
    m_profiler_frame.tick();
    
    m_logger.info("Starting playback of %s channel video with frame time %.2f ms", 
                 channel_name_map.at(m_channel_id), 
                 m_playback_controller->getFrameInterval().count() / 1000.0f);
    
    cv::Mat frame;
    while (m_current_frame < m_frame_manager->getEndFrame() && m_playback_controller->isPlaying())
    {
        m_profiler_frame.tick();
        
        // Get next frame
        if (m_video_decoder->getNextFrame(frame))
        {
            if (!frame.empty())
            {
                m_frame_buffer->updateFrame(frame);
                m_frame_update_dispatcher.emit();
            }
            else
            {
                m_logger.warn("Frame %d is empty for %s channel", 
                           m_current_frame, channel_name_map.at(m_channel_id));
            }
        }
        
        m_current_frame++;
        m_profiler_frame.tock();
        
        // Wait for next frame time
        std::this_thread::sleep_until(
            m_profiler_frame.getTickTime() + m_playback_controller->getFrameInterval()
        );
    }
    
    m_profiler_frame.dump();
    m_profiler_image.dump();
    m_profiler_frame.reset();
    m_profiler_image.reset(true);
    
    m_playback_complete_dispatcher.emit();
    m_logger.info("Channel %s finished playback", channel_name_map.at(m_channel_id));
}

void VideoChannel::onFrameUpdate()
{
    m_profiler_image.tick();
    
    cv::Mat display_frame = m_frame_buffer->getDisplayFrame(cv::Size(m_video_width, m_video_height));
    if (!display_frame.empty())
    {
        m_gui_updater->updateImage(display_frame);
    }
    
    // Update position
    if (m_position_callback)
    {
        m_position_callback(m_current_frame, m_channel_id);
    }
    
    m_profiler_image.tock();
}

void VideoChannel::onPlaybackComplete()
{
    transitionState(VideoChannelEvent::PAUSE);
    
    if (m_complete_callback)
    {
        m_complete_callback(m_channel_id);
    }
}

bool VideoChannel::transitionState(VideoChannelEvent event)
{
    bool success = m_state_machine->transition(event);
    if (!success)
    {
        m_logger.warn("Invalid state transition: %s in state %s", 
                     VideoChannelStateMachine::getEventName(event), m_state_machine->getStateName());
    }
    return success;
}

void VideoChannel::updateVideoSize()
{
    if (!isLoaded()) return;
    
    VideoProperties props = m_video_decoder->getProperties();
    setVideoSize(800, 800); // Default size, can be adjusted later
}

} // namespace VideoPlayback
