#include <AnalyzerWindow.h>

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

    // Create the GUI component
    m_pGui = std::make_unique<AnalyzerGui>(pBuilder);
    
    // Create the controller (which will create the business logic layer)
    m_pController = std::make_shared<AnalyzerController>(m_pGui.get());
    
    // Wire them together
    m_pGui->setController(m_pController);
    
    // Initialize the controller to set up the business logic
    m_pController->initialize();

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
