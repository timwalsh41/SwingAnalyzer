#ifndef VIDEOCHANNEL_H
#define VIDEOCHANNEL_H

#include <Logger.h>
#include <Profiler.h>
#include <VideoPlaybackTypes.h>

#include <glibmm/main.h>
#include <goocanvasmm.h>
#include <opencv2/opencv.hpp>

#include <mutex>
#include <string>

namespace VideoPlayback
{

class VideoChannel
{
  public:
    VideoChannel(const WhichVideoChannel channel, std::function<void(int,WhichVideoChannel)> update_video_position_callback);
    ~VideoChannel();

    const Glib::RefPtr<Goocanvas::Image> &GetImage()
    {
        return m_image;
    }

    const bool IsVideoLoaded()
    {
        return m_video_loaded;
    }

    const int getActiveFrame()
    {
        if (m_video_loaded)
        {
            return m_active_frame;
        }
        else
        {
            return 0;
        }
    }
    
    const int getStartFrame()
    {
        if (m_video_loaded)
        {
            return m_start_frame;
        }
        else
        {
            return 0;
        }
    }
    
    const int getEndFrame()
    {
        if (m_video_loaded)
        {
            return m_end_frame;
        }
        else
        {
            return 0;
        }
    }

    const inline int getEffectiveVideoLengthFrames()
    {
        if (m_video_loaded)
        {
            return m_end_frame - m_start_frame + 1;
        }
        else
        {
            return 0;
        }
    }
    
    const int getFrameCount()
    {
        if (m_video_loaded)
        {
            return m_frame_count;
        }
        else
        {
            return 0;
        }
    }
    
    const float getFramerateMultiplier()
    {
        if (m_video_loaded)
        {
            return m_framerate_multiplier;
        }
        else
        {
            return 0;
        }
    }

    const float getRawFramerate_fps()
    {
        if (m_video_loaded)
        {
            return m_frames_per_second;
        }
        else
        {
            return 0;
        }
    }

    const VideoPlaybackStatus getPlaybackStatus()
    {
        return m_continue_playback ? PLAYING : PAUSED;
    }

    void enableProfiler()
    {
        m_profiler_frame.enableProfiler();
        m_profiler_image.enableProfiler();
    }

    void disableProfiler()
    {
        m_profiler_frame.disableProfiler();
        m_profiler_image.disableProfiler();
    }
    
    // Start video playback by kicking off the playback worker thread
    void playVideo();

    // Stop video playback
    void pauseVideo();

    // Set the video position to the start frame
    void setVideoToStart();

    // Set the video to desired frame
    void setVideoToFrame(int frame_index);

    // Step video display by a number of frames
    void stepFrame(int frame_offset);

    void setFramerateMultiplier(float framerate_multiplier);

    // functions to manipulate active range of video, used by
    // synchronizer; pass end_frame = -1 to set to end of video
    void setStartFrame(int start_frame);
    void setEndFrame(int end_frame);

    void setVideoSize(int availableHeight, int availableWidth);
    const VideoSize getVideoSize();
    VideoLoadStatus loadVideo(const std::string &videoPath);
    void unloadVideo();

    void registerVideoPlaybackCompleteCallback(std::function<void(WhichVideoChannel)>);

  private:
    // video capture object holds the video data and provides individual frames
    cv::VideoCapture m_video_capture;

    // the Mat object holds the data for the current frame, this is used for thread-safe
    // updating of the frame
    cv::Mat m_frame_buffer{};

    // the display frame object holds the data for the current frame for updating the canvas
    // image in the main thread
    cv::Mat m_display_frame{};

    // the pixbuf object holds non-video data, e.g. a blank frame to display before
    // the video is loaded
    Glib::RefPtr<Gdk::Pixbuf> m_pixbuf;

    // the frame update dispatcher object is used for the video playback worker thread to signal to
    // the main thread to update the pixbuf
    Glib::Dispatcher m_frame_update_dispatcher;

    // the playback complete dispatcher object is used for the video playback worker thread to signal to
    // the main thread that playback is complete
    Glib::Dispatcher m_playback_complete_dispatcher;

    // this mutex is used to protect the active frame data, which is updated with the latest frame
    // from the video capture in the video playback thread, and read from the main thread to update 
    // the canvas
    std::mutex m_frameMutex;

    // the image object is the Goocanvas object that displays the video frames on the canvas
    Glib::RefPtr<Goocanvas::Image> m_image;

    // callback to synchronizer to update the slider position
    std::function<void(int,WhichVideoChannel)> m_update_video_position_callback = nullptr;

    // callback to synchronizer when playback is complete
    std::function<void(WhichVideoChannel)> m_video_complete_callback = nullptr;

    // Video properties
    float m_frames_per_second;
    int m_frame_count;

    // Video size
    int m_video_width{0}, m_video_height{0};
    float m_video_aspect_ratio{0};

    // current displayed frame, this will be kept in sync with the video position
    int m_active_frame{0};

    // active range of frames
    int m_start_frame{0};
    int m_end_frame{0};

    // time in microseconds to wait between frames
    float m_frame_time_us{0};

    // flag to continue playback in worker thread
    bool m_continue_playback{false};

    // flag to represent if a video is loaded
    bool m_video_loaded{false};

    // framerate multiplier to adjust playback speed
    float m_framerate_multiplier{1};

    // Name of channel
    WhichVideoChannel m_channel_id;

    // We will profile the frame capture and image update operations
    Utility::Profiler m_profiler_frame{std::string(channel_name_map.at(m_channel_id)) + "VideoChannelFrameProfiler"};
    Utility::Profiler m_profiler_image{std::string(channel_name_map.at(m_channel_id)) + "VideoChannelImageProfiler"};

    Logger &m_logger;

    // Directly set the video frame for display
    void setActiveFrame(int frame_index);

    // Transfer the display frame to the canvas image
    void updatePixBuf();

    // Function for the video playback worker thread that processes the video frames
    // and updates the pixbuf for display on the canvas
    void playVideoWorkerThread();
};

} // namespace VideoPlayback
#endif // VIDEOCHANNEL_H