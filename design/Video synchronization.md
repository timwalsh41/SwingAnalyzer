# Synchronizer state diagram

## Overview

From the user perspective, the system is either in synchronization mode, where the user can adjust the offset between the two channels, or the playback mode, where the synchronized videos are played or otherwise manipulated via frame stepping or slider dragging using the individual channel controls to time sync the two channels. These two modes actually break down into four states implemented by the synchronizer state machine as described in the below diagram and the notes that follow.

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

- `SyncToPlayback` state
    - On entry to this state, we establish the synchronization between the two video channels
    - We determine the offset between the two as the difference in the delta from the start frame to the active frame
    - We then update the start frame for one channel or the other according to the offset
    - This state exists only to execute the state entry actions, we transition to the `PlaybackPaused` state on a tick event, where the tick event is always executed after any state transition.
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

The classies involved in synchronization are:
- `AnalyzerWindow` extends `Gtk::Window` and represents the GUI window itself
- `Analyzer` provides an interface that encapsulates the GTK GUI implementation
- `SyncManager` is the top level implementation of the video playback and synchronization functionality
    - GUI events are passed from the `Analyzer` to the `SyncManager`, and then handled through the synchronization state machine
        - In some cases, data from the event are passed directly from `SyncManager` to `Synchronizer`
        - If needed, the event handler within the state then triggers a function call to `Synchronizer` to process the provided data
    - Likewise, where events during video playback require action in the GUI, e.g. video playback triggering update of the position sliders, these are passed through `SyncManager` to the `Analyzer`
- `Synchronizer` manages the state data related to the synchronization between the two channels, and implements methods for manipulation of the data
    - The `SyncManager` and the individual states are declared as friend classes to the `Synchronizer`
- `ISynchronizerState` implements the interface to the state machine state implmentation
    - Member variables accessible to the subclasses include a pointer to the `Synchronizer` and the logger
    - There is also a protected member variable for the state name string, with an accessor function
    - The concrete states implemented are:
        - `SynchronizerStateSync`: the state representing sync mode of operation where the user can adjust the two video channels independently to time sync them
        - `SynchronizerStateSyncToPlayback`: this is used as a transition state between sync mode and playback mode, it is used to simply set the synchronization parameters and then will immediately transition to the playback (paused) mode
        - `SynchronizerStatePlaybackPaused`: this state represents the playback mode when the videos are paused
        - `SynchronizerStatePlaybackPlaying`: this state represents the playback mode when the videos are playing

## Diagram

```plantuml

@startuml

object AnalyzerWindow
object Analyzer
object SyncManager
object Synchronizer
object ISynchronizerState
object SynchronizerStateSync
object SynchronizerStateSyncToPlayback
object SynchronizerStatePlaybackPaused
object SynchronizerStatePlaybackPlaying
object VideoChannel

SyncManager *-- ISynchronizerState
ISynchronizerState <|-- SynchronizerStatePlaybackPlaying
ISynchronizerState <|-- SynchronizerStatePlaybackPaused
ISynchronizerState <|-- SynchronizerStateSync
ISynchronizerState <|-- SynchronizerStateSyncToPlayback
Synchronizer <-- ISynchronizerState
SyncManager *-- Synchronizer
SyncManager o-- VideoChannel
VideoChannel <-- Synchronizer
Analyzer o-- SyncManager
AnalyzerWindow *-- Analyzer

@enduml

```