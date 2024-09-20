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

        // flag for continuous playback mode, as selected by user
        bool m_continuous_playback{false};
};

} // namespace VideoPlayback

#endif // SYNCHRONIZERIMPL_H