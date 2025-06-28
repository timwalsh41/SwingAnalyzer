#ifndef GTKIMAGEUPDATER_H
#define GTKIMAGEUPDATER_H

#include <IGUIImageUpdater.h>
#include <Logger.h>

#include <gdkmm/pixbuf.h>
#include <goocanvasmm.h>

namespace VideoPlayback
{

class GTKImageUpdater : public IGUIImageUpdater {
public:
    GTKImageUpdater();
    ~GTKImageUpdater() override = default;
    
    void updateImage(const cv::Mat& frame) override;
    void setImageSize(int width, int height) override;
    void* getImageObject() override;
    
    // GTK-specific methods
    const Glib::RefPtr<Goocanvas::Image>& getGooCanvasImage() const;
    const Glib::RefPtr<Goocanvas::Image>& getImage() const; // Alias for compatibility

private:
    Glib::RefPtr<Gdk::Pixbuf> m_pixbuf;
    Glib::RefPtr<Goocanvas::Image> m_image;
    int m_target_width;
    int m_target_height;
    Logger& m_logger;
    
    void updatePixBuf(const cv::Mat& frame);
};

} // namespace VideoPlayback

#endif // GTKIMAGEUPDATER_H
