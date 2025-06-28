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
    
    int getStartFrame() const { return start_frame_; }
    int getEndFrame() const { return end_frame_; }
    int getTotalFrames() const { return total_frames_; }
    
    // Frame navigation
    int getNextFrame(int current_frame) const;
    int getPreviousFrame(int current_frame) const;
    int stepFrame(int current_frame, int offset) const;

private:
    int start_frame_;
    int end_frame_;
    int total_frames_;
    Logger& logger_;
    
    void validateFrameRange();
};

} // namespace VideoPlayback

#endif // VIDEOFRAMEMANAGER_H
