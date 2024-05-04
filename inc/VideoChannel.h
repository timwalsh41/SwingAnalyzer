#ifndef VIDEOCHANNEL_H
#define VIDEOCHANNEL_H

#include <Logger.h>
#include <Profiler.h>

#include <glibmm/main.h>
#include <goocanvasmm.h>
#include <opencv2/opencv.hpp>

#include <mutex>
#include <string>

namespace VideoPlayback
{
struct VideoSize
{
    int width;
    int height;
    float aspectRatio;
};

enum VideoLoadStatus
{
    VIDEO_LOAD_SUCCESS,
    VIDEO_LOAD_FILE_NOT_FOUND,
    VIDEO_LOAD_INVALID_TYPE,
    VIDEO_LOAD_UNKNOWN_ERROR
};

class VideoChannel
{
  public:
    VideoChannel(const std::string name, std::function<void(float)> m_update_video_position_callback);
    ~VideoChannel();

    const Glib::RefPtr<Goocanvas::Image> &get_image()
    {
        return m_image;
    }

    const int getActiveFrame()
    {
        return m_active_frame;
    }
    const int getStartFrame()
    {
        return m_start_frame;
    }
    const int getEndFrame()
    {
        return m_end_frame;
    }
    const int getFrameCount()
    {
        return m_frame_count;
    }
    const float getVideoPosition()
    {
        return m_video_position;
    }
    const float getFramerateMultiplier()
    {
        return m_framerate_multiplier;
    }

    // callbacks for user controls
    void playOrPauseVideo();
    void stepFrame(int frame_offset);

    // callback for user update of slider position
    void setVideoPosition(float video_position);
    void setFramerateMultiplier(float framerate_multiplier);

    // functions to manipulate active range of video, used by
    // synchronizer
    void setStartFrame(int start_frame);
    void setEndFrame(int end_frame);

    void setVideoSize(int availableHeight, int availableWidth);
    const VideoSize getVideoSize();
    VideoLoadStatus loadVideo(const std::string &videoPath);
    void unloadVideo();

  private:
    // video capture object holds the video data and provides individual frames
    cv::VideoCapture m_video_capture;

    // the Mat object holds the data for the current frame
    cv::Mat m_frame{};

    // the pixbuf object holds non-video data, e.g. a blank frame to display before
    // the video is loaded
    Glib::RefPtr<Gdk::Pixbuf> m_pixbuf;

    // the dispatcher object is used for the video playback worker thread to signal to
    // the main thread to update the pixbuf
    Glib::Dispatcher m_dispatcher;

    // this mutex is used to protect the pixbuf, which is updated with the new frame data
    // the the video playback thread, and read from the main thread to update the canvas
    std::mutex m_pixBufMutex;

    // the image object is the Goocanvas object that displays the video frames on the canvas
    Glib::RefPtr<Goocanvas::Image> m_image;

    // callback to Analyzer to update the slider position
    std::function<void(float)> m_update_video_position_callback;

    // Video properties
    float m_frames_per_second;
    int m_frame_count;

    // Video size
    int m_video_width{0}, m_video_height{0};
    float m_video_aspect_ratio{0};

    // current displayed frame, this will be kept in sync with the video position
    int m_active_frame{0};

    // video position as a proportion of the range between the start frame and end frame, this will be kept in
    // sync with the active frame
    float m_video_position{0};

    // active range of frames
    int m_start_frame{0};
    int m_end_frame{0};

    // time in microseconds to wait between frames
    float m_frame_time_us{0};

    // flag to continue playback in worker thread
    bool m_continue_playback{false};

    // flag to update teh video slider position if needed
    bool m_update_video_position{false};

    // framerate multiplier to adjust playback speed
    float m_framerate_multiplier{1.};

    // Name of channel
    std::string m_name;

    // We will profile the frame capture and image update operations
    Utility::Profiler m_profiler_frame{"VideoChannelFrameProfiler"};
    Utility::Profiler m_profiler_image{"VideoChannelImageProfiler"};

    Logger &m_logger;

    // Directly set the video frame for display
    void setActiveFrame(int frame_index);

    // Function to update the display for the active frame
    void updateDisplayedFrame();

    // Function for the video playback worker thread that processes the video frames
    // and updates the pixbuf for display on the canvas
    void playVideoWorkerThread();

    // Function to update the video position
    void updateVideoPosition(float position, bool update_frame);

    // Start video playback by kicking off the playback worker thread
    const void playVideo();
};

} // namespace VideoPlayback
#endif // VIDEOCHANNEL_H