#include <VideoFrameManager.h>

namespace VideoPlayback
{

VideoFrameManager::VideoFrameManager() 
    : start_frame_(0), end_frame_(0), total_frames_(0), logger_(Logger::get()) {
}

void VideoFrameManager::setFrameRange(int start, int end, int total_frames) {
    total_frames_ = total_frames;
    start_frame_ = std::max(0, start);
    end_frame_ = (end == -1) ? total_frames_ - 1 : std::min(end, total_frames_ - 1);
    validateFrameRange();
}

void VideoFrameManager::setStartFrame(int start) {
    if (start < 0 || start >= end_frame_) {
        logger_.warn("Invalid start frame: %d (valid range: 0-%d)", start, end_frame_ - 1);
        return;
    }
    start_frame_ = start;
}

void VideoFrameManager::setEndFrame(int end) {
    if (end == -1) {
        end_frame_ = total_frames_ - 1;
    } else if (end <= start_frame_ || end >= total_frames_) {
        logger_.warn("Invalid end frame: %d (valid range: %d-%d)", 
                     end, start_frame_ + 1, total_frames_ - 1);
        return;
    } else {
        end_frame_ = end;
    }
}

void VideoFrameManager::setTotalFrames(int total) {
    if (total <= 0) {
        logger_.warn("Invalid total frames: %d", total);
        return;
    }
    
    total_frames_ = total;
    start_frame_ = 0;
    end_frame_ = total_frames_ - 1;
}

bool VideoFrameManager::isValidFrame(int frame) const {
    return frame >= start_frame_ && frame <= end_frame_;
}

int VideoFrameManager::clampFrame(int frame) const {
    return std::clamp(frame, start_frame_, end_frame_);
}

int VideoFrameManager::getEffectiveLength() const {
    return end_frame_ - start_frame_ + 1;
}

int VideoFrameManager::getNextFrame(int current_frame) const {
    if (current_frame >= end_frame_) {
        return end_frame_; // Already at end
    }
    return current_frame + 1;
}

int VideoFrameManager::getPreviousFrame(int current_frame) const {
    if (current_frame <= start_frame_) {
        return start_frame_; // Already at start
    }
    return current_frame - 1;
}

int VideoFrameManager::stepFrame(int current_frame, int offset) const {
    int new_frame = current_frame + offset;
    return clampFrame(new_frame);
}

void VideoFrameManager::validateFrameRange() {
    if (start_frame_ >= end_frame_) {
        logger_.warn("Invalid frame range: start=%d, end=%d. Resetting to full range.", 
                     start_frame_, end_frame_);
        start_frame_ = 0;
        end_frame_ = std::max(0, total_frames_ - 1);
    }
}

} // namespace VideoPlayback
