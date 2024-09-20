#ifndef ISYNCHRONSIZERSTATE_H
#define ISYNCHRONSIZERSTATE_H

#include <Synchronizer.h>
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
        ISynchronizerState(const std::string &name, Synchronizer* synchronizer) : 
            m_name(name), m_pSynchronizer(synchronizer), m_logger(Logger::get()) {}
        virtual ~ISynchronizerState() = default;

        virtual std::unique_ptr<ISynchronizerState> handleEvent(VideoSyncEvent event, WhichVideoChannel channel) = 0;

        std::string& getName()
        {
            return m_name;
        }

    private:
        std::string m_name;

    protected:
        Synchronizer *m_pSynchronizer;
        Logger &m_logger;
};

} // namespace VideoPlayback

#endif // ISYNCHRONSIZERSTATE_H
