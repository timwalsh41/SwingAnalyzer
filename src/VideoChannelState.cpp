#include <VideoChannelState.h>
#include <unordered_map>

namespace VideoPlayback
{

bool VideoChannelStateMachine::transition(VideoChannelEvent event) {
    if (!isValidTransition(event)) {
        return false;
    }

    switch (m_current_state) {
        case VideoChannelState::UNLOADED:
            if (event == VideoChannelEvent::LOAD_VIDEO) {
                m_current_state = VideoChannelState::STOPPED;
            }
            break;

        case VideoChannelState::STOPPED:
            switch (event) {
                case VideoChannelEvent::PLAY:
                    m_current_state = VideoChannelState::PLAYING;
                    break;
                case VideoChannelEvent::UNLOAD_VIDEO:
                    m_current_state = VideoChannelState::UNLOADED;
                    break;
                case VideoChannelEvent::STEP_FRAME:
                case VideoChannelEvent::SEEK_FRAME:
                    // Stay in STOPPED state
                    break;
                default:
                    return false;
            }
            break;

        case VideoChannelState::PLAYING:
            switch (event) {
                case VideoChannelEvent::PAUSE:
                case VideoChannelEvent::STEP_FRAME:
                case VideoChannelEvent::SEEK_FRAME:
                    m_current_state = VideoChannelState::STOPPED;
                    break;
                case VideoChannelEvent::UNLOAD_VIDEO:
                    m_current_state = VideoChannelState::UNLOADED;
                    break;
                default:
                    return false;
            }
            break;
    }

    return true;
}

bool VideoChannelStateMachine::isValidTransition(VideoChannelEvent event) const {
    switch (m_current_state) {
        case VideoChannelState::UNLOADED:
            return event == VideoChannelEvent::LOAD_VIDEO;

        case VideoChannelState::STOPPED:
            return event == VideoChannelEvent::PLAY ||
                   event == VideoChannelEvent::UNLOAD_VIDEO ||
                   event == VideoChannelEvent::STEP_FRAME ||
                   event == VideoChannelEvent::SEEK_FRAME;

        case VideoChannelState::PLAYING:
            return event == VideoChannelEvent::PAUSE ||
                   event == VideoChannelEvent::UNLOAD_VIDEO ||
                   event == VideoChannelEvent::STEP_FRAME ||
                   event == VideoChannelEvent::SEEK_FRAME;
    }

    return false;
}

const char* VideoChannelStateMachine::getStateName() const {
    static const std::unordered_map<VideoChannelState, const char*> state_names = {
        {VideoChannelState::UNLOADED, "UNLOADED"},
        {VideoChannelState::STOPPED, "STOPPED"},
        {VideoChannelState::PLAYING, "PLAYING"}
    };

    auto it = state_names.find(m_current_state);
    return (it != state_names.end()) ? it->second : "UNKNOWN";
}

const char* VideoChannelStateMachine::getEventName(VideoChannelEvent event) {
    static const std::unordered_map<VideoChannelEvent, const char*> event_names = {
        {VideoChannelEvent::LOAD_VIDEO, "LOAD_VIDEO"},
        {VideoChannelEvent::UNLOAD_VIDEO, "UNLOAD_VIDEO"},
        {VideoChannelEvent::PLAY, "PLAY"},
        {VideoChannelEvent::PAUSE, "PAUSE"},
        {VideoChannelEvent::STEP_FRAME, "STEP_FRAME"},
        {VideoChannelEvent::SEEK_FRAME, "SEEK_FRAME"},
        {VideoChannelEvent::ERROR_OCCURRED, "ERROR_OCCURRED"}
    };

    auto it = event_names.find(event);
    return (it != event_names.end()) ? it->second : "UNKNOWN";
}

} // namespace VideoPlayback
