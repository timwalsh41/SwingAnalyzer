#include <AnalyzerWindow.h>
#include <thread>

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

    // TEMPORARY CODE
    /*
    m_videoCapture >> m_Frame;
    cv::Mat rgbFrame;
    cv::cvtColor(m_Frame, rgbFrame, cv::COLOR_BGR2RGB);

    Glib::RefPtr<Gdk::Pixbuf> pixbuf = Gdk::Pixbuf::create_from_data(
        rgbFrame.data, Gdk::COLORSPACE_RGB, false, 8, rgbFrame.cols, rgbFrame.rows, rgbFrame.step
    );

    // Glib::RefPtr<Gdk::Pixbuf> pixbuf = Gdk::Pixbuf::create_from_file("/home/tim/Pictures/Pixel/IMG_20181022_162117.jpg");
    */
    Glib::RefPtr<Gdk::Pixbuf> pixbuf = Gdk::Pixbuf::create_from_file("/home/tim/Pictures/Pixel/IMG_20181022_162117.jpg");
    m_image = Goocanvas::Image::create(pixbuf, 0, 0);
    m_canvas->get_root_item()->add_child(m_image);
    
    // m_fixedContainer.put(m_drawingArea, 0, 0);
    // m_fixedContainer.put(*m_canvas, 0, 0);
    // add(m_fixedContainer);
    // m_fixedContainer.show();
    // m_drawingArea.show();
    m_canvas->show();

    m_videoCapture >> m_Frame;

    m_dispatcher.connect([this]() {
        std::lock_guard<std::mutex> lock(m_pixBufMutex);
        m_image->property_pixbuf() = m_pixbuf;
        // m_canvas->queue_draw();
    });

    std::thread workerThread(&AnalyzerWindow::playVideoWorkerThread, this);
    workerThread.detach();

    // m_drawingArea.signal_draw().connect(sigc::mem_fun(*this, &AnalyzerWindow::redraw_frame));
    /*m_canvas->signal_draw().connect(sigc::mem_fun(*this, &AnalyzerWindow::redraw_frame));

    Glib::signal_timeout().connect([this]() -> bool {
        // std::cout << "Capturing frame " << m_videoCapture.get(cv::CAP_PROP_POS_FRAMES) << "\n";
        m_videoCapture >> m_Frame;

        if (m_Frame.empty()) {
            std::cout << "Reached end of video\n";
            exit(1);
            return false; // Stop the timer
        }
        // m_drawingArea.queue_draw();
        m_canvas->queue_draw();
        return true;
    }, 1000 / m_Fps);
    */
}

void AnalyzerWindow::playVideoWorkerThread()
{
    // TODO: use a dispatcher to do the actual image pixbuf update, will need to store pixbuf in a member
    // variable, and lock with a mutex in both threads
    // 
    while (!m_Frame.empty())
    {
        cv::Mat rgbFrame;
        cv::cvtColor(m_Frame, rgbFrame, cv::COLOR_BGR2RGB);

        {
            std::lock_guard<std::mutex> lock(m_pixBufMutex);
            m_pixbuf = Gdk::Pixbuf::create_from_data(
                rgbFrame.data, Gdk::COLORSPACE_RGB, false, 8, rgbFrame.cols, rgbFrame.rows, rgbFrame.step
            );
        }

        m_dispatcher.emit();
        m_videoCapture >> m_Frame;
        std::this_thread::sleep_for(std::chrono::milliseconds(400));
    }
}

bool AnalyzerWindow::redraw_frame(const Cairo::RefPtr<Cairo::Context>& cr) 
{
    Gtk::Widget::on_draw(cr);

    if (!m_Frame.empty()) 
    {
        if (m_videoAspectRatio == 0) {
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

            // m_drawingArea.set_size_request(m_videoWidth, m_videoHeight);
            m_canvas->set_size_request(m_videoWidth, m_videoHeight);

            std::cout << "Window size: " << m_windowWidth << "x" << m_windowHeight << "\n";
            std::cout << "Video size: " << m_videoWidth << "x" << m_videoHeight << "\n";
            std::cout << "Video aspect ratio: " << m_videoAspectRatio << "\n";
        }

        /*
        cv::Mat resized_frame;
        cv::resize(m_Frame, resized_frame, cv::Size(m_videoWidth, m_videoHeight));
        
        cv::Mat rgbFrame;
        cv::cvtColor(resized_frame, rgbFrame, cv::COLOR_BGR2RGB);

        Glib::RefPtr<Gdk::Pixbuf> pixbuf = Gdk::Pixbuf::create_from_data(
            m_Frame.data, Gdk::COLORSPACE_RGB, false, 8, rgbFrame.cols, rgbFrame.rows, rgbFrame.step
        );

        auto image = Goocanvas::Image::create(pixbuf, 100, 100);
        */

        cv::Mat resized_frame;
        cv::resize(m_Frame, resized_frame, cv::Size(m_videoWidth, m_videoHeight));

        cv::Mat rgbFrame;
        cv::cvtColor(resized_frame, rgbFrame, cv::COLOR_BGR2RGB);

        Glib::RefPtr<Gdk::Pixbuf> pixbuf = Gdk::Pixbuf::create_from_data(
            rgbFrame.data, Gdk::COLORSPACE_RGB, false, 8, rgbFrame.cols, rgbFrame.rows, rgbFrame.step
        );

        m_image->property_pixbuf() = pixbuf;
        
        // Glib::RefPtr<Gdk::Pixbuf> pixbuf = Gdk::Pixbuf::create_from_file("/home/tim/Pictures/Pixel/IMG_20181022_162117.jpg");
        // auto image = Goocanvas::Image::create(pixbuf, -0, -200);
        // m_canvas->get_root_item()->add_child(image);

        /*while (m_canvas->get_root_item()->get_n_children() > 0)
        {
            m_canvas->get_root_item()->remove_child(0);
        }
        m_canvas->get_root_item()->add_child(image);
        std::cout << "Window has " << m_canvas->get_root_item()->get_n_children() << " children\n";
        */
        
        /*
        Gdk::Cairo::set_source_pixbuf(cr, pixbuf, 0, 0);
        cr->paint();

        // Draw a red line
        cr->set_source_rgb(1.0, 0.0, 0.0);
        cr->move_to(200, 200);
        cr->line_to(300, 300);
        cr->stroke();

        // Draw a blue ellipse
        cr->set_source_rgb(0.0, 0.0, 1.0);
        cr->save(); // save the current transformation matrix
        cr->translate(250, 250);
        cr->scale(1.0, 0.7);
        cr->arc(0.0, 0.0, 50.0, 0.0, 2.0 * M_PI);
        cr->restore(); // restore the transformation matrix
        cr->stroke();
        */
    }


    return false;
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
