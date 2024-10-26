# Overview

The video channel frame update can occur through the following mechanisms:
- Internal playback
- User initiated frame step
- User initiated slider drag

The update pathway for each of these is shown below

## Internal playback

In internal playback, the displayed frame is updated periodically. We use a mutex to transfer the newly captured frame through a frame buffer to the displayed frame object, where it is then used to update the canvas image. At each frame update, we trigger a slider position update.

```plantuml

@startuml

box VideoChannel #LightBlue
participant WorkerThread
participant playVideoWorkerThread

participant MainThread
participant DispatchCallback
participant updatePixBuf

participant Dispatcher
end box

participant SyncManager

WorkerThread -> playVideoWorkerThread : Periodic call
note right: Thread-safe load\n of frame into buffer\nRetrieve next frame
playVideoWorkerThread -> Dispatcher : emit()
Dispatcher -> MainThread : Signal callback
MainThread -> DispatchCallback : DispatchCallback()
note right: Thread-safe copy\nof frame buffer\ninto display frame
DispatchCallback -> updatePixBuf : updatePixBuf()
note right: Update displayed\nimage on canvas
DispatchCallback -> SyncManager : Update video\nposition callback
note right: Update slider

@enduml

```
## Externally triggered updates

There are two paths to update the displayed frame: stepping by an offset, and setting the active frame index directly. These are very similar pathways that involve setting the frame index, retrieving the corresponding frame, and then updating the display. These operations are performed in a single thread, simplifying the data management.

### Slider drag

The slider drag is handled in the synchronizer, which calculates the index of the frame that the video should be at. It triggers the video channel to load that frame. Since the update came from the slider itself, we do not trigger a slider position update.

```plantuml

@startuml

participant Synchronizer
box "VideoChannel" #LightBlue
participant setVideoToFrame
participant setActiveFrame
participant updatePixBuf
end box

Synchronizer -> setVideoToFrame
note right: Set active frame\nbased on input
setVideoToFrame -> setActiveFrame
note right: Load requested\nframe into\ndisplay frame
setActiveFrame -> updatePixBuf
note right: Update displayed\nimage on canvas

@enduml

```

### Frame step

Here we step the frame by the provided offset, triggering a slider update at the end.

Right now frame steps can come from the Analyzer (in sync mode, requests come right from the buttons on the GUI) or the Synchronizer (in playback mode, step requests go to the Synchronizer and are then sent to both channels).

```plantuml

@startuml 

participant Analyzer
participant Synchronizer

box "VideoChannel" #LightBlue
participant stepFrame
participant setActiveFrame
participant updatePixBuf
end box

alt Analyzer induced
    Analyzer -> stepFrame
else Synchronizer induced
    Synchronizer -> stepFrame
end
stepFrame -> setActiveFrame
note right: Load requested\nframe into\ndisplay frame
setActiveFrame -> updatePixBuf
note right: Update displayed\nimage on canvas
stepFrame -> Synchronizer: Update video\nposition callback
note left: Update slider

@enduml

```