#ifndef ANALYZER_H
#define ANALYZER_H

#include <Logger.h>
#include <Synchronizer.h>
#include <VideoChannel.h>

#include <goocanvasmm.h>
#include <gtkmm.h>

#include <memory>

struct AnalyzerGuiObjects
{
    // Left/right channel video control buttons
    Gtk::Button *btnBack5FramesLeft{nullptr}, *btnBack5FramesRight{nullptr};
    Gtk::Button *btnBack1FrameLeft{nullptr}, *btnBack1FrameRight{nullptr};
    Gtk::Button *btnForward1FrameLeft{nullptr}, *btnForward1FrameRight{nullptr};
    Gtk::Button *btnForward5FramesLeft{nullptr}, *btnForward5FramesRight{nullptr};

    // Left/right channel video position scroll bars
    Gtk::Scrollbar *scrollVideoLeft, *scrollVideoRight;

    // Load/unload video buttons and video label
    Gtk::Button *btnLoadLeftVideo, *btnLoadRightVideo;
    Gtk::Button *btnUnloadLeftVideo, *btnUnloadRightVideo;
    Gtk::Label *lblVideoLeft, *lblVideoRight;

    // Joint video playback controls
    Gtk::Button *btnGoToStart, *btnBack5Frames, *btnBack1Frame, *btnForward1Frame, *btnForward5Frames, *btnPlay;

    // Joint video position scroll bar
    Gtk::Scrollbar *scrollVideo;

    // Playback rate combo box
    Gtk::ComboBox *comboPlaybackRate;

    // Sync mode check box
    Gtk::CheckButton *chkSyncMode;

    // Annotation controls
    Gtk::Button *btnPolyline, *btnEllipse, *btnFreehand, *btnDeleteLast, *btnClear;

    // Help button
    Gtk::Button *btnHelp;
};

class Analyzer
{
  public:
    Analyzer(const Glib::RefPtr<Gtk::Builder> &pBuilder);
    ~Analyzer();

    // Signal callbacks
    void loadLeftVideo();
    void loadRightVideo();

    // Callback for toggling control enabling
    void syncModeToggle(bool syncMode);

    // Callbacks for left/right channel to update video position for the slider
    void updateLeftVideoPosition(float video_position);
    void updateRightVideoPosition(float video_position);

  private:
    // maintain a reference to the GTK builder to access the GUI components
    Glib::RefPtr<Gtk::Builder> m_builder;

    // the analyzer owns the GooCanvas objects that we'll add to the window
    Goocanvas::Canvas *m_canvas_left, *m_canvas_right;

    // the analyzer owns the video channels
    VideoPlayback::VideoChannel m_channel_left{"left",
                                               [this](float position) { this->updateLeftVideoPosition(position); }};
    VideoPlayback::VideoChannel m_channel_right{"right",
                                               [this](float position) { this->updateRightVideoPosition(position); }};

    // hold the connections for the scrollbar callbacks so we can disable them when
    // the scrollbar position is updated by the video channels
    sigc::connection m_left_channel_scrollbar_connection, m_right_channel_scrollbar_connection;

    // the analyzer owns the synchronizer
    std::unique_ptr<VideoPlayback::Synchronizer> m_pSynchronizer;

    // we store the handles to all of our GUI objects in a struct
    AnalyzerGuiObjects m_guiObjects;

    Logger &m_logger;

    // helper functions to set up UI and callbacks
    void loadGuiObjects();
    void setUpGui();
    void connectSignals();
};

#endif // ANALYZER_H