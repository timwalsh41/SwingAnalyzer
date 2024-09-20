#include <Analyzer.h>
#include <SyncManager.h>
#include <SynchronizerStateSync.h>

#include <memory>

namespace VideoPlayback
{
    SyncManager::SyncManager(Analyzer *analyzer, VideoChannel *channel_left, VideoChannel *channel_right) :
        m_pAnalyzer(analyzer), m_synchronizer(channel_left, channel_right), m_logger(Logger::get())
    {
        m_state = std::make_unique<SynchronizerStateSync>(&m_synchronizer);

        channel_left->registerVideoPlaybackCompleteCallback([this](std::string& source_channel){this->videoPlaybackCompleteCallback(source_channel);});
        channel_right->registerVideoPlaybackCompleteCallback([this](std::string& source_channel){this->videoPlaybackCompleteCallback(source_channel);});

        m_video_complete_dispatcher.connect([this]{handleEvent(PLAYBACK_COMPLETE, m_playback_complete_channel.load());});
    }

    void SyncManager::handleEvent(VideoSyncEvent event, WhichVideoChannel channel)
    {
        std::unique_ptr<ISynchronizerState> next_state = m_state->handleEvent(event, channel);
        
        while (next_state)
        {
            m_state = std::move(next_state);

            // immediately tick the new state, as some states can transition directly out
            next_state = m_state->handleEvent(TICK, NO_CHANNEL);
        }
    }

    // Thread-safe playback complete callback. This can be invoked by either video channel, so we use an atomic
    // to capture the source channel and then emit a signal via our dispatcher to then handle the event in the main
    // thread.
    void SyncManager::videoPlaybackCompleteCallback(std::string& source_channel)
    {
        m_playback_complete_channel.store((source_channel == "left") ? LEFT_CHANNEL : RIGHT_CHANNEL);
        m_video_complete_dispatcher.emit();
    }

} // namespace VideoPlayback