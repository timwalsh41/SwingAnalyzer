#ifndef SYNCHRONIZER_H
#define SYNCHRONIZER_H

#include <VideoChannel.h>
#include <Logger.h>

class Analyzer;

namespace VideoPlayback
{

class Synchronizer
{
    // The SyncManager and its states are friend classes that can manipulate
    // synchronizer data as needed
    friend class SyncManager;
    friend class SynchronizerStateSyncToPlayback;
    friend class SynchronizerStateSync;
    friend class SynchronizerStatePlaybackPlaying;
    friend class SynchronizerStatePlaybackPaused;

    public:
        Synchronizer(Analyzer *analyzer, VideoChannel *channel_left, VideoChannel *channel_right);
        ~Synchronizer() = default;

    private:
        Analyzer *m_pAnalyzer;
        VideoChannel *m_pChannel_left, *m_pChannel_right;
        Logger &m_logger;

        // synchronized video offset (left channel position - right channel position)
        int m_video_frame_offset{0};

        // framerate ratio, left video framerate divided by right video framerate
        float m_framerate_ratio{1.0};

        // effective length of the left and right video channels, including user selected
        // start/end frame and offset
        int m_left_length{0}, m_right_length{0};
        
        // the combined length is the shorter of the right and left channels
        int m_combined_length{0};

        // flag for continuous playback mode, as selected by user
        bool m_continuous_playback{false};

        // member variable to hold the new position setpoint from the GUI callback
        // for the joint slider
        float m_new_video_position{0};

        // member variable to hold the new active frame from the video channel frame
        // update callback
        int m_new_active_frame{0};

        // when running in playback mode, we'll use the frame update from only one channel
        // to trigger updates to the slider; we'll set the appropriate channel upon transition
        // from sync to playback mode
        WhichVideoChannel m_video_position_update_channel{NO_CHANNEL};

        // set both videos to the start position, e.g. the start frame for each, with
        // the offset applied
        void setVideosToStartPosition();

        // set the playback parameters for the synced videos:
        // - offset between the two video channels
        // - start and end frames for both video channels
        // - playback length
        // - which channel to use for slider updates
        void setupVideoSync();

        // clear the start and end frames for both video channels
        void clearStartAndEndFrames();

        // calculate the framerate ratio for the two channels
        void calculateFramerateRatio();

        // step both videos by the given number of frames, with the step size adjusted
        // for differences in frame rate; video channel itself protects against stepping
        // out of bounds
        void stepVideos(int step_size);

        // Receive video position update, for updating video position based on
        // joint slider position
        void videoPositionUpdate(WhichVideoChannel channel);

        // Receive video frame update, for updating the GUI sliders on video channel
        // frame update during playback
        void videoFrameUpdate(WhichVideoChannel channel, bool update_joint_slider);

        void pauseVideos();
};

} // namespace VideoPlayback

#endif // SYNCHRONIZERIMPL_H