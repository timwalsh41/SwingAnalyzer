#include <AnalyzerWindow.h>

#include <thread>

/*
TODO:
[ ] Window sizing based on getting allocation is not working properly
*/

AnalyzerWindow::AnalyzerWindow(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &pBuilder,
                               const std::string &videoPath)
    : Gtk::Window(cobject), m_builder(pBuilder)
{
    m_windowWidth = 1600;
    m_windowHeight = 1200;

    set_default_size(m_windowWidth, m_windowHeight);
    set_title("Swing Analyzer");

    m_pAnalyzer = std::make_unique<Analyzer>(pBuilder);

    show_all();
}

const WindowDecorationSize AnalyzerWindow::getWindowDecorationSize()
{
    Gdk::Rectangle frame_extents;

    if (get_window())
    {
        get_window()->get_frame_extents(frame_extents);

        int content_width, content_height;
        get_size(content_width, content_height);

        return {frame_extents.get_height() - content_height, (frame_extents.get_width() - content_width) / 2};
    }

    return {0, 0};
}
