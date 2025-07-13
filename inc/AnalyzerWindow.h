#ifndef ANALYZERWINDOW_H
#define ANALYZERWINDOW_H

#include <AnalyzerController.h>
#include <AnalyzerGui.h>

#include <glibmm/main.h>
#include <gtkmm.h>

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

    std::shared_ptr<AnalyzerController> m_pController;
    std::unique_ptr<AnalyzerGui> m_pGui;

    // window width and height are used for resizing the video
    int m_windowWidth{0}, m_windowHeight{0};

    // this function is used to get the size of the usable window area (not including the title bar)
    const WindowDecorationSize getWindowDecorationSize();
};

#endif // ANALYZERWINDOW_H