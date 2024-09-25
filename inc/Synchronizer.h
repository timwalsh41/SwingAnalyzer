#ifndef SYNCHRONIZER_H
#define SYNCHRONIZER_H

#include <VideoChannel.h>
#include <Logger.h>

namespace VideoPlayback
{

class Synchronizer
{
    friend class SyncManager;
    friend class SynchronizerStateSyncToPlayback;
    friend class SynchronizerStateSync;
    friend class SynchronizerStatePlaybackPlaying;
    friend class SynchronizerStatePlaybackPaused;

    public:
        Synchronizer(VideoChannel *channel_left, VideoChannel *channel_right);
        ~Synchronizer() = default;

    private:
        VideoChannel *m_pChannel_left, *m_pChannel_right;
        Logger &m_logger;

        // synchronized video offset (left channel position - right channel position)
        int m_video_frame_offset{0};

        // framerate ratio, left video framerate divided by right video framerate
        float m_framerate_ratio{1.0};

        // flag for continuous playback mode, as selected by user
        bool m_continuous_playback{false};

        // set both videos to the start position, e.g. the start frame for each, with
        // the offset applied
        void setVideosToStartPosition();

        // set the offset between the two video channels based on the start frame for
        // each and the current active frame
        void setVideoChannelOffset();

        // calculate the framerate ratio for the two channels
        void calculateFramerateRatio();

        // step both videos by the given number of frames, with the step size adjusted
        // for differences in frame rate; video channel itself protects against stepping
        // out of bounds
        void stepVideos(int step_size);

        void pauseVideos();
};

} // namespace VideoPlayback

#endif // SYNCHRONIZERIMPL_H