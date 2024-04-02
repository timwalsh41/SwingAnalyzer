#include <AnalyzerWindow.h>
#include <thread>

/* 
TODO:
[ ] Use clang-format to apply consistent style
*/


AnalyzerWindow::AnalyzerWindow(const std::string& videoPath)
     : m_videoCapture(videoPath), m_Fps(m_videoCapture.get(cv::CAP_PROP_FPS))
{
    if (!m_videoCapture.isOpened()) {
        throw std::runtime_error("Failed to open video file");
    }
    else
    {
        std::cout << "Loaded " << videoPath << '\n';
        std::cout << "FPS: " << m_Fps << '\n';
        std::cout << "Total frames: " << m_videoCapture.get(cv::CAP_PROP_FRAME_COUNT) << "\n";
    }
    
    m_windowWidth = 1200;
    m_windowHeight = 800;

    set_default_size(m_windowWidth, m_windowHeight);
    set_title("Video drawing test");
    
    /*
    // call realize to force the window to instantiate
    realize();

    // window must be instantiated before getting decoration size
    auto [title_bar_height, border_width] = get_window_decoration_size();
    std::cout << "Title bar height: " << title_bar_height << "\n";
    std::cout << "Border width: " << border_width << "\n";
    */

    m_canvas = Gtk::manage(new Goocanvas::Canvas());
    add(*m_canvas);

    // initialize the canvas with a blank image
    m_pixbuf = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, false, 8, 1, 1);
    m_image = Goocanvas::Image::create(m_pixbuf, 0, 0);
    m_canvas->get_root_item()->add_child(m_image);

    // add a rectangle for test purpose
    auto rect = Goocanvas::Rect::create(250,400,100,100);
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
    SetVideoSize();

    // start our worker thread to process the video frames
    std::thread workerThread(&AnalyzerWindow::playVideoWorkerThread, this);
    workerThread.detach();

    show_all();
}

void AnalyzerWindow::playVideoWorkerThread()
{
    while (!m_Frame.empty())
    {
        cv::Mat resized_frame;
        cv::resize(m_Frame, resized_frame, cv::Size(m_videoWidth, m_videoHeight));

        cv::Mat rgbFrame;
        cv::cvtColor(resized_frame, rgbFrame, cv::COLOR_BGR2RGB);

        {
            std::lock_guard<std::mutex> lock(m_pixBufMutex);
            m_pixbuf = Gdk::Pixbuf::create_from_data(
                rgbFrame.data, Gdk::COLORSPACE_RGB, false, 8, rgbFrame.cols, rgbFrame.rows, rgbFrame.step
            );
        }

        m_dispatcher.emit();
        m_videoCapture >> m_Frame;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    exit(1);
}

void AnalyzerWindow::SetVideoSize()
{
    if (!m_Frame.empty()) 
    {
        if (m_videoAspectRatio == 0) 
        {
            // set up video size - note this will not support window resizing
            m_videoAspectRatio = static_cast<double>(m_Frame.cols) /
                static_cast<double>(m_Frame.rows);

            int padding = 200;

            if (m_windowWidth / m_videoAspectRatio <= m_windowHeight) 
            {
                m_videoWidth = m_windowWidth-padding;
                m_videoHeight = (m_windowWidth-padding) / m_videoAspectRatio;
            } 
            else 
            {
                m_videoWidth = (m_windowHeight-padding) * m_videoAspectRatio;
                m_videoHeight = m_windowHeight-padding;
            }

            m_canvas->set_size_request(m_videoWidth, m_videoHeight);

            std::cout << "Window size: " << m_windowWidth << "x" << m_windowHeight << "\n";
            std::cout << "Video size: " << m_videoWidth << "x" << m_videoHeight << "\n";
            std::cout << "Video aspect ratio: " << m_videoAspectRatio << "\n";
        }
    }
}

window_decoration_size AnalyzerWindow::get_window_decoration_size()
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
