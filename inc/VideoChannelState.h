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
    VideoChannelStateMachine() : m_current_state(VideoChannelState::UNLOADED) {}

    // State queries
    bool canPlay() const { return m_current_state == VideoChannelState::STOPPED; }
    bool canPause() const { return m_current_state == VideoChannelState::PLAYING; }
    bool canSeek() const { return m_current_state != VideoChannelState::UNLOADED; }
    bool canStep() const { return m_current_state != VideoChannelState::UNLOADED; }
    bool isLoaded() const { return m_current_state != VideoChannelState::UNLOADED; }
    bool isPlaying() const { return m_current_state == VideoChannelState::PLAYING; }

    VideoChannelState getCurrentState() const { return m_current_state; }

    // State transitions
    bool transition(VideoChannelEvent event);
    
    // Check if transition is valid without performing it
    bool isValidTransition(VideoChannelEvent event) const;

    // Get state name for logging
    const char* getStateName() const;

    // Get event name for logging
    static const char* getEventName(VideoChannelEvent event);

private:
    VideoChannelState m_current_state;
};

} // namespace VideoPlayback

#endif // VIDEOCHANNELSTATE_H
