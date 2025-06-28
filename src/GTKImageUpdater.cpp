#include <GTKImageUpdater.h>

namespace VideoPlayback
{

GTKImageUpdater::GTKImageUpdater() 
    : target_width_(100), target_height_(100), logger_(Logger::get()) {
    pixbuf_ = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, false, 8, target_width_, target_height_);
    image_ = Goocanvas::Image::create(pixbuf_, 0, 0);
}

void GTKImageUpdater::updateImage(const cv::Mat& frame) {
    if (frame.empty()) {
        logger_.warn("Attempted to update image with empty frame");
        return;
    }
    
    updatePixBuf(frame);
}

void GTKImageUpdater::setImageSize(int width, int height) {
    if (width < 10 || height < 10) {
        logger_.error("Invalid image size: %dx%d", width, height);
        return;
    }
    
    target_width_ = width;
    target_height_ = height;
    logger_.debug("Image size set to %dx%d", width, height);
}

void* GTKImageUpdater::getImageObject() {
    return static_cast<void*>(image_.operator->());
}

const Glib::RefPtr<Goocanvas::Image>& GTKImageUpdater::getGooCanvasImage() const {
    return image_;
}

const Glib::RefPtr<Goocanvas::Image>& GTKImageUpdater::getImage() const {
    return image_;
}

void GTKImageUpdater::updatePixBuf(const cv::Mat& frame) {
    try {
        // Resize frame to target size
        cv::Mat resized_frame;
        cv::resize(frame, resized_frame, cv::Size(target_width_, target_height_));
        
        // Convert BGR to RGB
        cv::Mat rgb_frame;
        cv::cvtColor(resized_frame, rgb_frame, cv::COLOR_BGR2RGB);
        
        // Create pixbuf from RGB data
        pixbuf_ = Gdk::Pixbuf::create_from_data(
            rgb_frame.data, 
            Gdk::COLORSPACE_RGB, 
            false, 
            8, 
            rgb_frame.cols,
            rgb_frame.rows, 
            rgb_frame.step
        );
        
        // Update the image
        image_->property_pixbuf() = pixbuf_;
        
    } catch (const std::exception& e) {
        logger_.error("Error updating pixbuf: %s", e.what());
    }
}

} // namespace VideoPlayback
