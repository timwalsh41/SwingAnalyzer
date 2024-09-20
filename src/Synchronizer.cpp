#include <Synchronizer.h>

namespace VideoPlayback
{
    Synchronizer::Synchronizer(VideoChannel *channel_left, VideoChannel *channel_right) : 
        m_pChannel_left(channel_left), m_pChannel_right(channel_right), m_logger(Logger::get())
    {

    }

} // namespace VideoPlayback