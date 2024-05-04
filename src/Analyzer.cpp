#include <Analyzer.h>

Analyzer::Analyzer(const Glib::RefPtr<Gtk::Builder> &pBuilder) : m_builder(pBuilder), m_logger{Logger::get()}
{
    m_logger.info("Initializing Analyzer");

    m_pSynchronizer = std::make_unique<VideoPlayback::Synchronizer>(&m_channel_left, &m_channel_right);

    loadGuiObjects();
    setUpGui();
    connectSignals();

    // by default we'll start in sync mode
    syncModeToggle(true);
    m_guiObjects.chkSyncMode->set_active(true);

    // TODO: TEMPORARY TEST CODE TO ADD AN ANNOTATION ON THE LEFT CANVAS
    auto rect = Goocanvas::Rect::create(250, 400, 100, 100);
    rect->property_stroke_color() = "red";
    rect->property_line_width() = 6;
    m_canvas_left->get_root_item()->add_child(rect);
    m_canvas_left->show();
    m_channel_left.loadVideo("/home/tim/Desktop/PXL_20240329_005944373.TS.mp4");
    auto video_size = m_channel_left.getVideoSize();
    m_canvas_left->set_size_request(video_size.width, video_size.height);
    m_channel_left.playOrPauseVideo();
}

Analyzer::~Analyzer()
{
}

void Analyzer::loadLeftVideo()
{
}

void Analyzer::loadRightVideo()
{
}

void Analyzer::updateLeftVideoPosition(float video_position)
{
    if (video_position < 0 || video_position > 1)
    {
        m_logger.error("Invalid video position for left channel: %f", video_position);
        return;
    }

    // we'll block the scrollbar update while we update the scrollbar position
    m_left_channel_scrollbar_connection.block();
    m_guiObjects.scrollVideoLeft->set_value(video_position);
    m_left_channel_scrollbar_connection.unblock();
}

void Analyzer::updateRightVideoPosition(float video_position)
{
    if (video_position < 0 || video_position > 1)
    {
        m_logger.error("Invalid video position for right channel: %f", video_position);
        return;
    }

    // we'll block the scrollbar update while we update the scrollbar position
    m_right_channel_scrollbar_connection.block();
    m_guiObjects.scrollVideoRight->set_value(video_position);
    m_right_channel_scrollbar_connection.unblock();
}

// When in sync mode, we disable the joint controls and enable the individual video
// controls, and vice versa when in playback mode
void Analyzer::syncModeToggle(bool syncMode)
{
    m_logger.info("Enabling %s controls", syncMode ? "sync" : "playback");

    m_pSynchronizer->setSyncMode(syncMode);

    m_guiObjects.btnGoToStart->set_sensitive(!syncMode);
    m_guiObjects.btnBack5Frames->set_sensitive(!syncMode);
    m_guiObjects.btnBack1Frame->set_sensitive(!syncMode);
    m_guiObjects.btnForward1Frame->set_sensitive(!syncMode);
    m_guiObjects.btnForward5Frames->set_sensitive(!syncMode);
    m_guiObjects.scrollVideo->set_sensitive(!syncMode);
    m_guiObjects.comboPlaybackRate->set_sensitive(!syncMode);

    m_guiObjects.btnLoadLeftVideo->set_sensitive(syncMode);
    m_guiObjects.btnUnloadLeftVideo->set_sensitive(syncMode);
    m_guiObjects.btnLoadRightVideo->set_sensitive(syncMode);
    m_guiObjects.btnUnloadRightVideo->set_sensitive(syncMode);
    m_guiObjects.lblVideoLeft->set_sensitive(syncMode);
    m_guiObjects.lblVideoRight->set_sensitive(syncMode);
}

void Analyzer::connectSignals()
{

    m_guiObjects.btnPlay->signal_clicked().connect([this]() {
        this->m_channel_left.playOrPauseVideo();
        this->m_channel_right.playOrPauseVideo();
    });

    m_guiObjects.btnBack5FramesLeft->signal_clicked().connect([this]() { this->m_channel_left.stepFrame(-5); });
    m_guiObjects.btnBack1FrameLeft->signal_clicked().connect([this]() { this->m_channel_left.stepFrame(-1); });
    m_guiObjects.btnForward1FrameLeft->signal_clicked().connect([this]() { this->m_channel_left.stepFrame(1); });
    m_guiObjects.btnForward5FramesLeft->signal_clicked().connect([this]() { this->m_channel_left.stepFrame(5); });

    m_guiObjects.chkSyncMode->signal_toggled().connect(
        [this]() { this->syncModeToggle(this->m_guiObjects.chkSyncMode->get_active()); });

    m_left_channel_scrollbar_connection = m_guiObjects.scrollVideoLeft->signal_value_changed().connect([this]() {
        this->m_channel_left.setVideoPosition(this->m_guiObjects.scrollVideoLeft->get_value());
    });
}

void Analyzer::setUpGui()
{
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
        m_logger.fatal("Failed to find LeftVideoBox");
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
        m_logger.fatal("Failed to find RightVideoBox");
        throw Gtk::BuilderError(Gtk::BuilderError::INVALID_ID, "Failed to find RightVideoBox");
    }

    m_canvas_left->get_root_item()->add_child(m_channel_left.get_image());
    m_canvas_right->get_root_item()->add_child(m_channel_right.get_image());

    auto left_adjustment = Gtk::Adjustment::create(0., 0., 1., 0.025, 0.1, 0.);
    m_guiObjects.scrollVideoLeft->set_adjustment(left_adjustment);

    auto right_adjustment = Gtk::Adjustment::create(0., 0., 1., 0.025, 0.1, 0.);
    m_guiObjects.scrollVideoLeft->set_adjustment(right_adjustment);
}

void Analyzer::loadGuiObjects()
{
    m_builder->get_widget("BtnBack5FramesLeft", m_guiObjects.btnBack5FramesLeft);
    if (!m_guiObjects.btnBack5FramesLeft)
    {
        m_logger.fatal("Failed to find BtnBack5FramesLeft");
        throw Gtk::BuilderError(Gtk::BuilderError::INVALID_ID, "Failed to find BtnBack5FramesLeft");
    }

    m_builder->get_widget("BtnBack5FramesRight", m_guiObjects.btnBack5FramesRight);
    if (!m_guiObjects.btnBack5FramesRight)
    {
        m_logger.fatal("Failed to find BtnBack5FramesRight");
        throw Gtk::BuilderError(Gtk::BuilderError::INVALID_ID, "Failed to find BtnBack5FramesRight");
    }

    m_builder->get_widget("BtnBack1FrameLeft", m_guiObjects.btnBack1FrameLeft);
    if (!m_guiObjects.btnBack1FrameLeft)
    {
        m_logger.fatal("Failed to find BtnBack1FrameLeft");
        throw Gtk::BuilderError(Gtk::BuilderError::INVALID_ID, "Failed to find BtnBack1FrameLeft");
    }

    m_builder->get_widget("BtnBack1FrameRight", m_guiObjects.btnBack1FrameRight);
    if (!m_guiObjects.btnBack1FrameRight)
    {
        m_logger.fatal("Failed to find BtnBack1FrameRight");
        throw Gtk::BuilderError(Gtk::BuilderError::INVALID_ID, "Failed to find BtnBack1FrameRight");
    }

    m_builder->get_widget("BtnForward1FrameLeft", m_guiObjects.btnForward1FrameLeft);
    if (!m_guiObjects.btnForward1FrameLeft)
    {
        m_logger.fatal("Failed to find BtnForward1FrameLeft");
        throw Gtk::BuilderError(Gtk::BuilderError::INVALID_ID, "Failed to find BtnForward1FrameLeft");
    }

    m_builder->get_widget("BtnForward1FrameRight", m_guiObjects.btnForward1FrameRight);
    if (!m_guiObjects.btnForward1FrameRight)
    {
        m_logger.fatal("Failed to find BtnForward1FrameRight");
        throw Gtk::BuilderError(Gtk::BuilderError::INVALID_ID, "Failed to find BtnForward1FrameRight");
    }

    m_builder->get_widget("BtnForward5FramesLeft", m_guiObjects.btnForward5FramesLeft);
    if (!m_guiObjects.btnForward5FramesLeft)
    {
        m_logger.fatal("Failed to find BtnForward5FramesLeft");
        throw Gtk::BuilderError(Gtk::BuilderError::INVALID_ID, "Failed to find BtnForward5FramesLeft");
    }

    m_builder->get_widget("BtnForward5FramesRight", m_guiObjects.btnForward5FramesRight);
    if (!m_guiObjects.btnForward5FramesRight)
    {
        m_logger.fatal("Failed to find BtnForward5FramesRight");
        throw Gtk::BuilderError(Gtk::BuilderError::INVALID_ID, "Failed to find BtnForward5FramesRight");
    }

    m_builder->get_widget("ScrollVideoLeft", m_guiObjects.scrollVideoLeft);
    if (!m_guiObjects.scrollVideoLeft)
    {
        m_logger.fatal("Failed to find ScrollVideoLeft");
        throw Gtk::BuilderError(Gtk::BuilderError::INVALID_ID, "Failed to find ScrollVideoLeft");
    }

    m_builder->get_widget("ScrollVideoRight", m_guiObjects.scrollVideoRight);
    if (!m_guiObjects.scrollVideoRight)
    {
        m_logger.fatal("Failed to find ScrollVideoRight");
        throw Gtk::BuilderError(Gtk::BuilderError::INVALID_ID, "Failed to find ScrollVideoRight");
    }

    m_builder->get_widget("BtnLoadLeftVideo", m_guiObjects.btnLoadLeftVideo);
    if (!m_guiObjects.btnLoadLeftVideo)
    {
        m_logger.fatal("Failed to find BtnLoadLeftVideo");
        throw Gtk::BuilderError(Gtk::BuilderError::INVALID_ID, "Failed to find BtnLoadLeft");
    }

    m_builder->get_widget("BtnLoadRightVideo", m_guiObjects.btnLoadRightVideo);
    if (!m_guiObjects.btnLoadRightVideo)
    {
        m_logger.fatal("Failed to find BtnLoadRightVideo");
        throw Gtk::BuilderError(Gtk::BuilderError::INVALID_ID, "Failed to find BtnLoadRight");
    }

    m_builder->get_widget("BtnUnloadLeftVideo", m_guiObjects.btnUnloadLeftVideo);
    if (!m_guiObjects.btnUnloadLeftVideo)
    {
        m_logger.fatal("Failed to find BtnUnloadLeftVideo");
        throw Gtk::BuilderError(Gtk::BuilderError::INVALID_ID, "Failed to find BtnUnloadLeft");
    }

    m_builder->get_widget("BtnUnloadRightVideo", m_guiObjects.btnUnloadRightVideo);
    if (!m_guiObjects.btnUnloadRightVideo)
    {
        m_logger.fatal("Failed to find BtnUnloadRightVideo");
        throw Gtk::BuilderError(Gtk::BuilderError::INVALID_ID, "Failed to find BtnUnloadRight");
    }

    m_builder->get_widget("LblVideoLeft", m_guiObjects.lblVideoLeft);
    if (!m_guiObjects.lblVideoLeft)
    {
        m_logger.fatal("Failed to find LblVideoLeft");
        throw Gtk::BuilderError(Gtk::BuilderError::INVALID_ID, "Failed to find LblVideoFileLeft");
    }

    m_builder->get_widget("LblVideoRight", m_guiObjects.lblVideoRight);
    if (!m_guiObjects.lblVideoRight)
    {
        m_logger.fatal("Failed to find LblVideoRight");
        throw Gtk::BuilderError(Gtk::BuilderError::INVALID_ID, "Failed to find LblVideoFileRight");
    }

    m_builder->get_widget("BtnGoToStart", m_guiObjects.btnGoToStart);
    if (!m_guiObjects.btnGoToStart)
    {
        m_logger.fatal("Failed to find BtnGoToStart");
        throw Gtk::BuilderError(Gtk::BuilderError::INVALID_ID, "Failed to find BtnGoToStart");
    }

    m_builder->get_widget("BtnBack5Frames", m_guiObjects.btnBack5Frames);
    if (!m_guiObjects.btnBack5Frames)
    {
        m_logger.fatal("Failed to find BtnBack5Frames");
        throw Gtk::BuilderError(Gtk::BuilderError::INVALID_ID, "Failed to find BtnBack5Frames");
    }

    m_builder->get_widget("BtnBack1Frame", m_guiObjects.btnBack1Frame);
    if (!m_guiObjects.btnBack1Frame)
    {
        m_logger.fatal("Failed to find BtnBack1Frame");
        throw Gtk::BuilderError(Gtk::BuilderError::INVALID_ID, "Failed to find BtnBack1Frame");
    }

    m_builder->get_widget("BtnForward1Frame", m_guiObjects.btnForward1Frame);
    if (!m_guiObjects.btnForward1Frame)
    {
        m_logger.fatal("Failed to find BtnForward1Frame");
        throw Gtk::BuilderError(Gtk::BuilderError::INVALID_ID, "Failed to find BtnForward1Frame");
    }

    m_builder->get_widget("BtnForward5Frames", m_guiObjects.btnForward5Frames);
    if (!m_guiObjects.btnForward5Frames)
    {
        m_logger.fatal("Failed to find BtnForward5Frames");
        throw Gtk::BuilderError(Gtk::BuilderError::INVALID_ID, "Failed to find BtnForward5Frames");
    }

    m_builder->get_widget("BtnPlay", m_guiObjects.btnPlay);
    if (!m_guiObjects.btnPlay)
    {
        m_logger.fatal("Failed to find BtnPlay");
        throw Gtk::BuilderError(Gtk::BuilderError::INVALID_ID, "Failed to find BtnPlay");
    }

    m_builder->get_widget("ScrollVideo", m_guiObjects.scrollVideo);
    if (!m_guiObjects.scrollVideo)
    {
        m_logger.fatal("Failed to find ScrollVideo");
        throw Gtk::BuilderError(Gtk::BuilderError::INVALID_ID, "Failed to find ScrollVideo");
    }

    m_builder->get_widget("ComboPlaybackRate", m_guiObjects.comboPlaybackRate);
    if (!m_guiObjects.comboPlaybackRate)
    {
        m_logger.fatal("Failed to find ComboPlaybackRate");
        throw Gtk::BuilderError(Gtk::BuilderError::INVALID_ID, "Failed to find ComboPlaybackRate");
    }

    m_builder->get_widget("ChkSyncMode", m_guiObjects.chkSyncMode);
    if (!m_guiObjects.chkSyncMode)
    {
        m_logger.fatal("Failed to find ChkSyncMode");
        throw Gtk::BuilderError(Gtk::BuilderError::INVALID_ID, "Failed to find ChkSyncMode");
    }

    m_builder->get_widget("BtnPolyline", m_guiObjects.btnPolyline);
    if (!m_guiObjects.btnPolyline)
    {
        m_logger.fatal("Failed to find BtnPolyline");
        throw Gtk::BuilderError(Gtk::BuilderError::INVALID_ID, "Failed to find BtnPolyline");
    }

    m_builder->get_widget("BtnEllipse", m_guiObjects.btnEllipse);
    if (!m_guiObjects.btnEllipse)
    {
        m_logger.fatal("Failed to find BtnEllipse");
        throw Gtk::BuilderError(Gtk::BuilderError::INVALID_ID, "Failed to find BtnEllipse");
    }

    m_builder->get_widget("BtnFreehand", m_guiObjects.btnFreehand);
    if (!m_guiObjects.btnFreehand)
    {
        m_logger.fatal("Failed to find BtnFreehand");
        throw Gtk::BuilderError(Gtk::BuilderError::INVALID_ID, "Failed to find BtnFreehand");
    }

    m_builder->get_widget("BtnDeleteLast", m_guiObjects.btnDeleteLast);
    if (!m_guiObjects.btnDeleteLast)
    {
        m_logger.fatal("Failed to find BtnDeleteLast");
        throw Gtk::BuilderError(Gtk::BuilderError::INVALID_ID, "Failed to find BtnDeleteLast");
    }

    m_builder->get_widget("BtnClear", m_guiObjects.btnClear);
    if (!m_guiObjects.btnClear)
    {
        m_logger.fatal("Failed to find BtnClear");
        throw Gtk::BuilderError(Gtk::BuilderError::INVALID_ID, "Failed to find BtnClear");
    }

    m_builder->get_widget("BtnHelp", m_guiObjects.btnHelp);
    if (!m_guiObjects.btnHelp)
    {
        m_logger.fatal("Failed to find BtnHelp");
        throw Gtk::BuilderError(Gtk::BuilderError::INVALID_ID, "Failed to find BtnHelp");
    }

    m_logger.info("Loaded all GUI objects");
}
