#include <gtkmm.h>
#include <opencv2/opencv.hpp>
#include <cairomm/context.h>
#include <glibmm/main.h>
#include <iostream>
#include <memory>

struct window_decoration_size {
    int title_bar_height;
    int border_width;
};

class VideoWindow : public Gtk::Window {
public:
    VideoWindow(const std::string& videoPath);
    bool redraw_frame(const Cairo::RefPtr<Cairo::Context>& cr);

private:
    Gtk::Fixed m_fixedContainer;
    Gtk::DrawingArea m_drawingArea;
    cv::VideoCapture m_videoCapture;
    cv::Mat m_Frame;
    double m_Fps;

    int m_windowWidth{0}, m_windowHeight{0};
    float m_videoWidth{0}, m_videoHeight{0};
    float m_videoAspectRatio{0};

    window_decoration_size get_window_decoration_size();
};