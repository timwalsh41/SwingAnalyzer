#include <GTKImageUpdater.h>

namespace VideoPlayback
{

GTKImageUpdater::GTKImageUpdater() 
    : m_target_width(100), m_target_height(100), m_logger(Logger::get()) {
    m_pixbuf = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, false, 8, m_target_width, m_target_height);
    m_image = Goocanvas::Image::create(m_pixbuf, 0, 0);
}

void GTKImageUpdater::updateImage(const cv::Mat& frame) {
    if (frame.empty()) {
        m_logger.warn("Attempted to update image with empty frame");
        return;
    }
    
    updatePixBuf(frame);
}

void GTKImageUpdater::setImageSize(int width, int height) {
    if (width < 10 || height < 10) {
        m_logger.error("Invalid image size: %dx%d", width, height);
        return;
    }
    
    m_target_width = width;
    m_target_height = height;
    m_logger.debug("Image size set to %dx%d", width, height);
}

void* GTKImageUpdater::getImageObject() {
    return static_cast<void*>(m_image.operator->());
}

const Glib::RefPtr<Goocanvas::Image>& GTKImageUpdater::getGooCanvasImage() const {
    return m_image;
}

const Glib::RefPtr<Goocanvas::Image>& GTKImageUpdater::getImage() const {
    return m_image;
}

void GTKImageUpdater::updatePixBuf(const cv::Mat& frame) {
    try {
        // Resize frame to target size
        cv::Mat resized_frame;
        cv::resize(frame, resized_frame, cv::Size(m_target_width, m_target_height));
        
        // Convert BGR to RGB
        cv::Mat rgb_frame;
        cv::cvtColor(resized_frame, rgb_frame, cv::COLOR_BGR2RGB);
        
        // Create pixbuf from RGB data
        m_pixbuf = Gdk::Pixbuf::create_from_data(
            rgb_frame.data, 
            Gdk::COLORSPACE_RGB, 
            false, 
            8, 
            rgb_frame.cols,
            rgb_frame.rows, 
            rgb_frame.step
        );
        
        // Update the image
        m_image->property_pixbuf() = m_pixbuf;
        
    } catch (const std::exception& e) {
        m_logger.error("Error updating pixbuf: %s", e.what());
    }
}

} // namespace VideoPlayback
