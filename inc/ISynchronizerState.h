#ifndef ISYNCHRONSIZERSTATE_H
#define ISYNCHRONSIZERSTATE_H

#include <ISynchronizerActions.h>
#include <Logger.h>
#include <VideoPlaybackTypes.h>

#include <memory>
#include <string>
#include <unordered_map>

namespace VideoPlayback
{

class ISynchronizerState
{   
    public:
        ISynchronizerState(const std::string &name, ISynchronizerActions* synchronizer) : 
            m_name(name), m_pSynchronizer(synchronizer), m_logger(Logger::get()) {}
        virtual ~ISynchronizerState() = default;

        virtual std::unique_ptr<ISynchronizerState> handleEvent(VideoSyncEvent event, WhichVideoChannel channel) = 0;

        const std::string& getName() const
        {
            return m_name;
        }

    private:
        std::string m_name;

    protected:
        ISynchronizerActions* m_pSynchronizer;
        Logger &m_logger;
};

} // namespace VideoPlayback

#endif // ISYNCHRONSIZERSTATE_H
