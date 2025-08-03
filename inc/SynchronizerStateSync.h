#ifndef SYNCHRONIZERSTATESYNC_H
#define SYNCHRONIZERSTATESYNC_H

#include <ISynchronizerState.h>

namespace VideoPlayback
{   

class SynchronizerStateSync : public ISynchronizerState
{
    public:
        SynchronizerStateSync(ISynchronizerActions*);
        ~SynchronizerStateSync() = default;

        std::unique_ptr<ISynchronizerState> handleEvent(VideoSyncEvent event, WhichVideoChannel channel) override;
};

} // namespace VideoPlayback

#endif // SYNCHRONIZERSTATESYNC_H