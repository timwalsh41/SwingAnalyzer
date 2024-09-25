#ifndef VIDEOPLAYBACKTYPES_H
#define VIDEOPLAYBACKTYPES_H

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
    VIDEO_LOAD_INVALID_TYPE,
    VIDEO_LOAD_UNKNOWN_ERROR
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
  VIDEO_POSITION_UPDATE,
  LOAD_VIDEO
};

const inline std::unordered_map<VideoSyncEvent, const char*> sync_event_name_map = {
    {VideoSyncEvent::TICK, "tick"},
    {VideoSyncEvent::PLAY_OR_PAUSE, "play_or_pause"},
    {VideoSyncEvent::PLAY, "play"},
    {VideoSyncEvent::PAUSE, "pause"},
    {VideoSyncEvent::SYNC_ON, "sync_on"},
    {VideoSyncEvent::SYNC_OFF, "sync_off"},
    {VideoSyncEvent::PLAYBACK_COMPLETE, "playback_complete"}
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