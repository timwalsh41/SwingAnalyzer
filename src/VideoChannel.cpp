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
    : channel_id_(channel)
    , position_callback_(update_callback)
    , current_frame_(0)
    , framerate_multiplier_(1.0f)
    , video_width_(100)
    , video_height_(100)
    , profiler_frame_(std::string(channel_name_map.at(channel_id_)) + "VideoChannelFrameProfiler")
    , profiler_image_(std::string(channel_name_map.at(channel_id_)) + "VideoChannelImageProfiler")
    , logger_(Logger::get()) {
    
    // Initialize components
    state_machine_ = std::make_unique<VideoChannelStateMachine>();
    video_decoder_ = std::make_unique<OpenCVVideoDecoder>();
    gui_updater_ = std::make_unique<GTKImageUpdater>();
    frame_manager_ = std::make_unique<VideoFrameManager>();
    playback_controller_ = std::make_unique<VideoPlaybackController>();
    frame_buffer_ = std::make_unique<ThreadSafeFrameBuffer>();
    
    // Setup dispatchers
    frame_update_dispatcher_.connect([this]() { onFrameUpdate(); });
    playback_complete_dispatcher_.connect([this]() { onPlaybackComplete(); });
    
    // Reset profilers
    profiler_frame_.reset();
    profiler_image_.reset(true);
    
    logger_.info("VideoChannel %s initialized", channel_name_map.at(channel_id_));
}

VideoChannel::~VideoChannel() {
    if (isPlaying()) {
        pauseVideo();
    }
    logger_.debug("VideoChannel %s destroyed", channel_name_map.at(channel_id_));
}

bool VideoChannel::isLoaded() const {
    return state_machine_->isLoaded();
}

bool VideoChannel::isPlaying() const {
    return state_machine_->isPlaying();
}

VideoChannelState VideoChannel::getState() const {
    return state_machine_->getCurrentState();
}

VideoLoadStatus VideoChannel::loadVideo(const std::string& path) {
    if (!transitionState(VideoChannelEvent::LOAD_VIDEO)) {
        logger_.warn("Cannot load video in current state: %s", state_machine_->getStateName());
        return VIDEO_LOAD_UNKNOWN_ERROR;
    }
    
    VideoLoadStatus status = video_decoder_->loadVideo(path);
    if (status != VIDEO_LOAD_SUCCESS) {
        // Revert state transition
        state_machine_->transition(VideoChannelEvent::ERROR_OCCURRED);
        return status;
    }
    
    // Setup frame management
    VideoProperties props = video_decoder_->getProperties();
    frame_manager_->setTotalFrames(props.frame_count);
    
    // Update playback controller
    playback_controller_->setFramerate(props.fps, framerate_multiplier_);
    
    // Update video size
    updateVideoSize();
    
    // Set to start frame
    current_frame_ = frame_manager_->getStartFrame();
    
    logger_.info("Loaded %s channel video: %dx%d, %.2f fps, %d frames", 
                 channel_name_map.at(channel_id_), props.width, props.height, 
                 props.fps, props.frame_count);
    
    setVideoToStart();
    return VIDEO_LOAD_SUCCESS;
}

void VideoChannel::unloadVideo() {
    if (isPlaying()) {
        pauseVideo();
    }
    
    if (transitionState(VideoChannelEvent::UNLOAD_VIDEO)) {
        video_decoder_->unloadVideo();
        current_frame_ = 0;
        logger_.info("Unloaded %s channel video", channel_name_map.at(channel_id_));
    }
}

void VideoChannel::playVideo() {
    if (!transitionState(VideoChannelEvent::PLAY)) {
        logger_.warn("Cannot play video in current state: %s", state_machine_->getStateName());
        return;
    }
    
    // Reset to start if at end
    if (current_frame_ >= frame_manager_->getEndFrame()) {
        current_frame_ = frame_manager_->getStartFrame();
        video_decoder_->seekToFrame(current_frame_);
        logger_.info("Reset %s channel video playback to start", channel_name_map.at(channel_id_));
    }
    
    // Start playback
    playback_controller_->startPlayback([this]() { playbackWorkerFunction(); });
    
    logger_.info("Started playback of %s channel video", channel_name_map.at(channel_id_));
}

void VideoChannel::pauseVideo() {
    if (transitionState(VideoChannelEvent::PAUSE)) {
        playback_controller_->stopPlayback();
        logger_.info("Paused %s channel video", channel_name_map.at(channel_id_));
    }
}

void VideoChannel::stepFrame(int offset) {
    if (!state_machine_->canStep()) {
        logger_.warn("Cannot step frame in current state: %s", state_machine_->getStateName());
        return;
    }
    
    if (offset == 0) return;
    
    // Stop playback if active
    if (isPlaying()) {
        transitionState(VideoChannelEvent::STEP_FRAME);
        playback_controller_->stopPlayback();
    }
    
    int new_frame = frame_manager_->stepFrame(current_frame_, offset);
    
    logger_.debug("Stepping %s channel frame from %d to %d", 
                  channel_name_map.at(channel_id_), current_frame_, new_frame);
    
    seekToFrame(new_frame);
}

void VideoChannel::seekToFrame(int frame) {
    if (!state_machine_->canSeek()) {
        logger_.warn("Cannot seek in current state: %s", state_machine_->getStateName());
        return;
    }
    
    if (!frame_manager_->isValidFrame(frame)) {
        logger_.warn("Invalid frame index: %d (valid range: %d-%d)", 
                     frame, frame_manager_->getStartFrame(), frame_manager_->getEndFrame());
        return;
    }
    
    // Stop playback if active
    if (isPlaying()) {
        transitionState(VideoChannelEvent::SEEK_FRAME);
        playback_controller_->stopPlayback();
    }
    
    current_frame_ = frame;
    
    // Load and display the frame
    if (video_decoder_->seekToFrame(current_frame_)) {
        cv::Mat frame_data;
        if (video_decoder_->getCurrentFrame(frame_data)) {
            frame_buffer_->updateFrame(frame_data);
            cv::Mat display_frame = frame_buffer_->getDisplayFrame(cv::Size(video_width_, video_height_));
            if (!display_frame.empty()) {
                gui_updater_->updateImage(display_frame);
            }
        }
    }
    
    // Notify position update
    if (position_callback_) {
        position_callback_(current_frame_, channel_id_);
    }
}

void VideoChannel::setVideoToStart() {
    if (isLoaded()) {
        seekToFrame(frame_manager_->getStartFrame());
    }
}

void VideoChannel::setStartFrame(int start) {
    if (!isLoaded()) return;
    
    if (isPlaying()) {
        transitionState(VideoChannelEvent::PAUSE);
        playback_controller_->stopPlayback();
    }
    
    frame_manager_->setStartFrame(start);
}

void VideoChannel::setEndFrame(int end) {
    if (!isLoaded()) return;
    
    if (isPlaying()) {
        transitionState(VideoChannelEvent::PAUSE);
        playback_controller_->stopPlayback();
    }
    
    frame_manager_->setEndFrame(end);
}

int VideoChannel::getStartFrame() const {
    return isLoaded() ? frame_manager_->getStartFrame() : 0;
}

int VideoChannel::getEndFrame() const {
    return isLoaded() ? frame_manager_->getEndFrame() : 0;
}

int VideoChannel::getEffectiveLength() const {
    return isLoaded() ? frame_manager_->getEffectiveLength() : 0;
}

int VideoChannel::getCurrentFrame() const {
    return isLoaded() ? current_frame_ : 0;
}

int VideoChannel::getTotalFrames() const {
    return isLoaded() ? frame_manager_->getTotalFrames() : 0;
}

void VideoChannel::setFramerateMultiplier(float multiplier) {
    if (multiplier > MAX_FRAMERATE_MULTIPLIER || multiplier < MIN_FRAMERATE_MULTIPLIER) {
        logger_.warn("Invalid framerate multiplier %.3f, valid range is (%.3f,%.3f)", 
                     multiplier, MIN_FRAMERATE_MULTIPLIER, MAX_FRAMERATE_MULTIPLIER);
        return;
    }
    
    // Stop playback if active
    if (isPlaying()) {
        pauseVideo();
    }
    
    framerate_multiplier_ = multiplier;
    
    if (isLoaded()) {
        VideoProperties props = video_decoder_->getProperties();
        playback_controller_->setFramerate(props.fps, framerate_multiplier_);
    }
}

float VideoChannel::getFramerateMultiplier() const {
    return isLoaded() ? framerate_multiplier_ : 0.0f;
}

float VideoChannel::getFramerate() const {
    if (!isLoaded()) return 0.0f;
    
    VideoProperties props = video_decoder_->getProperties();
    return props.fps;
}

VideoSize VideoChannel::getVideoSize() const {
    if (!isLoaded()) {
        return {0, 0, 0.0f};
    }
    
    VideoProperties props = video_decoder_->getProperties();
    return {video_width_, video_height_, props.aspect_ratio};
}

void VideoChannel::setVideoSize(int width, int height) {
    if (!isLoaded()) return;
    
    if (width < 10 || height < 10) {
        logger_.error("Invalid video size: %dx%d", width, height);
        return;
    }
    
    VideoProperties props = video_decoder_->getProperties();
    
    // Maintain aspect ratio
    if (static_cast<float>(width) / props.aspect_ratio <= height) {
        video_width_ = width;
        video_height_ = static_cast<int>(static_cast<float>(width) / props.aspect_ratio);
    } else {
        video_height_ = height;
        video_width_ = static_cast<int>(static_cast<float>(height) * props.aspect_ratio);
    }
    
    gui_updater_->setImageSize(video_width_, video_height_);
    
    logger_.info("Set %s channel video size to %dx%d", 
                 channel_name_map.at(channel_id_), video_width_, video_height_);
}

void* VideoChannel::getImageObject() {
    return gui_updater_->getImageObject();
}

void VideoChannel::registerPlaybackCompleteCallback(std::function<void(WhichVideoChannel)> callback) {
    complete_callback_ = callback;
}

// Compatibility methods with old VideoChannel API
const bool VideoChannel::IsVideoLoaded() const {
    return isLoaded();
}

const int VideoChannel::getActiveFrame() const {
    return getCurrentFrame();
}

const int VideoChannel::getFrameCount() const {
    return getTotalFrames();
}

const float VideoChannel::getRawFramerate_fps() const {
    return getFramerate();
}

const VideoPlaybackStatus VideoChannel::getPlaybackStatus() const {
    return isPlaying() ? PLAYING : PAUSED;
}

const int VideoChannel::getEffectiveVideoLengthFrames() const {
    return getEffectiveLength();
}

void VideoChannel::setVideoToFrame(int frame_index) {
    seekToFrame(frame_index);
}

void VideoChannel::registerVideoPlaybackCompleteCallback(std::function<void(WhichVideoChannel)> callback) {
    registerPlaybackCompleteCallback(callback);
}

const Glib::RefPtr<Goocanvas::Image>& VideoChannel::GetImage() {
    // Return the Goocanvas::Image from the GTK updater
    GTKImageUpdater* gtk_updater = static_cast<GTKImageUpdater*>(gui_updater_.get());
    return gtk_updater->getImage();
}

void VideoChannel::enableProfiler() {
    profiler_frame_.enableProfiler();
    profiler_image_.enableProfiler();
}

void VideoChannel::disableProfiler() {
    profiler_frame_.disableProfiler();
    profiler_image_.disableProfiler();
}

// Private methods

void VideoChannel::playbackWorkerFunction() {
    profiler_frame_.tick();
    
    logger_.info("Starting playback of %s channel video with frame time %.2f ms", 
                 channel_name_map.at(channel_id_), 
                 playback_controller_->getFrameInterval().count() / 1000.0f);
    
    cv::Mat frame;
    while (current_frame_ < frame_manager_->getEndFrame() && playback_controller_->isPlaying()) {
        profiler_frame_.tick();
        
        // Get next frame
        if (video_decoder_->getNextFrame(frame)) {
            if (!frame.empty()) {
                frame_buffer_->updateFrame(frame);
                frame_update_dispatcher_.emit();
            } else {
                logger_.warn("Frame %d is empty for %s channel", 
                           current_frame_, channel_name_map.at(channel_id_));
            }
        }
        
        current_frame_++;
        profiler_frame_.tock();
        
        // Wait for next frame time
        std::this_thread::sleep_until(
            profiler_frame_.getTickTime() + playback_controller_->getFrameInterval()
        );
    }
    
    profiler_frame_.dump();
    profiler_image_.dump();
    profiler_frame_.reset();
    profiler_image_.reset(true);
    
    playback_complete_dispatcher_.emit();
    logger_.info("Channel %s finished playback", channel_name_map.at(channel_id_));
}

void VideoChannel::onFrameUpdate() {
    profiler_image_.tick();
    
    cv::Mat display_frame = frame_buffer_->getDisplayFrame(cv::Size(video_width_, video_height_));
    if (!display_frame.empty()) {
        gui_updater_->updateImage(display_frame);
    }
    
    // Update position
    if (position_callback_) {
        position_callback_(current_frame_, channel_id_);
    }
    
    profiler_image_.tock();
}

void VideoChannel::onPlaybackComplete() {
    transitionState(VideoChannelEvent::PAUSE);
    
    if (complete_callback_) {
        complete_callback_(channel_id_);
    }
}

bool VideoChannel::transitionState(VideoChannelEvent event) {
    bool success = state_machine_->transition(event);
    if (!success) {
        logger_.warn("Invalid state transition: %s in state %s", 
                     VideoChannelStateMachine::getEventName(event), state_machine_->getStateName());
    }
    return success;
}

void VideoChannel::updateVideoSize() {
    if (!isLoaded()) return;
    
    VideoProperties props = video_decoder_->getProperties();
    setVideoSize(800, 800); // Default size, can be adjusted later
}

} // namespace VideoPlayback
