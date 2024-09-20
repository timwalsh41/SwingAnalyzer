#ifndef SYNCHRONIZERSTATEPLAYBACKPAUSED_H
#define SYNCHRONIZERSTATEPLAYBACKPAUSED_H

#include <ISynchronizerState.h>

namespace VideoPlayback
{   

class SynchronizerStatePlaybackPaused : public ISynchronizerState
{
    public:
        SynchronizerStatePlaybackPaused(Synchronizer*);
        ~SynchronizerStatePlaybackPaused() = default;

        std::unique_ptr<ISynchronizerState> handleEvent(VideoSyncEvent event, WhichVideoChannel channel) override;
};

} // namespace VideoPlayback

#endif // SYNCHRONIZERSTATEPLAYBACKPAUSED_H