#ifndef ANALYZERGUI_H
#define ANALYZERGUI_H

#include <IVideoSynchronizationController.h>
#include <IVideoChannel.h>
#include <Logger.h>
#include <VideoPlaybackTypes.h>

#include <goocanvasmm.h>
#include <gtkmm.h>

#include <memory>
#include <optional>
#include <string>

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
    Gtk::ComboBoxText *comboPlaybackRate;

    // Sync mode check box
    Gtk::CheckButton *chkSyncMode;

    // Annotation controls
    Gtk::Button *btnPolyline, *btnEllipse, *btnFreehand, *btnDeleteLast, *btnClear;

    // Help button
    Gtk::Button *btnHelp;
};

class AnalyzerGui
{
  public:
    AnalyzerGui(const Glib::RefPtr<Gtk::Builder> &pBuilder);
    ~AnalyzerGui();

    // Set the controller for handling user actions
    void setController(std::shared_ptr<VideoPlayback::IVideoSynchronizationController> controller);

    // GUI update methods (called by controller)
    void updateVideoPosition(VideoPlayback::WhichVideoChannel channel, float position);
    void updatePlaybackState(VideoPlayback::WhichVideoChannel channel, VideoPlayback::VideoPlaybackStatus status);
    void updateSyncModeControls(bool syncMode);
    void updateVideoLoadedState(VideoPlayback::WhichVideoChannel channel, bool success);
    void updateVideoUnloadedState(VideoPlayback::WhichVideoChannel channel);

    // Canvas management for video display
    void setVideoRenderer(VideoPlayback::WhichVideoChannel channel, VideoPlayback::IVideoChannel* videoChannel);

  private:
    // maintain a reference to the GTK builder to access the GUI components
    Glib::RefPtr<Gtk::Builder> m_builder;

    // Controller for handling user actions
    std::shared_ptr<VideoPlayback::IVideoSynchronizationController> m_controller;

    // the GUI owns the GooCanvas objects that we'll add to the window
    std::unique_ptr<Goocanvas::Canvas> m_canvas_left, m_canvas_right;

    // hold the connections for the scrollbar callbacks so we can disable them when
    // the scrollbar position is updated by the video channels
    sigc::connection m_left_channel_scrollbar_connection, m_right_channel_scrollbar_connection;

    // likewise for the dual channel connection in playback mode
    sigc::connection m_dual_channel_scrollbar_connection;

    // we store the handles to all of our GUI objects in a struct
    AnalyzerGuiObjects m_guiObjects;

    Logger &m_logger;

    // helper functions to set up UI and callbacks
    void loadGuiObjects();
    void setUpGui();
    void connectSignals();
    
    std::optional<std::string> showVideoFileDialog(const std::string& title);
    
    // Internal signal callbacks
    void onLoadLeftVideo();
    void onLoadRightVideo();
    void onSyncModeToggle();
    void onVideoPositionChanged(VideoPlayback::WhichVideoChannel channel, float position);
};

#endif // ANALYZERGUI_H
