#ifndef IGUIIMAGEUPDATER_H
#define IGUIIMAGEUPDATER_H

#include <opencv2/opencv.hpp>

namespace VideoPlayback
{

class IGUIImageUpdater {
public:
    virtual ~IGUIImageUpdater() = default;
    
    virtual void updateImage(const cv::Mat& frame) = 0;
    virtual void setImageSize(int width, int height) = 0;
    virtual void* getImageObject() = 0; // Returns the GUI-specific image object
};

} // namespace VideoPlayback

#endif // IGUIIMAGEUPDATER_H
