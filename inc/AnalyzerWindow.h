#ifndef ANALYZERWINDOW_H
#define ANALYZERWINDOW_H

#include <cairomm/context.h>
#include <glibmm/main.h>
#include <goocanvasmm.h>
#include <gtkmm.h>
#include <iostream>
#include <memory>
#include <opencv2/opencv.hpp>
#include <Analyzer.h>

struct WindowDecorationSize
{
    int title_bar_height;
    int border_width;
};

class AnalyzerWindow : public Gtk::Window
{
  public:
    AnalyzerWindow(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &pBuilder, const std::string &videoPath);

  private:
    // the GTK builder has references to all of the GUI components loaded from the Glade design
    Glib::RefPtr<Gtk::Builder> m_builder;

    std::unique_ptr<Analyzer> m_pAnalyzer;
        
    // window width and height are used for resizing the video
    int m_windowWidth{0}, m_windowHeight{0};

    // this function is used to get the size of the usable window area (not including the title bar)
    WindowDecorationSize getWindowDecorationSize();
};

#endif // ANALYZERWINDOW_H