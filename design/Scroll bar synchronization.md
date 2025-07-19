# Scroll bar synchronization

## Purpose

This document describes the implementation of the scroll bar synchronization feature between video channels in the SwingAnalyzer application.

## Sequence Diagrams

### Video playback frame update triggers scrollbar update

```plantuml
@startuml

collections VideoChannel
participant SyncManager
participant Synchronizer
participant VideoSynchronizationEngine
participant AnalyzerController
participant AnalyzerGui

VideoChannel -> VideoChannel : Playback frame update\n(playbackWorkerFunction)
VideoChannel -> VideoChannel : Frame update dispatcher\n(onFrameUpdate callback)
VideoChannel -> VideoChannel : Update displayed image
VideoChannel -> SyncManager : videoFrameUpdate(frame, channel)\n(via m_position_update_callback)
SyncManager -> SyncManager : Store active frame in\nm_new_active_frame
SyncManager -> SyncManager : VIDEO_FRAME_UPDATE event\n(L/R channel specific)\nHandled in state machine
alt Sync Mode
    SyncManager -> Synchronizer : videoFrameUpdate(channel, false)
    note right of Synchronizer : Set update_joint_slider\nto false
else Playback mode
    SyncManager -> Synchronizer : videoFrameUpdate(channel, true)
    note right of Synchronizer : Set update_joint_slider\nto true, will only trigger\nupdate for one channel
end alt
Synchronizer -> Synchronizer : Set frame based on new position
Synchronizer -> AnalyzerController : onVideoPositionChanged(channel, position)
note right of AnalyzerController: Triggered via observer
AnalyzerController -> AnalyzerGui : updateVideoPosition(channel, position)
AnalyzerGui -> AnalyzerGui : Update appropriate scrollbar\nposition (while blocking\nscrollbar update callbacks)

@enduml
```

### User drags channel scrollbar

Here, the scrollbar triggers the update of the video frame in one (sync mode) or both (playback mode) channels. The video channel triggers the callbacks and observer to update the slider. Since the slider update is conducted in a blocking mode, we don't get an infinite loop here. That is, the user moving the slider will trigger the slider getting adjusted because of the frame update, but because the latter slider update does not trigger a frame update we do not get an infinite loop.

```plantuml
@startuml

actor User
participant AnalyzerGui
participant AnalyzerController
participant VideoSynchronizationEngine
participant SyncManager
participant Synchronizer
collections VideoChannel

User -> AnalyzerGui : Channel slider drag
AnalyzerGui -> AnalyzerGui : onVideoPositionChanged(channel, position)
AnalyzerGui -> AnalyzerController : setVideoPosition(position, channel)\n[IVideoSynchronizationController]
AnalyzerController -> VideoSynchronizationEngine : setVideoPosition(position, channel)
VideoSynchronizationEngine -> SyncManager : videoPositionUpdate(position, channel)
SyncManager -> SyncManager : Store position in m_new_video_position
SyncManager -> SyncManager : VIDEO_POSITION_UPDATE event
SyncManager -> Synchronizer : videoPositionUpdate(channel)\n(channel-specific update)
note right of SyncManager : Handled via SyncManager\nstate machine\nVIDEO_POSITION_UPDATE event
Synchronizer -> VideoChannel : seekToFrame(calculated_frame)\n(pauses channel playback if active)
VideoChannel -> VideoChannel : Stop playback if active\nValidate frame bounds\nUpdate decoder position
VideoChannel -> VideoChannel : Update frame buffer and GUI
VideoChannel -> SyncManager : videoFrameUpdate(frame, channel)\n(notify position change\nvia m_position_callback)
note right of SyncManager : Handled via SyncManager\nstate machine\nVIDEO_FRAME_UPDATE event
alt Sync mode
    SyncManager -> Synchronizer : videoFrameUpdate(channel, false)
    note right of SyncManager: Set update_joint_slider\nto false
else Playback mode
    SyncManager -> Synchronizer : videoFrameUpdate(channel, true)
    note right of SyncManager: Set update_joint_slider\nto true
end alt
Synchronizer -> Synchronizer : Set frame based on new position
Synchronizer -> AnalyzerController : onVideoPositionChanged(channel, new_position)
note right of AnalyzerController: Triggered via observer
AnalyzerController -> AnalyzerGui : updateVideoPosition(channel, position)
AnalyzerGui -> AnalyzerGui : Update appropriate scrollbar\nposition (while blocking\nscrollbar update callbacks)

@enduml

```

### User steps frame

```plantuml
@startuml

actor User
participant AnalyzerGui
participant AnalyzerController
participant VideoSynchronizationEngine
participant SyncManager
participant Synchronizer
collections VideoChannel

User -> AnalyzerGui : Step frame button
AnalyzerGui -> AnalyzerController : stepFrame(channel, frames)
AnalyzerController -> VideoSynchronizationEngine : stepVideo(channel, frames)
VideoSynchronizationEngine -> SyncManager : handleEvent(STEP_xxx, channel)
SyncManager -> VideoChannel : stepFrame(frames)
note right of SyncManager : Handled via SyncManager\nstate machine\nstep frames events:\nSTEP_FORWARD_SMALL\nSTEP_FORWARD_LARGE\nSTEP_BACKWARD_SMALL\nSTEP_BACKWARD_LARGE
VideoChannel -> VideoChannel : Calculate new frame index
VideoChannel -> VideoChannel : seekToFrame(new_frame)\n(pauses channel playback if active)
VideoChannel -> VideoChannel : Stop playback if active\nValidate frame bounds\nUpdate decoder position
VideoChannel -> VideoChannel : Update frame buffer and GUI
VideoChannel -> SyncManager : videoFrameUpdate(frame, channel)\n(notify position change\nvia m_position_callback)
note right of SyncManager : Handled via SyncManager\nstate machine\nVIDEO_FRAME_UPDATE event
alt Sync mode
    SyncManager -> Synchronizer : videoFrameUpdate(channel, false)
    note right of SyncManager: Set update_joint_slider\nto false
else Playback mode
    SyncManager -> Synchronizer : videoFrameUpdate(channel, true)
    note right of SyncManager: Set update_joint_slider\nto true
end alt
Synchronizer -> Synchronizer : Set frame based on new position
Synchronizer -> AnalyzerController : onVideoPositionChanged(channel, new_position)
note right of AnalyzerController: Triggered via observer
AnalyzerController -> AnalyzerGui : updateVideoPosition(channel, position)
AnalyzerGui -> AnalyzerGui : Update appropriate scrollbar\nposition (while blocking\nscrollbar update callbacks)

@enduml

```

### Preventing Infinite Loops
The key to preventing infinite loops where the scroll bar updates the video position, which updates the scrollbar, ad nauseum, is that the scrollbar updates are implemented in a blocking fashion. That is, before setting the scroll bar position based on video channel frame updates, we block the scrollbar connection to prevent invoking the scroll bar value change callbacks.

```cpp
// In AnalyzerGui::updateVideoPosition()
void AnalyzerGui::updateVideoPosition(WhichVideoChannel channel, float position) {
    switch (channel) {
        case LEFT_CHANNEL:
            // Block the signal connection to prevent recursion
            m_left_channel_scrollbar_connection.block();
            m_guiObjects.scrollVideoLeft->set_value(position);
            m_left_channel_scrollbar_connection.unblock();
            break;
        // ...
    }
}

// In AnalyzerGui::connectSignals()
m_left_channel_scrollbar_connection = m_guiObjects.scrollVideoLeft->signal_value_changed().connect([this]() {
    // This callback is blocked during programmatic updates
    this->onVideoPositionChanged(LEFT_CHANNEL, this->m_guiObjects.scrollVideoLeft->get_value());
});
```
