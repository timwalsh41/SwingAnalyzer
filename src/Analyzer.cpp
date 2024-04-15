#include <Analyzer.h>

Analyzer::Analyzer(const Glib::RefPtr<Gtk::Builder> &pBuilder) : 
    m_builder(pBuilder),
    m_logger{Logger::get()}
{
    m_logger.logString(INFO, "Initializing Analyzer");

    m_pSynchronizer = std::make_unique<Synchronizer>(&m_channel_left, &m_channel_right);

    m_canvas_left = Gtk::manage(new Goocanvas::Canvas());
    m_canvas_right = Gtk::manage(new Goocanvas::Canvas());

    Gtk::Box *left_video_box = nullptr;
    m_builder->get_widget("LeftVideoBox", left_video_box);
    if (left_video_box)
    {
        left_video_box->add(*m_canvas_left);
        left_video_box->reorder_child(*m_canvas_left, 0);
    }
    else
    {
        throw Gtk::BuilderError(Gtk::BuilderError::INVALID_ID, "Failed to find LeftVideoBox");
    }

    Gtk::Box *right_video_box = nullptr;
    m_builder->get_widget("RightVideoBox", right_video_box);
    if (right_video_box)
    {
        right_video_box->add(*m_canvas_right);
        right_video_box->reorder_child(*m_canvas_right, 0);
    }
    else
    {
        throw Gtk::BuilderError(Gtk::BuilderError::INVALID_ID, "Failed to find RightVideoBox");
    }

    m_canvas_left->get_root_item()->add_child(m_channel_left.get_image());
    m_canvas_right->get_root_item()->add_child(m_channel_right.get_image());

    // TODO: TEMPORARY TEST CODE
    auto rect = Goocanvas::Rect::create(250, 400, 100, 100);
    rect->property_stroke_color() = "red";
    rect->property_line_width() = 6;
    m_canvas_left->get_root_item()->add_child(rect);
    m_canvas_left->show();
    m_channel_left.loadVideo("/home/tim/Desktop/PXL_20240329_005944373.TS.mp4");
    auto video_size = m_channel_left.getVideoSize();
    m_canvas_left->set_size_request(video_size.width, video_size.height);
    m_channel_left.playVideo();
}

Analyzer::~Analyzer()
{
}

void Analyzer::load_left_video()
{
    
}