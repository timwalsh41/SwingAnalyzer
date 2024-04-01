#ifndef ANALYZERWINDOW_H
#define ANALYZERWINDOW_H

#include <gtkmm.h>
#include <opencv2/opencv.hpp>
#include <goocanvasmm.h>
#include <cairomm/context.h>
#include <glibmm/main.h>
#include <iostream>
#include <memory>

struct window_decoration_size {
    int title_bar_height;
    int border_width;
};

class AnalyzerWindow : public Gtk::Window {
public:
    AnalyzerWindow(const std::string& videoPath);
    bool redraw_frame(const Cairo::RefPtr<Cairo::Context>& cr);

private:
    Goocanvas::Canvas* m_canvas;
    Gtk::Fixed m_fixedContainer;
    Gtk::DrawingArea m_drawingArea;
    cv::VideoCapture m_videoCapture;
    cv::Mat m_Frame{};
    Glib::RefPtr<Goocanvas::Image> m_image;
    double m_Fps;
    Glib::RefPtr<Gdk::Pixbuf> m_pixbuf;

    Glib::Dispatcher m_dispatcher;
    std::mutex m_pixBufMutex;

    int m_windowWidth{0}, m_windowHeight{0};
    float m_videoWidth{0}, m_videoHeight{0};
    float m_videoAspectRatio{0};

    window_decoration_size get_window_decoration_size();
    void playVideoWorkerThread();
};

#endif // ANALYZERWINDOW_H