#ifndef VIDEO_PLAYBACK_CONSTANTS_H
#define VIDEO_PLAYBACK_CONSTANTS_H

#include <algorithm>
#include <array>

namespace VideoPlayback
{
// Playback rate options - these will be selected
constexpr inline std::array<float, 6> playback_rates_array = {0.1, 0.25, 0.5, 1.0, 2.0, 5.0};

// framerate multiplier limits - these modify the video's internal framerate to adjust
// playback speed
constexpr inline float MAX_FRAMERATE_MULTIPLIER =
    1 / (*std::min_element(playback_rates_array.begin(), playback_rates_array.end()));
constexpr inline float MIN_FRAMERATE_MULTIPLIER =
    1 / (*std::max_element(playback_rates_array.begin(), playback_rates_array.end()));

// frame step sizes for playback mode
constexpr inline int FRAME_STEP_SMALL = 1;
constexpr inline int FRAME_STEP_LARGE = 5;
} // namespace VideoPlayback

#endif // VIDEO_PLAYBACK_CONSTANTS_H