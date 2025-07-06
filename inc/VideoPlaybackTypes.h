#ifndef VIDEOPLAYBACKTYPES_H
#define VIDEOPLAYBACKTYPES_H

#include <unordered_map>

namespace VideoPlayback
{

struct VideoSize
{
    int width;
    int height;
    float aspectRatio;
};

enum VideoLoadStatus
{
    VIDEO_LOAD_SUCCESS,
    VIDEO_LOAD_FILE_NOT_FOUND,
    VIDEO_LOAD_ERROR
};

enum VideoPlaybackStatus
{
    PLAYING,
    PAUSED
};

enum VideoSyncEvent
{
  TICK,
  PLAY_OR_PAUSE,
  PLAY,
  PAUSE,
  SYNC_ON,
  SYNC_OFF,
  PLAYBACK_COMPLETE,
  STEP_FORWARD_LARGE,
  STEP_BACKWARD_LARGE,
  STEP_FORWARD_SMALL,
  STEP_BACKWARD_SMALL,
  SET_VIDEO_TO_START,
  PLAYBACK_RATE_UPDATE,
  VIDEO_POSITION_UPDATE,    // Position updated by user dragging slider
  VIDEO_FRAME_UPDATE,       // Frame step by video channel during playback
  LOAD_VIDEO
};

const inline std::unordered_map<VideoSyncEvent, const char*> sync_event_name_map = {
    {VideoSyncEvent::TICK, "tick"},
    {VideoSyncEvent::PLAY_OR_PAUSE, "play_or_pause"},
    {VideoSyncEvent::PLAY, "play"},
    {VideoSyncEvent::PAUSE, "pause"},
    {VideoSyncEvent::SYNC_ON, "sync_on"},
    {VideoSyncEvent::SYNC_OFF, "sync_off"},
    {VideoSyncEvent::PLAYBACK_COMPLETE, "playback_complete"},
    {VideoSyncEvent::STEP_FORWARD_LARGE, "step_forward_large"},
    {VideoSyncEvent::STEP_BACKWARD_LARGE, "step_backward_large"},
    {VideoSyncEvent::STEP_FORWARD_SMALL, "step_forward_small"},
    {VideoSyncEvent::STEP_BACKWARD_SMALL, "step_backward_small"},
    {VideoSyncEvent::SET_VIDEO_TO_START, "set_video_to_start"},
    {VideoSyncEvent::PLAYBACK_RATE_UPDATE, "playback_rate_update"},
    {VideoSyncEvent::VIDEO_POSITION_UPDATE, "video_position_update"},
    {VideoSyncEvent::VIDEO_FRAME_UPDATE, "video_frame_update"},
    {VideoSyncEvent::LOAD_VIDEO, "load_video"}
};

enum WhichVideoChannel
{
  LEFT_CHANNEL,
  RIGHT_CHANNEL,
  BOTH_CHANNELS,
  NO_CHANNEL
};

const inline std::unordered_map<WhichVideoChannel, const char*> channel_name_map = {
    {WhichVideoChannel::LEFT_CHANNEL, "left"},
    {WhichVideoChannel::RIGHT_CHANNEL, "right"},
    {WhichVideoChannel::BOTH_CHANNELS, "both"},
    {WhichVideoChannel::NO_CHANNEL, "none"}
};

} // namespace VideoPlayback

#endif // VIDEOPLAYBACKTYPES_H