#include <AnalyzerGui.h>
#include <IVideoChannel.h>
#include <VideoPlaybackConstants.h>

#include <gtkmm/filechooserdialog.h>
#include <gtkmm/filefilter.h>

#include <iomanip>
#include <sstream>

AnalyzerGui::AnalyzerGui(const Glib::RefPtr<Gtk::Builder> &pBuilder) : m_builder(pBuilder), m_logger{Logger::get()}
{
    m_logger.setLogLevel(LogSeverity::DEBUG);
    m_logger.info("Initializing AnalyzerGui");

    // Set up the GUI
    loadGuiObjects();
    setUpGui();
    connectSignals();

    // by default we'll start in sync mode
    m_guiObjects.chkSyncMode->set_active(true);
    updateSyncModeControls(true);

    m_logger.info("AnalyzerGui initialized");
}

AnalyzerGui::~AnalyzerGui()
{
    m_logger.info("AnalyzerGui destroyed");
}

void AnalyzerGui::setController(std::shared_ptr<VideoPlayback::IVideoSynchronizationController> controller)
{
    m_controller = controller;
    m_logger.info("Controller set in AnalyzerGui");
}

void AnalyzerGui::updateVideoPosition(VideoPlayback::WhichVideoChannel channel, float position)
{
    if (position < 0 || position > 1)
    {
        m_logger.error("Invalid video position for channel %d: %f", static_cast<int>(channel), position);
        return;
    }

    switch (channel)
    {
        case VideoPlayback::WhichVideoChannel::LEFT_CHANNEL:
            // we'll block the scrollbar update while we update the scrollbar position
            m_left_channel_scrollbar_connection.block();
            m_guiObjects.scrollVideoLeft->set_value(position);
            m_left_channel_scrollbar_connection.unblock();
            break;
            
        case VideoPlayback::WhichVideoChannel::RIGHT_CHANNEL:
            // we'll block the scrollbar update while we update the scrollbar position
            m_right_channel_scrollbar_connection.block();
            m_guiObjects.scrollVideoRight->set_value(position);
            m_right_channel_scrollbar_connection.unblock();
            break;
            
        case VideoPlayback::WhichVideoChannel::BOTH_CHANNELS:
            // we'll block the scrollbar update while we update the scrollbar position
            m_dual_channel_scrollbar_connection.block();
            m_guiObjects.scrollVideo->set_value(position);
            m_dual_channel_scrollbar_connection.unblock();
            break;
            
        default:
            m_logger.warn("Invalid channel for video position update: %d", static_cast<int>(channel));
            break;
    }
}

void AnalyzerGui::updatePlaybackState(VideoPlayback::WhichVideoChannel channel, VideoPlayback::VideoPlaybackStatus status)
{
    // Update play button text based on playback state
    // This is a simplified implementation - you might want more sophisticated state tracking
    m_logger.debug("Playback state changed for channel %d: %d", static_cast<int>(channel), static_cast<int>(status));
}

void AnalyzerGui::updateSyncModeControls(bool syncMode)
{
    m_logger.info("Updating GUI for %s mode", syncMode ? "sync" : "playback");

    m_guiObjects.btnGoToStart->set_sensitive(!syncMode);
    m_guiObjects.btnBack5Frames->set_sensitive(!syncMode);
    m_guiObjects.btnBack1Frame->set_sensitive(!syncMode);
    m_guiObjects.btnForward1Frame->set_sensitive(!syncMode);
    m_guiObjects.btnForward5Frames->set_sensitive(!syncMode);
    m_guiObjects.scrollVideo->set_sensitive(!syncMode);

    m_guiObjects.btnLoadLeftVideo->set_sensitive(syncMode);
    m_guiObjects.btnUnloadLeftVideo->set_sensitive(syncMode);
    m_guiObjects.btnLoadRightVideo->set_sensitive(syncMode);
    m_guiObjects.btnUnloadRightVideo->set_sensitive(syncMode);
    m_guiObjects.lblVideoLeft->set_sensitive(syncMode);
    m_guiObjects.lblVideoRight->set_sensitive(syncMode);
    m_guiObjects.btnBack1FrameLeft->set_sensitive(syncMode);
    m_guiObjects.btnBack5FramesLeft->set_sensitive(syncMode);
    m_guiObjects.btnForward1FrameLeft->set_sensitive(syncMode);
    m_guiObjects.btnForward5FramesLeft->set_sensitive(syncMode);
    m_guiObjects.scrollVideoLeft->set_sensitive(syncMode);
    m_guiObjects.btnBack1FrameRight->set_sensitive(syncMode);
    m_guiObjects.btnBack5FramesRight->set_sensitive(syncMode);
    m_guiObjects.btnForward1FrameRight->set_sensitive(syncMode);
    m_guiObjects.btnForward5FramesRight->set_sensitive(syncMode);
    m_guiObjects.scrollVideoRight->set_sensitive(syncMode);
}

void AnalyzerGui::updateVideoLoadedState(VideoPlayback::WhichVideoChannel channel, bool success)
{
    m_logger.info("Video load %s for channel %d", success ? "succeeded" : "failed", static_cast<int>(channel));
    
    // Update video filename labels, button states, etc.
    // This is where you'd update the GUI to reflect the loaded video state
}

void AnalyzerGui::updateVideoUnloadedState(VideoPlayback::WhichVideoChannel channel)
{
    m_logger.info("Video unloaded for channel %d", static_cast<int>(channel));
    
    // Update GUI to reflect unloaded state
}

void AnalyzerGui::setVideoRenderer(VideoPlayback::WhichVideoChannel channel, VideoPlayback::IVideoChannel* videoChannel)
{
    if (!videoChannel)
    {
        m_logger.error("Null video channel provided for renderer setup");
        return;
    }

    switch (channel)
    {
        case VideoPlayback::WhichVideoChannel::LEFT_CHANNEL:
            if (m_canvas_left)
            {
                m_canvas_left->get_root_item()->add_child(videoChannel->GetImage());
                
                // TEMPORARY TEST CODE TO ADD AN ANNOTATION ON THE LEFT CANVAS
                auto rect = Goocanvas::Rect::create(250, 200, 100, 100);
                rect->property_stroke_color() = "red";
                rect->property_line_width() = 6;
                m_canvas_left->get_root_item()->add_child(rect);

                m_canvas_left->show();
                m_logger.info("Left video renderer set up");
            }
            break;
            
        case VideoPlayback::WhichVideoChannel::RIGHT_CHANNEL:
            if (m_canvas_right)
            {
                m_canvas_right->get_root_item()->add_child(videoChannel->GetImage());
                m_canvas_right->show();
                m_logger.info("Right video renderer set up");
            }
            break;
            
        default:
            m_logger.error("Invalid channel for video renderer setup: %d", static_cast<int>(channel));
            break;
    }
}

std::optional<std::string> AnalyzerGui::showVideoFileDialog(const std::string& title)
{
    Gtk::FileChooserDialog dialog(title, Gtk::FILE_CHOOSER_ACTION_OPEN);
    
    dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
    dialog.add_button("_Open", Gtk::RESPONSE_OK);
    
    const std::string DEFAULT_VIDEO_DIR = "/home/tim/Downloads/Golf";
    dialog.set_current_folder(DEFAULT_VIDEO_DIR);
    
    auto video_filter = Gtk::FileFilter::create();
    video_filter->set_name("Video Files");
    video_filter->add_pattern("*.mp4");
    video_filter->add_pattern("*.avi");
    video_filter->add_pattern("*.mov");
    video_filter->add_pattern("*.mkv");
    dialog.add_filter(video_filter);
    
    auto all_filter = Gtk::FileFilter::create();
    all_filter->set_name("All Files");
    all_filter->add_pattern("*");
    dialog.add_filter(all_filter);
    
    int result = dialog.run();
    
    if (result == Gtk::RESPONSE_OK)
    {
        std::string filename = dialog.get_filename();
        m_logger.info("File selected: %s", filename.c_str());
        return filename;
    }
    else
    {
        m_logger.info("File dialog cancelled");
        return std::nullopt;
    }
}

void AnalyzerGui::onLoadLeftVideo()
{
    m_logger.info("Load left video requested");
    
    auto video_path = showVideoFileDialog("Select Left Video");
    if (!video_path.has_value())
    {
        m_logger.info("No file selected for left video");
        return;
    }
    
    if (m_controller)
    {
        m_controller->loadVideo(VideoPlayback::WhichVideoChannel::LEFT_CHANNEL, video_path.value());
    }
}

void AnalyzerGui::onLoadRightVideo()
{
    m_logger.info("Load right video requested");
    
    auto video_path = showVideoFileDialog("Select Right Video");
    if (!video_path.has_value())
    {
        m_logger.info("No file selected for right video");
        return;
    }
    
    if (m_controller)
    {
        m_controller->loadVideo(VideoPlayback::WhichVideoChannel::RIGHT_CHANNEL, video_path.value());
    }
}

void AnalyzerGui::onSyncModeToggle()
{
    bool syncMode = m_guiObjects.chkSyncMode->get_active();
    m_logger.info("Sync mode toggled to: %s", syncMode ? "enabled" : "disabled");
    
    if (m_controller)
    {
        m_controller->setSyncMode(syncMode);
    }
}

void AnalyzerGui::onVideoPositionChanged(VideoPlayback::WhichVideoChannel channel, float position)
{
    if (m_controller)
    {
        m_controller->setVideoPosition(channel, position);
    }
}

void AnalyzerGui::connectSignals()
{
    // Universal play/pause button
    m_guiObjects.btnPlay->signal_clicked().connect(
        [this]() { 
            if (m_controller) {
                m_controller->playVideo(VideoPlayback::BOTH_CHANNELS);
            }
        });

    // Sync mode select box
    m_guiObjects.chkSyncMode->signal_toggled().connect(
        [this]() { this->onSyncModeToggle(); });

    // Video loading buttons
    m_guiObjects.btnLoadLeftVideo->signal_clicked().connect(
        [this]() { this->onLoadLeftVideo(); });
    m_guiObjects.btnLoadRightVideo->signal_clicked().connect(
        [this]() { this->onLoadRightVideo(); });

    // Sync mode individual video controls
    m_guiObjects.btnBack5FramesLeft->signal_clicked().connect([this]() {
        if (m_controller) {
            m_controller->stepVideo(VideoPlayback::LEFT_CHANNEL, -VideoPlayback::FRAME_STEP_LARGE);
        }
    });
    m_guiObjects.btnBack1FrameLeft->signal_clicked().connect([this]() {
        if (m_controller) {
            m_controller->stepVideo(VideoPlayback::LEFT_CHANNEL, -VideoPlayback::FRAME_STEP_SMALL);
        }
    });
    m_guiObjects.btnForward1FrameLeft->signal_clicked().connect([this]() {
        if (m_controller) {
            m_controller->stepVideo(VideoPlayback::LEFT_CHANNEL, VideoPlayback::FRAME_STEP_SMALL);
        }
    });
    m_guiObjects.btnForward5FramesLeft->signal_clicked().connect([this]() {
        if (m_controller) {
            m_controller->stepVideo(VideoPlayback::LEFT_CHANNEL, VideoPlayback::FRAME_STEP_LARGE);
        }
    });

    m_guiObjects.btnBack5FramesRight->signal_clicked().connect([this]() {
        if (m_controller) {
            m_controller->stepVideo(VideoPlayback::RIGHT_CHANNEL, -VideoPlayback::FRAME_STEP_LARGE);
        }
    });
    m_guiObjects.btnBack1FrameRight->signal_clicked().connect([this]() {
        if (m_controller) {
            m_controller->stepVideo(VideoPlayback::RIGHT_CHANNEL, -VideoPlayback::FRAME_STEP_SMALL);
        }
    });
    m_guiObjects.btnForward1FrameRight->signal_clicked().connect([this]() {
        if (m_controller) {
            m_controller->stepVideo(VideoPlayback::RIGHT_CHANNEL, VideoPlayback::FRAME_STEP_SMALL);
        }
    });
    m_guiObjects.btnForward5FramesRight->signal_clicked().connect([this]() {
        if (m_controller) {
            m_controller->stepVideo(VideoPlayback::RIGHT_CHANNEL, VideoPlayback::FRAME_STEP_LARGE);
        }
    });

    m_left_channel_scrollbar_connection = m_guiObjects.scrollVideoLeft->signal_value_changed().connect([this]() {
        this->onVideoPositionChanged(VideoPlayback::LEFT_CHANNEL, this->m_guiObjects.scrollVideoLeft->get_value());
    });

    m_right_channel_scrollbar_connection = m_guiObjects.scrollVideoRight->signal_value_changed().connect([this]() {
        this->onVideoPositionChanged(VideoPlayback::RIGHT_CHANNEL, this->m_guiObjects.scrollVideoRight->get_value());
    });

    m_dual_channel_scrollbar_connection = m_guiObjects.scrollVideo->signal_value_changed().connect([this]() {
        this->onVideoPositionChanged(VideoPlayback::BOTH_CHANNELS, this->m_guiObjects.scrollVideo->get_value());
    });

    // Playback mode joint controls
    m_guiObjects.btnBack1Frame->signal_clicked().connect([this]() {
        if (m_controller) {
            m_controller->stepVideo(VideoPlayback::BOTH_CHANNELS, -VideoPlayback::FRAME_STEP_SMALL);
        }
    });
    m_guiObjects.btnForward1Frame->signal_clicked().connect([this]() {
        if (m_controller) {
            m_controller->stepVideo(VideoPlayback::BOTH_CHANNELS, VideoPlayback::FRAME_STEP_SMALL);
        }
    });
    m_guiObjects.btnBack5Frames->signal_clicked().connect([this]() {
        if (m_controller) {
            m_controller->stepVideo(VideoPlayback::BOTH_CHANNELS, -VideoPlayback::FRAME_STEP_LARGE);
        }
    });
    m_guiObjects.btnForward5Frames->signal_clicked().connect([this]() {
        if (m_controller) {
            m_controller->stepVideo(VideoPlayback::BOTH_CHANNELS, VideoPlayback::FRAME_STEP_LARGE);
        }
    });
    m_guiObjects.btnGoToStart->signal_clicked().connect([this]() {
        if (m_controller) {
            m_controller->setVideoToStart(VideoPlayback::BOTH_CHANNELS);
        }
    });
}

void AnalyzerGui::setUpGui()
{
    m_canvas_left = std::make_unique<Goocanvas::Canvas>();
    m_canvas_right = std::make_unique<Goocanvas::Canvas>();

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

    // TODO: TEMPORARY TEST CODE - SET VIDEO SIZE
    m_canvas_left->set_size_request(500, 1000);
    m_canvas_right->set_size_request(500, 1000);

    auto left_adjustment = Gtk::Adjustment::create(0., 0., 1., 0.025, 0.1, 0.);
    m_guiObjects.scrollVideoLeft->set_adjustment(left_adjustment);

    auto right_adjustment = Gtk::Adjustment::create(0., 0., 1., 0.025, 0.1, 0.);
    m_guiObjects.scrollVideoRight->set_adjustment(right_adjustment);

    auto dual_adjustment = Gtk::Adjustment::create(0., 0., 1., 0.025, 0.1, 0.);
    m_guiObjects.scrollVideo->set_adjustment(dual_adjustment);

    // Populate the playback rate option combo box
    for (float rate : VideoPlayback::playback_rates_array)
    {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << rate << "x";
        m_guiObjects.comboPlaybackRate->append(oss.str());
    }

    m_guiObjects.comboPlaybackRate->set_active_text("1.00x");
    m_guiObjects.comboPlaybackRate->signal_changed().connect([this]() {
        // TODO: Implement playback rate change
        return;
    });
}

void AnalyzerGui::loadGuiObjects()
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
