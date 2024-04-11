#include <AnalyzerWindow.h>
#include <Logger.h>

#include <thread>

/*
TODO:
[ ] Use clang-format to apply consistent style
[ ] Window sizing based on getting allocation is not working properly
*/

AnalyzerWindow::AnalyzerWindow(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &pBuilder,
                               const std::string &videoPath)
    : Gtk::Window(cobject), m_builder(pBuilder), m_videoCapture(videoPath), m_Fps(m_videoCapture.get(cv::CAP_PROP_FPS))
{
    Logger& logger = Logger::init("log.txt");
    logger.logString("AnalyzerWindow::AnalyzerWindow()");

    if (!m_videoCapture.isOpened())
    {
        throw std::runtime_error("Failed to open video file");
    }
    else
    {
        std::cout << "Loaded " << videoPath << '\n';
        std::cout << "FPS: " << m_Fps << '\n';
        std::cout << "Total frames: " << m_videoCapture.get(cv::CAP_PROP_FRAME_COUNT) << "\n";
    }

    m_windowWidth = 1600;
    m_windowHeight = 1200;

    set_default_size(m_windowWidth, m_windowHeight);
    set_title("Video drawing test");

    /*
    // call realize to force the window to instantiate
    realize();

    // window must be instantiated before getting decoration size
    auto [title_bar_height, border_width] = getWindowDecorationSize();
    std::cout << "Title bar height: " << title_bar_height << "\n";
    std::cout << "Border width: " << border_width << "\n";
    */

    m_canvas = Gtk::manage(new Goocanvas::Canvas());
    Gtk::Box *left_video_box = nullptr;
    m_builder->get_widget("LeftVideoBox", left_video_box);
    if (left_video_box)
    {
        left_video_box->add(*m_canvas);
        left_video_box->reorder_child(*m_canvas, 0);
    }
    else
    {
        throw Gtk::BuilderError(Gtk::BuilderError::INVALID_ID, "Failed to find LeftVideoBox");
    }

    // TODO: add placeholder blank image to canvas for right channel

    // initialize the canvas with a blank image
    m_pixbuf = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, false, 8, 1, 1);
    m_image = Goocanvas::Image::create(m_pixbuf, 0, 0);
    m_canvas->get_root_item()->add_child(m_image);

    // add a rectangle for test purpose
    auto rect = Goocanvas::Rect::create(250, 400, 100, 100);
    rect->property_stroke_color() = "red";
    rect->property_line_width() = 6;
    m_canvas->get_root_item()->add_child(rect);
    m_canvas->show();

    // our dispatcher will trigger the update of the canvas image pixbuf
    // the dispatcher is triggered by the worker thread
    m_dispatcher.connect([this]() {
        std::lock_guard<std::mutex> lock(m_pixBufMutex);
        m_image->property_pixbuf() = m_pixbuf;
        // m_canvas->queue_draw();
    });

    // load the first frame
    m_videoCapture >> m_Frame;

    // start our worker thread to process the video frames
    std::thread workerThread(&AnalyzerWindow::playVideoWorkerThread, this);
    workerThread.detach();

    show_all();
}

void AnalyzerWindow::playVideoWorkerThread()
{
    while (!m_Frame.empty())
    {
        // we shouldn't need this call here...
        setVideoSize();

        cv::Mat resized_frame;
        cv::resize(m_Frame, resized_frame, cv::Size(m_videoWidth, m_videoHeight));

        cv::Mat rgb_frame;
        cv::cvtColor(resized_frame, rgb_frame, cv::COLOR_BGR2RGB);

        {
            std::lock_guard<std::mutex> lock(m_pixBufMutex);
            m_pixbuf = Gdk::Pixbuf::create_from_data(rgb_frame.data, Gdk::COLORSPACE_RGB, false, 8, rgb_frame.cols,
                                                     rgb_frame.rows, rgb_frame.step);
        }

        m_dispatcher.emit();
        m_videoCapture >> m_Frame;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    exit(1);
}

void AnalyzerWindow::setVideoSize()
{
    if (!m_Frame.empty())
    {
        // set up video size - note this will not support window resizing
        m_videoAspectRatio = static_cast<double>(m_Frame.cols) / static_cast<double>(m_Frame.rows);

        Gtk::Box *left_button_box = nullptr;
        m_builder->get_widget("LeftVideoButtonBox", left_button_box);
        if (!left_button_box)
        {
            throw Gtk::BuilderError(Gtk::BuilderError::INVALID_ID, "Failed to find LeftVideoButtonBox");
        }

        Gtk::Box *left_video_box = nullptr;
        m_builder->get_widget("LeftVideoBox", left_video_box);
        if (!left_video_box)
        {
            throw Gtk::BuilderError(Gtk::BuilderError::INVALID_ID, "Failed to find LeftVideoBox");
        }

        Gtk::Allocation left_video_box_allocation = left_video_box->get_allocation();
        Gtk::Allocation left_button_box_allocation = left_button_box->get_allocation();

        int availableHeight = left_video_box_allocation.get_height() - left_button_box_allocation.get_height();
        int availableWidth = left_video_box_allocation.get_width();

        if (availableHeight == 0 || availableWidth == 0)
        {
            availableHeight = 0.8 * m_windowHeight;
            availableWidth = 0.8 * m_windowWidth;
        }

        if (availableWidth / m_videoAspectRatio <= availableHeight)
        {
            m_videoWidth = availableWidth;
            m_videoHeight = availableWidth / m_videoAspectRatio;
        }
        else
        {
            m_videoHeight = availableHeight;
            m_videoWidth = availableHeight * m_videoAspectRatio;
        }

        m_canvas->set_size_request(m_videoWidth, m_videoHeight);

        // std::cout << "Window size: " << m_windowWidth << "x" << m_windowHeight << "\n";
        // std::cout << "Video size: " << m_videoWidth << "x" << m_videoHeight << "\n";
        // std::cout << "Video aspect ratio: " << m_videoAspectRatio << "\n";
    }
}

void AnalyzerWindow::on_realize()
{
    Gtk::Window::on_realize();
    setVideoSize();
}

WindowDecorationSize AnalyzerWindow::getWindowDecorationSize()
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
