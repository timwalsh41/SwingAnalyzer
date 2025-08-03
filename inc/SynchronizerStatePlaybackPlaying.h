#ifndef SYNCHRONIZERSTATEPLAYBACKPLAYING_H
#define SYNCHRONIZERSTATEPLAYBACKPLAYING_H

#include <ISynchronizerState.h>

namespace VideoPlayback
{   

class SynchronizerStatePlaybackPlaying : public ISynchronizerState
{
    public:
        SynchronizerStatePlaybackPlaying(ISynchronizerActions*);
        ~SynchronizerStatePlaybackPlaying() = default;

        std::unique_ptr<ISynchronizerState> handleEvent(VideoSyncEvent event, WhichVideoChannel channel) override;
};

} // namespace VideoPlayback

#endif // SYNCHRONIZERSTATEPLAYBACKPLAYING_H