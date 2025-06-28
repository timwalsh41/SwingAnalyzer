#ifndef VIDEOCHANNELSTATE_H
#define VIDEOCHANNELSTATE_H

namespace VideoPlayback
{

enum class VideoChannelState {
    UNLOADED,       // No video file loaded
    STOPPED,        // Video loaded but not playing
    PLAYING         // Video loaded and actively playing
};

enum class VideoChannelEvent {
    LOAD_VIDEO,
    UNLOAD_VIDEO,
    PLAY,
    PAUSE,
    STEP_FRAME,
    SEEK_FRAME,
    ERROR_OCCURRED
};

class VideoChannelStateMachine {
public:
    VideoChannelStateMachine() : current_state_(VideoChannelState::UNLOADED) {}

    // State queries
    bool canPlay() const { return current_state_ == VideoChannelState::STOPPED; }
    bool canPause() const { return current_state_ == VideoChannelState::PLAYING; }
    bool canSeek() const { return current_state_ != VideoChannelState::UNLOADED; }
    bool canStep() const { return current_state_ != VideoChannelState::UNLOADED; }
    bool isLoaded() const { return current_state_ != VideoChannelState::UNLOADED; }
    bool isPlaying() const { return current_state_ == VideoChannelState::PLAYING; }
    
    VideoChannelState getCurrentState() const { return current_state_; }
    
    // State transitions
    bool transition(VideoChannelEvent event);
    
    // Get state name for logging
    const char* getStateName() const;
    
    // Get event name for logging
    static const char* getEventName(VideoChannelEvent event);

private:
    VideoChannelState current_state_;
    
    bool isValidTransition(VideoChannelEvent event) const;
};

} // namespace VideoPlayback

#endif // VIDEOCHANNELSTATE_H
