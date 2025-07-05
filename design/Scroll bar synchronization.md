## Sync mode

### User drags channel scrollbar

```plantuml
@startuml

actor User
participant Analyzer
participant SyncManager
participant Synchronizer
collections VideoChannel

User -> Analyzer : Channel slider drag
Analyzer -> SyncManager : videoPositionUpdate(position, channel)
SyncManager -> SyncManager : Store position in m_new_video_position
SyncManager -> SyncManager : VIDEO_POSITION_UPDATE event
SyncManager -> Synchronizer : videoPositionUpdate(channel)\n(channel-specific update)
Synchronizer -> VideoChannel : seekToFrame(calculated_frame)\n(pauses channel playback if active)
VideoChannel -> VideoChannel : Stop playback if active\nValidate frame bounds\nUpdate decoder position
VideoChannel -> VideoChannel : Update frame buffer and GUI

@enduml
```

### Video frame update triggers scrollbar update

```plantuml
@startuml

collections VideoChannel
participant SyncManager
participant Synchronizer
participant Analyzer

VideoChannel -> VideoChannel : Playback frame update\n(playbackWorkerFunction)
VideoChannel -> VideoChannel : Frame update dispatcher\n(onFrameUpdate callback)
VideoChannel -> SyncManager : m_position_callback(frame, channel)\n(via constructor callback)
SyncManager -> SyncManager : Store active frame in\nm_new_active_frame
SyncManager -> Synchronizer : Store frame index
SyncManager -> SyncManager : VIDEO_FRAME_UPDATE event\n(L/R channel specific)
SyncManager -> Synchronizer : videoFrameUpdate(channel, false)\n(individual channel mode)
Synchronizer -> Analyzer : updateLeftVideoPosition() or\nupdateRightVideoPosition()\n(channel-specific slider update)

@enduml
```

## Playback mode

### User drags joint scrollbar

```plantuml
@startuml

actor User
participant Analyzer
participant SyncManager
participant Synchronizer
collections VideoChannel

User -> Analyzer : Global slider drag
Analyzer -> SyncManager : videoPositionUpdate(position, BOTH_CHANNELS)
SyncManager -> SyncManager : Store position in m_new_video_position
SyncManager -> SyncManager : VIDEO_POSITION_UPDATE event\n(triggers transition to\nPLAYBACKPAUSED state\nif in PLAYBACKPLAYING)
SyncManager -> Synchronizer : videoPositionUpdate(BOTH_CHANNELS)
Synchronizer -> VideoChannel : seekToFrame(left_frame)\nseekToFrame(right_frame)\n(calculated based on combined length)

@enduml
```

### Update joint scrollbar during playback

```plantuml
@startuml

collections VideoChannel
participant SyncManager
participant Synchronizer
participant Analyzer

VideoChannel -> VideoChannel : Playback frame update\n(playbackWorkerFunction)
VideoChannel -> VideoChannel : Frame update dispatcher\n(onFrameUpdate callback)
VideoChannel -> SyncManager : m_position_callback(frame, channel)\n(only from designated channel)
note right: In playback mode, only one channel\n(m_video_position_update_channel)\ntriggers joint slider updates
SyncManager -> SyncManager : Store active frame in\nm_new_active_frame
SyncManager -> Synchronizer : Store frame index
SyncManager -> SyncManager : VIDEO_FRAME_UPDATE event\n(designated channel only)
SyncManager -> Synchronizer : videoFrameUpdate(channel, true)\n(joint slider mode)
Synchronizer -> Analyzer : updateCombinedVideoPosition()\n(joint slider update)

@enduml
```

## Implementation Details

### VideoChannel Callback Architecture

The VideoChannel implements position callbacks through a constructor parameter:
```cpp
VideoChannel(WhichVideoChannel channel, 
            std::function<void(int, WhichVideoChannel)> update_callback)
```

The callback (`m_position_callback`) is triggered in two key locations:
1. **seekToFrame()** - When externally seeking to a specific frame (slider drag, stepping)
2. **onFrameUpdate()** - During internal playback via the frame update dispatcher

### Position Update Flow

**External Updates (Slider/Step):**
- `Analyzer` → `SyncManager::videoPositionUpdate()` → `Synchronizer::videoPositionUpdate()` → `VideoChannel::seekToFrame()`
- `seekToFrame()` triggers `m_position_callback()` to update the slider position

**Internal Updates (Playback):**
- `playbackWorkerFunction()` → `m_frame_update_dispatcher.emit()` → `onFrameUpdate()` → `m_position_callback()`
- The callback flows to `SyncManager::videoFrameUpdate()` → state machine → `Synchronizer::videoFrameUpdate()`

### State-Dependent Behavior

**Sync Mode:**
- Each channel independently updates its own slider via `updateLeftVideoPosition()` or `updateRightVideoPosition()`
- Both channels can trigger position updates independently

**Playback Mode:**
- Only the designated channel (`m_video_position_update_channel`) updates the joint slider
- The channel selection is determined during sync-to-playback transition based on which video is shorter
- Joint slider updates use `updateCombinedVideoPosition()`

### Thread Safety

The VideoChannel uses thread-safe mechanisms:
- `Glib::Dispatcher` for cross-thread communication from playback worker to main thread
- `ThreadSafeFrameBuffer` for frame data sharing between threads
- Position callbacks are invoked from the main thread context

## Error Handling Scenarios

### Input Validation Errors

The system includes comprehensive input validation with appropriate error responses:

#### Invalid Slider Position
```plantuml
@startuml

actor User
participant Analyzer  
participant SyncManager
participant Logger

User -> Analyzer : Slider drag (invalid position)
Analyzer -> SyncManager : videoPositionUpdate(position < 0 || > 1, channel)
SyncManager -> SyncManager : Validate position range
alt Invalid position
    SyncManager -> Logger : warn("Invalid video position %.3f, valid range is (0.0,1.0)")
    SyncManager -> User : Early return, no action taken
else Valid position
    SyncManager -> SyncManager : Continue normal processing
end

@enduml
```

#### Invalid Frame Index
```plantuml
@startuml

participant Synchronizer
participant VideoChannel
participant VideoFrameManager
participant Logger

Synchronizer -> VideoChannel : seekToFrame(invalid_frame)
VideoChannel -> VideoChannel : Validate state (canSeek())
alt Invalid state
    VideoChannel -> Logger : warn("Cannot seek in current state: %s")
    VideoChannel -> Synchronizer : Early return
else Valid state
    VideoChannel -> VideoFrameManager : isValidFrame(frame)
    alt Invalid frame
        VideoChannel -> Logger : warn("Invalid frame index: %d (valid range: %d-%d)")
        VideoChannel -> Synchronizer : Early return
    else Valid frame
        VideoChannel -> VideoChannel : Continue seek operation
    end
end

@enduml
```

#### Invalid Video Size
```plantuml
@startuml

participant Caller
participant VideoChannel
participant Logger

Caller -> VideoChannel : setVideoSize(width, height)
VideoChannel -> VideoChannel : Check if video loaded
alt Not loaded
    VideoChannel -> Caller : Early return
else Loaded
    VideoChannel -> VideoChannel : Validate dimensions (width < 10 || height < 10)
    alt Invalid dimensions
        VideoChannel -> Logger : error("Invalid video size: %dx%d")
        VideoChannel -> Caller : Early return
    else Valid dimensions
        VideoChannel -> VideoChannel : Apply size with aspect ratio
    end
end

@enduml
```

### State Machine Validation Errors

#### Invalid State Transitions
```plantuml
@startuml

participant VideoChannel
participant VideoChannelStateMachine
participant Logger

VideoChannel -> VideoChannelStateMachine : transition(event)
VideoChannelStateMachine -> VideoChannelStateMachine : isValidTransition(event)
alt Invalid transition
    VideoChannelStateMachine -> VideoChannel : return false
    VideoChannel -> Logger : warn("Invalid state transition: %s in state %s")
    VideoChannel -> VideoChannel : Operation rejected
else Valid transition
    VideoChannelStateMachine -> VideoChannelStateMachine : Apply state change
    VideoChannelStateMachine -> VideoChannel : return true
end

@enduml
```

#### Playback State Conflicts
```plantuml
@startuml

participant User
participant VideoChannel
participant VideoChannelStateMachine
participant Logger

User -> VideoChannel : playVideo()
VideoChannel -> VideoChannelStateMachine : transition(PLAY)
alt Cannot play (wrong state)
    VideoChannelStateMachine -> VideoChannel : return false
    VideoChannel -> Logger : warn("Cannot play video in current state: %s")
    VideoChannel -> User : Operation ignored
else Can play
    VideoChannel -> VideoChannel : Start playback process
end

@enduml
```

### Frame Management Errors

#### Frame Range Validation
```plantuml
@startuml

participant VideoChannel
participant VideoFrameManager
participant Logger

VideoChannel -> VideoFrameManager : setStartFrame(invalid_start)
VideoFrameManager -> VideoFrameManager : Validate range (start < 0 || start >= end)
alt Invalid range
    VideoFrameManager -> Logger : warn("Invalid start frame: %d (valid range: 0-%d)")
    VideoFrameManager -> VideoChannel : Early return, no change
else Valid range
    VideoFrameManager -> VideoFrameManager : Apply frame range
end

@enduml
```

#### Frame Step Bounds Checking
```plantuml
@startuml

participant VideoChannel
participant VideoFrameManager

VideoChannel -> VideoFrameManager : stepFrame(current_frame, offset)
VideoFrameManager -> VideoFrameManager : Calculate new_frame = current + offset
VideoFrameManager -> VideoFrameManager : clampFrame(new_frame)
note right: Automatically clamps to valid range\n[start_frame, end_frame]
VideoFrameManager -> VideoChannel : return clamped_frame

@enduml
```

### Thread Safety and Resource Errors

#### Empty Frame Handling
```plantuml
@startuml

participant playbackWorkerFunction
participant VideoDecoder
participant ThreadSafeFrameBuffer
participant Logger

playbackWorkerFunction -> VideoDecoder : getNextFrame(frame)
VideoDecoder -> playbackWorkerFunction : return frame
alt Frame is empty
    playbackWorkerFunction -> Logger : warn("Frame %d is empty for %s channel")
    playbackWorkerFunction -> playbackWorkerFunction : Skip frame update, continue loop
else Frame is valid
    playbackWorkerFunction -> ThreadSafeFrameBuffer : updateFrame(frame)
    playbackWorkerFunction -> playbackWorkerFunction : emit frame update
end

@enduml
```

#### Buffer Validation
```plantuml
@startuml

participant Caller
participant ThreadSafeFrameBuffer
participant Logger

Caller -> ThreadSafeFrameBuffer : updateFrame(new_frame)
ThreadSafeFrameBuffer -> ThreadSafeFrameBuffer : Check if new_frame.empty()
alt Empty frame
    ThreadSafeFrameBuffer -> Logger : warn("Attempted to update frame buffer with empty frame")
    ThreadSafeFrameBuffer -> Caller : Early return, no update
else Valid frame
    ThreadSafeFrameBuffer -> ThreadSafeFrameBuffer : Lock mutex and clone frame
end

@enduml
```

### Decoder and File System Errors

#### Video Load Failure
```plantuml
@startuml

participant VideoChannel
participant VideoDecoder
participant VideoChannelStateMachine
participant Logger

VideoChannel -> VideoDecoder : loadVideo(path)
VideoDecoder -> VideoDecoder : Attempt to open video file
alt Load failure
    VideoDecoder -> VideoChannel : return VIDEO_LOAD_ERROR_*
    VideoChannel -> VideoChannelStateMachine : transition(ERROR_OCCURRED)
    note right: Revert state to handle failure
    VideoChannel -> VideoChannel : Return error status
else Load success
    VideoDecoder -> VideoChannel : return VIDEO_LOAD_SUCCESS
    VideoChannel -> VideoChannel : Continue initialization
end

@enduml
```

#### Frame Seek Failure
```plantuml
@startuml

participant VideoChannel
participant OpenCVVideoDecoder
participant Logger

VideoChannel -> OpenCVVideoDecoder : seekToFrame(frame_number)
OpenCVVideoDecoder -> OpenCVVideoDecoder : Validate loaded and frame range
alt Invalid frame or not loaded
    OpenCVVideoDecoder -> Logger : warn("Invalid frame number: %d (valid range: 0-%d)")
    OpenCVVideoDecoder -> VideoChannel : return false
else Valid request
    OpenCVVideoDecoder -> OpenCVVideoDecoder : cv::VideoCapture.set(POS_FRAMES)
    OpenCVVideoDecoder -> VideoChannel : return success/failure
end

@enduml
```

### Configuration Validation Errors

#### Framerate Multiplier Validation
```plantuml
@startuml

participant Caller
participant VideoChannel
participant Logger

Caller -> VideoChannel : setFramerateMultiplier(multiplier)
VideoChannel -> VideoChannel : Validate range (MIN_FRAMERATE_MULTIPLIER to MAX_FRAMERATE_MULTIPLIER)
alt Out of range
    VideoChannel -> Logger : warn("Invalid framerate multiplier %.3f, valid range is (%.3f,%.3f)")
    VideoChannel -> Caller : Early return, no change
else Valid range
    VideoChannel -> VideoChannel : Pause if playing
    VideoChannel -> VideoChannel : Apply multiplier
end

@enduml
```

#### Synchronizer Channel Validation
```plantuml
@startuml

participant SynchronizerState
participant Synchronizer
participant Logger

SynchronizerState -> SynchronizerState : handleEvent(STEP_FORWARD_SMALL, channel)
SynchronizerState -> SynchronizerState : Validate channel (LEFT_CHANNEL || RIGHT_CHANNEL)
alt Invalid channel
    SynchronizerState -> Logger : warn("Invalid channel selected for frame step %d")
    SynchronizerState -> SynchronizerState : No action taken
else Valid channel
    SynchronizerState -> Synchronizer : Forward to appropriate video channel
end

@enduml
```
