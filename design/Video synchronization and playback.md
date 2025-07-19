# Synchronization and playback

## Synchronization theory of operations

The video synchronization system enables users to time-align two video channels (left and right) and then play them back with events synchronized between the two. The system is built around four core components that work together: the **VideoSynchronizationEngine**, **SyncManager**, **Synchronizer**, and **VideoChannel** classes.

### Component Overview

**VideoSynchronizationEngine** serves as the top-level business logic coordinator that:
- Manages observer pattern for GUI notifications
- Coordinates high-level video synchronization operations
- Provides a clean interface for controller layer integration
- Delegates state management to `SyncManager`

**SyncManager** maintains the synchronization system state:
    - Owns two `VideoChannel` instances (left and right) using shared pointers to share the video channels between `SyncManager` and `Synchronizer`
    - Implements a state machine to manage sync and playback modes
    - Routes events to the appropriate state handlers
    - Manages observer notifications for video position changes

**Synchronizer** manages the synchronization logic and data:
- Calculates frame offsets between video channels
- Handles framerate differences between videos
- Manages synchronized playback parameters
- Uses observer pattern for GUI updates instead of direct calls

**VideoChannel** handles individual video playback with a modular architecture using composition:
- **State Management**: `VideoChannelStateMachine` manages state transitions (UNLOADED/STOPPED/PLAYING) with event-driven transitions
- **Video Decoding**: `IVideoDecoder`/`OpenCVVideoDecoder` handles video file loading, seeking, and frame extraction
- **Frame Management**: `VideoFrameManager` provides frame range validation, bounds checking, and navigation logic
- **Thread Safety**: `ThreadSafeFrameBuffer` enables safe frame data sharing between playback and display threads using mutex protection
- **Playback Control**: `VideoPlaybackController` manages background playback threading with precise timing control
- **GUI Integration**: `IGUIImageUpdater`/`GTKImageUpdater` handles BGR-to-RGB conversion and Goocanvas pixbuf updates
- **Threading Model**: Uses `Glib::Dispatcher` for cross-thread communication from background playback to main GUI thread

### Synchronization Process

The synchronization process occurs in two distinct phases:

#### 1. Sync Mode (Manual Alignment)
When the system is in sync mode, users can independently control each video channel to establish the desired temporal alignment:

- **Individual Control**: Each video channel responds directly to its own independent set of controls (frame stepping, slider positioning)
- **Independent Positioning**: Users can set each video to different frame positions to align specific events (e.g., golf ball contact)
- **No Offset Calculation**: The system tracks current frame positions but does not yet calculate or apply any synchronization offset
- **GUI State**: Individual channel controls are enabled, while combined playback controls are disabled

#### 2. Transition to Playback Mode (Offset Calculation)
When transitioning from sync mode to playback mode, the system calculates and locks in the synchronization:

- **Offset Calculation**: The system determines the frame offset using the formula:
  ```
  offset = (left_active_frame - left_start_frame) - (right_active_frame - right_start_frame)
  ```
- **Start Frame Adjustment**: Based on the calculated offset:
  - If offset > 0: Left channel start frame is advanced by the offset amount
  - If offset < 0: Right channel start frame is advanced by the absolute offset amount
- **Length Determination**: The system calculates the effective playable length of each channel and uses the shorter length as the combined playback length
- **Master Channel Selection**: The channel with the shorter effective length becomes the master for GUI slider updates, which is to say the frame updates for the master channel triggers the joint slider update, and the second channel frame update events are ignored while in playback mode

#### 3. Synchronized Playback Mode
Once synchronization parameters are established, the system maintains temporal alignment between the two channels:

- **Framerate Compensation**: If videos have different framerates, the system calculates a framerate ratio and applies proportional frame stepping
- **Coordinated Control**: All playback operations (play, pause, step, seek) are applied to both channels with appropriate scaling
- **Unified GUI**: The combined slider controls both videos simultaneously, with position updates coming from the designated master channel
- **Boundary Enforcement**: Both videos are constrained to play within their calculated synchronized ranges

### Frame Update Mechanism

The system uses a callback-based approach for position synchronization:

1. **VideoChannel Position Callbacks**: Each `VideoChannel` is configured with a callback function (`m_position_callback`) that triggers when frame position changes during playback or seeking operations; this callback is set to the `videoFrameUpdate` method of the `SyncManager`
2. **Background Playback Thread**: The `VideoPlaybackController` manages background playback in a separate thread, with `playbackWorkerFunction()` continuously retrieving pending frames via `OpenCVVideoDecoder`
3. **Thread-Safe Frame Buffering**: Frames are stored in `ThreadSafeFrameBuffer` with mutex protection, then `Glib::Dispatcher` signals the main thread to update the GUI with the latest frame when it becomes available
4. **Main Thread Processing**: The `onFrameUpdate()` callback retrieves frames from the buffer, processes them through `GTKImageUpdater`, and triggers position callbacks
5. **State Machine Routing**: The `SyncManager` receives position callbacks and routes them through the current state's event handler as `VIDEO_FRAME_UPDATE` events
6. **Synchronizer Processing**: The `Synchronizer` calculates normalized position (0.0 to 1.0) and notifies observers (e.g to update appropriate sliders based on sync/playback mode)
7. **GUI Updates**: Individual channel sliders (sync mode) or joint slider (playback mode) are updated through direct calls to `AnalyzerGui` methods

This callback-based approach provides efficient position tracking with clean separation between threading, frame processing, and GUI updates.

### Framerate Handling

When videos have different framerates, the system maintains temporal synchronization:

- **Ratio Calculation**: `framerate_ratio = left_fps / right_fps`
- **Proportional Stepping**: Playback frame step operations are scaled by the framerate ratio
- **Temporal Alignment**: Videos advance at rates proportional to their framerates to maintain synchronized timing

This architecture ensures that regardless of the source videos' framerates or the user's initial alignment choices, the synchronized playback maintains perfect temporal coordination between the two video channels.

# Synchronizer state diagram

## Overview

From the user's perspective, the system is either in synchronization mode, where the user can adjust the offset between the two channels via frame stepping or slider dragging using the individual channel controls to time sync the two channels, or in playback mode, where the synchronized videos are played or otherwise manipulated by joint controls for stepping frames or seeking through the video with a scroll bar. These two modes actually break down into four states implemented by the synchronizer state machine as described in the below diagram and the notes that follow.

## Diagram

```plantuml

@startuml

[*] --> Sync

Sync -> SyncToPlayback : Sync mode deselected
Sync: Clear offset
Sync: Clear channel start/end frames

SyncToPlayback --> PlaybackPaused : Tick event
SyncToPlayback: Set sync parameters

PlaybackPaused --> PlaybackPlaying: Play button

PlaybackPlaying --> PlaybackPaused : Pause, step frame,\nor slider drag

PlaybackPlaying --> Sync : Sync mode\nselected

PlaybackPaused --> Sync: Sync mode selected

@enduml

```

## Notes

- Event handling is accomplished through `SyncManager::handleEvent(event, channel)`. The event is forwarded on to the synchronization state machine. This callback signature does not provide for passing any data. Therefore, we rely on the `SyncManager` handling of specific events where data are required. For specifics, please refer to the table below (where events have associated data, it is listed) and the `SyncManager` implementation of event handling for those events.
- `SyncToPlayback` state
    - On entry to this state, we establish the synchronization between the two video channels by calculating the net offset in video position that exists when the user triggers the transition from sync to playback mode
    - We determine the offset between the two as the difference in the delta from the start frame to the active frame
    - We then update the start frame for one channel or the other depending on the sign of the offset
    - This state exists only to execute the state entry actions, we transition to the `PlaybackPaused` state on a tick event, where the tick event is always executed after any state transition
        - In the event handler in `SyncManager`, first the nominal event is handled and then the `SyncManager` immediately inserts a tick event in order to transition the `SyncToPlayback` state to `PlaybackPaused`
- `Sync` state
    - On entry to this state, we clear the synchronization settings, that is we set the offset to zero and reset the start and end frames

## Events

The table below describes the events handled by the state machine. Each event includes a channel selection (left, right, both, or none). There may also be data associated with the event that is processed through the sync manager.

| Event | Description | Associated data | Handling in sync mode | Handling in playback mode |
|-------|-------------|-----------------|-----------------------|---------------------------|
| `TICK` | Artificially injected event after each state transition | N/A | Calculate sync parameters | N/A |
| `PLAY_OR_PAUSE` | Request to play (if paused) or pause (if playing) video(s) | N/A | If both channels are paused, then play both, otherwise pause both | Pause videos if playing, and vice versa |
| `PLAY` | Request to play video(s) | N/A | Play both video channels | Play both video channels |
| `PAUSE` | Request to pause video(s) | N/A | Pause both video channels | Pause both video channels |
| `SYNC_ON` | Request to enter sync mode | N/A | N/A | Transition to sync mode |
| `SYNC_OFF` | Request to enter playback mode | N/A | Transition to playback mode (via `SyncToPlayback`) | N/A |
| `PLAYBACK_COMPLETE` | Video channel indicating playback is complete | N/A | Do nothing (videos will start from beginning on next playback) | Restore videos to start position and pause |
| `STEP_FORWARD_LARGE` | Request to step video(s) forward by 5 frames | N/A | Pause playback and step video | Pause playback and step videos |
| `STEP_FORWARD_SMALL` | Request to step video(s) forward by 1 frame | N/A | Pause playback and step video | Pause playback and step videos |
| `STEP_BACKWARD_LARGE` | Request to step video(s) backward by 5 frames | N/A | Pause playback and step video | Pause playback and step videos |
| `STEP_BACKWARD_SMALL` | Request to step video(s) backward by 1 frame | N/A | Pause playback and step video | Pause playback and step videos |
| `SET_VIDEO_TO_START` | Request to set video(s) to start position | N/A | N/A | Set video to start |
| `PLAYBACK_RATE_UPDATE` | Request to update the playback rate | Desired playback rate multiplier | Pause videos and update video channel playback rate | Pause videos and update video channel playback rate |
| `VIDEO_POSITION_UPDATE` | Request to set the video to desired position | Float position between 0.0 (start) and 1.0 (end) | Pause video and update active frame | Pause videos and update active frames |
| `VIDEO_FRAME_UPDATE` | Video channel playback has stepped forward by one frame | Integer frame index | Update GUI slider position | Update GUI slider position |
| `LOAD_VIDEO` | Request to load a new video file | Path of video file to load | Attempt to load new video file | Attempt to load new video file |

# Synchronizer class diagram

## Description

The classes involved in synchronization are:

**GUI Layer:**
- `AnalyzerWindow` extends `Gtk::Window` and represents the main application window
- `AnalyzerGui` provides pure GUI management focused on GTK widget handling and user interface components

**Controller Layer:**
- `AnalyzerController` implements both `IVideoSynchronizationController` and `IVideoSynchronizationObserver` interfaces
  - Mediates between GUI events and business logic using dependency injection
  - Provides clean separation of concerns between presentation and business logic through observer pattern implementation

**Business Logic Layer:**
- `VideoSynchronizationEngine` serves as the main business logic coordinator
  - Manages observer pattern for loose coupling with GUI layer
  - Coordinates high-level video synchronization operations
  - Provides clean interface for controller integration
- `SyncManager` is the state machine orchestrator for video playback and synchronization
  - Owns video channels using shared pointer ownership model to share with `Synchronizer`
  - Routes events through the synchronization state machine
  - Manages observer notifications for position updates
- `Synchronizer` manages the core synchronization logic and data
  - Calculates frame offsets and handles framerate differences
  - Uses observer pattern to route event information to the GUI
  - Friend classes with `SyncManager` and synchronization state machine states for controlled access

**State Machine:**
- `ISynchronizerState` implements the interface for state machine states
  - Provides access to Synchronizer and logger for state implementations
  - Contains state name and event handling interface
  - The concrete states implemented are:
    - `SynchronizerStateSync`: Independent video channel control for manual alignment
    - `SynchronizerStateSyncToPlayback`: Transition state for offset calculation and parameter setup
    - `SynchronizerStatePlaybackPaused`: Synchronized playback in paused state
    - `SynchronizerStatePlaybackPlaying`: Synchronized playback in playing state

**Video Infrastructure:**
- `VideoChannel` implements the logic for loading and playback of a single video channel
  - Uses state machine (`VideoChannelStateMachine`) for clean state management
  - Delegates video decoding to `IVideoDecoder` interface (implemented by `OpenCVVideoDecoder`)
  - Delegates GUI updates to `IGUIImageUpdater` interface (implemented by `GTKImageUpdater`)
  - Uses helper classes for frame management, playback control, and thread-safe buffering

## Diagram

```plantuml

@startuml

package "GUI Layer" {
    class AnalyzerWindow {
        +AnalyzerWindow()
        -m_pController: shared_ptr<AnalyzerController>
        -m_pGui: unique_ptr<AnalyzerGui>
        -m_builder: Glib::RefPtr<Gtk::Builder>
    }
    
    class AnalyzerGui {
        +AnalyzerGui()
        +setVideoRenderer()
        +updateVideoPosition()
        +syncModeToggle()
        -m_controller: IVideoSynchronizationController*
        -m_guiObjects: AnalyzerGuiObjects
        -m_canvas_left: Goocanvas::Canvas*
        -m_canvas_right: Goocanvas::Canvas*
    }
}

package "Controller Layer" {
    interface IVideoSynchronizationController {
        +playVideo()
        +pauseVideo()
        +stepVideo()
        +setVideoPosition()
        +loadVideo()
        +setSyncMode()
    }
    
    interface IVideoSynchronizationObserver {
        +onVideoPositionChanged()
        +onPlaybackStateChanged()
        +onSyncModeChanged()
        +onVideoLoaded()
    }
    
    class AnalyzerController {
        +AnalyzerController()
        +initialize()
        +getVideoChannel()
        -m_gui: AnalyzerGui*
        -m_engine: shared_ptr<VideoSynchronizationEngine>
    }
}

package "Business Logic Layer" {
    class VideoSynchronizationEngine {
        +VideoSynchronizationEngine()
        +addObserver()
        +playVideo()
        +loadVideo()
        +start()
        -m_syncManager: unique_ptr<SyncManager>
        -m_observers: vector<weak_ptr<IVideoSynchronizationObserver>>
    }
    
    class SyncManager {
        +SyncManager()
        +handleEvent()
        +setObserver()
        +start()
        -m_channel_left: shared_ptr<VideoChannel>
        -m_channel_right: shared_ptr<VideoChannel>
        -m_synchronizer: Synchronizer
        -m_state: unique_ptr<ISynchronizerState>
        -m_observer: weak_ptr<IVideoSynchronizationObserver>
    }
    
    class Synchronizer {
        +setObserver()
        +calculateFramerateRatio()
        +stepVideos()
        +videoPositionUpdate()
        +videoFrameUpdate()
        -m_pChannel_left: shared_ptr<VideoChannel>
        -m_pChannel_right: shared_ptr<VideoChannel>
        -m_observer: weak_ptr<IVideoSynchronizationObserver>
        -m_video_frame_offset: int
        -m_framerate_ratio: float
    }
}

package "State Machine" {
    interface ISynchronizerState {
        +handleEvent()
        +getName()
        #m_pSynchronizer: Synchronizer*
        #m_name: string
    }
    
    class SynchronizerStateSync
    class SynchronizerStateSyncToPlayback
    class SynchronizerStatePlaybackPaused
    class SynchronizerStatePlaybackPlaying
}

package "Video Infrastructure" {
    class VideoChannel {
        +VideoChannel()
        +loadVideo()
        +playVideo()
        +pauseVideo()
        +stepFrame()
        +getImage()
        -m_state_machine: unique_ptr<VideoChannelStateMachine>
        -m_video_decoder: unique_ptr<IVideoDecoder>
        -m_gui_updater: unique_ptr<IGUIImageUpdater>
        -m_frame_manager: unique_ptr<VideoFrameManager>
        -m_playback_controller: unique_ptr<VideoPlaybackController>
        -m_frame_buffer: unique_ptr<ThreadSafeFrameBuffer>
    }
    
    class VideoChannelStateMachine
    interface IVideoDecoder
    class OpenCVVideoDecoder
    interface IGUIImageUpdater
    class GTKImageUpdater
    class VideoFrameManager
    class VideoPlaybackController
    class ThreadSafeFrameBuffer
}

' Layer relationships - Clean separation with observer pattern
AnalyzerWindow *-- AnalyzerGui
AnalyzerWindow o-- AnalyzerController
AnalyzerGui --> IVideoSynchronizationController
AnalyzerController ..|> IVideoSynchronizationController
AnalyzerController ..|> IVideoSynchronizationObserver
AnalyzerController o-- VideoSynchronizationEngine

' Business logic relationships
VideoSynchronizationEngine *-- SyncManager
SyncManager *-- Synchronizer
SyncManager o-- VideoChannel
Synchronizer --> VideoChannel

' State machine relationships
SyncManager o-- ISynchronizerState
ISynchronizerState <|-- SynchronizerStateSync
ISynchronizerState <|-- SynchronizerStateSyncToPlayback
ISynchronizerState <|-- SynchronizerStatePlaybackPaused
ISynchronizerState <|-- SynchronizerStatePlaybackPlaying
Synchronizer <-- ISynchronizerState

' VideoChannel internal architecture
VideoChannel *-- VideoChannelStateMachine
VideoChannel *-- IVideoDecoder
VideoChannel *-- IGUIImageUpdater
VideoChannel *-- VideoFrameManager
VideoChannel *-- VideoPlaybackController
VideoChannel *-- ThreadSafeFrameBuffer

' Interface implementations
IVideoDecoder <|-- OpenCVVideoDecoder
IGUIImageUpdater <|-- GTKImageUpdater

' Observer pattern relationships (shown with dashed lines to emphasize loose coupling)
VideoSynchronizationEngine ..> IVideoSynchronizationObserver : notifies
SyncManager ..> IVideoSynchronizationObserver : notifies
@enduml

```

## Video Playback Position Update Sequence

The following sequence diagram shows how video position updates flow through the modular `VideoChannel` architecture for video playback:

```plantuml

@startuml

participant "VideoPlaybackController" as VPC
participant "VideoChannel" as VC
participant "OpenCVVideoDecoder" as VD
participant "ThreadSafeFrameBuffer" as TSF
participant "GTKImageUpdater" as GUI
participant "SyncManager" as SM
participant "Synchronizer" as SY

box "VideoChannel Components" #LightBlue
    participant VPC
    participant VD
    participant TSF
    participant GUI
end box

note over VPC, SY : Video Position Update Flow - Internal Playback

== Background Thread Playback ==
VPC -> VC : playbackWorkerFunction()
note right of VPC : Background thread execution

VC -> VD : getNextFrame(frame)
activate VD
note right of VD : Extract frame from video file
deactivate VD

VC -> TSF : updateFrame(frame)
activate TSF
note right of TSF : Thread-safe frame storage\n(mutex protected)
deactivate TSF

VC -> VC : m_frame_update_dispatcher.emit()
note right of VC : Signal main thread

== Main Thread Update ==
VC -> VC : onFrameUpdate()
note right of VC : Callback for frame update dispatcher\nfrom playback worker thread

VC -> TSF : getDisplayFrame(target_size)
activate TSF
note right of TSF : Thread-safe frame retrieval\n(resized for display)
deactivate TSF

VC -> GUI : updateImage(display_frame)
activate GUI
note right of GUI : BGR to RGB conversion\nUpdate Goocanvas pixbuf
deactivate GUI

VC -> SM : videoFrameUpdate(frame, channel)
note right of VC : Invoked via VideoChannel::m_position_callback

== State Machine Processing ==
SM -> SM : videoFrameUpdate(frame, channel)
note right of SM : Store active frame

SM -> SM : handleEvent(VIDEO_FRAME_UPDATE, channel)
note right of SM : Route through current state

SM -> SY : videoFrameUpdate(channel, update_joint_slider)
note right of SY : Notify observers of frame update

@enduml

```

## User-Initiated Position Update Sequence

The following sequence diagram shows how external position updates (slider drags, frame stepping) are handled:

```plantuml

@startuml

participant "SyncManager" as SM
participant "Synchronizer" as SY
participant "VideoChannel" as VC
participant "VideoFrameManager" as VFM
participant "VideoChannelStateMachine" as VSM
participant "OpenCVVideoDecoder" as VD
participant "ThreadSafeFrameBuffer" as TSF
participant "GTKImageUpdater" as GUI

box "VideoChannel Components" #LightBlue
    participant VFM
    participant VSM
    participant VD
    participant TSF
    participant GUI
end box

== Position Update Request ==
SM -> SY : videoPositionUpdate(channel)
note right of SM : From slider position update,\nvia AnalyzerController to\nVideoSynchronizationEnginer\nto SyncManager 
SY -> SY : Calculate target frame\nfrom normalized position
note right of SY : frame = start + (position * length)

SY -> VC : seekToFrame(target_frame)

== Frame Seeking Process ==
VC -> VSM : canSeek()
activate VSM
note right of VSM : State validation
deactivate VSM

VC -> VFM : isValidFrame(target_frame)
activate VFM
note right of VFM : Bounds checking
deactivate VFM

VC -> VC : Stop playback if active
note right of VC : Pause current playback

VC -> VD : seekToFrame(target_frame)
activate VD
note right of VD : Seek to position in video file
deactivate VD

VC -> VD : getCurrentFrame(frame_data)
activate VD
note right of VD : Extract frame at position
deactivate VD

VC -> TSF : updateFrame(frame_data)
activate TSF
note right of TSF : Store frame in thread-safe buffer
deactivate TSF

VC -> TSF : getDisplayFrame(target_size)
activate TSF
note right of TSF : Retrieve resized frame
deactivate TSF

VC -> GUI : updateImage(display_frame)
activate GUI
note right of GUI : Update display with new frame
deactivate GUI

VC -> SM : m_position_callback(frame, channel)
note right of VC : Update slider position\n(avoid feedback loops)

@enduml

```

## Scroll Bar Synchronizaion

See <a href="Scroll bar synchronization.md">scroll bar synchronization</a> for more details on handling scroll bar updates.