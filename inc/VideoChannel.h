#ifndef VIDEOCHANNEL_H
#define VIDEOCHANNEL_H

#include <glibmm/main.h>
#include <goocanvasmm.h>
#include <opencv2/opencv.hpp>

#include <mutex>

#include <Logger.h>

struct VideoSize
{
    float width;
    float height;
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
    VideoChannel();
    ~VideoChannel();

    const Glib::RefPtr<Goocanvas::Image> &get_image()
    {
        return m_image;
    }

  void setVideoSize(int availableHeight, int availableWidth);
  const VideoSize getVideoSize();
  VideoLoadStatus loadVideo(const std::string &videoPath);
  void unloadVideo();
  void playVideo();

  private:
    // video capture object holds the video data and provides individual frames
    cv::VideoCapture m_videoCapture;

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

    // Video properties
    double m_fps;
    int m_frame_count;

    // Video size
    float m_videoWidth{0}, m_videoHeight{0};
    float m_videoAspectRatio{0};

    Logger& m_logger;

    // Function for the video playback worker thread that processes the video frames
    // and updates the pixbuf for display on the canvas
    void playVideoWorkerThread();


};

#endif // VIDEOCHANNEL_H