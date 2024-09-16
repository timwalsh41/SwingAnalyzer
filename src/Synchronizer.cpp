#include <Synchronizer.h>
#include <Analyzer.h>
namespace VideoPlayback
{
    Synchronizer::Synchronizer(Analyzer *analyzer, VideoChannel *channel_left, VideoChannel *channel_right)
        : m_pAnalyzer(analyzer), m_pChannel_left(channel_left), m_pChannel_right(channel_right),  m_logger{Logger::get()}
    {

    }

    void Synchronizer::setSyncMode(bool syncMode)
    {

    }

    void Synchronizer::playOrPauseVideo()
    {
        if (m_syncMode == SYNC)
        {
            bool isVideoPlaying = m_pChannel_right->getPlaybackStatus() == PLAYING || 
                                  m_pChannel_left->getPlaybackStatus() == PLAYING;

            if (isVideoPlaying)
            {
                m_pChannel_left->pauseVideo();
                m_pChannel_right->pauseVideo();
            }
            else
            {
                m_pChannel_left->playVideo();
                m_pChannel_right->playVideo();
            }
        }
        else if (m_syncMode == PLAYBACK)
        {

        }
        else
        {
            m_logger.error("Invalid sync mode %d in Synchronizer::playOrPauseVideo()", m_syncMode);
        }
    }

} // namespace VideoPlayback