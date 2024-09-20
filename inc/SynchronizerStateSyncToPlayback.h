#ifndef SYNCHRONIZERSTATESYNCTOPLAYBACK_H
#define SYNCHRONIZERSTATESYNCTOPLAYBACK_H

#include <ISynchronizerState.h>

namespace VideoPlayback
{   

class SynchronizerStateSyncToPlayback : public ISynchronizerState
{
    public:
        SynchronizerStateSyncToPlayback(Synchronizer*);
        ~SynchronizerStateSyncToPlayback() = default;

        std::unique_ptr<ISynchronizerState> handleEvent(VideoSyncEvent event, WhichVideoChannel channel) override;
};

} // namespace VideoPlayback

#endif // SYNCHRONIZERSTATESYNCTOPLAYBACK_H