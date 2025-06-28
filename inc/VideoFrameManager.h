#ifndef VIDEOFRAMEMANAGER_H
#define VIDEOFRAMEMANAGER_H

#include <Logger.h>

#include <algorithm>

namespace VideoPlayback
{

class VideoFrameManager {
public:
    VideoFrameManager();

    void setFrameRange(int start, int end, int total_frames);
    void setStartFrame(int start);
    void setEndFrame(int end);
    void setTotalFrames(int total);

    bool isValidFrame(int frame) const;
    int clampFrame(int frame) const;
    int getEffectiveLength() const;

    int getStartFrame() const { return m_start_frame; }
    int getEndFrame() const { return m_end_frame; }
    int getTotalFrames() const { return m_total_frames; }

    // Frame navigation
    int getNextFrame(int current_frame) const;
    int getPreviousFrame(int current_frame) const;
    int stepFrame(int current_frame, int offset) const;

private:
    int m_start_frame;
    int m_end_frame;
    int m_total_frames;
    Logger& m_logger;

    void validateFrameRange();
};

} // namespace VideoPlayback

#endif // VIDEOFRAMEMANAGER_H
