#ifndef VIDEO_PLAYBACK_CONSTANTS_H
#define VIDEO_PLAYBACK_CONSTANTS_H

#include <algorithm>
#include <array>

namespace VideoPlayback
{
// Playback rate options - these will be selected
constexpr std::array<float, 6> playback_rates_array = {0.1, 0.25, 0.5, 1.0, 2.0, 5.0};

// framerate multiplier limits - these modify the video's internal framerate to adjust
// playback speed
constexpr float MAX_FRAMERATE_MULTIPLIER =
    1 / (*std::min_element(playback_rates_array.begin(), playback_rates_array.end()));
constexpr float MIN_FRAMERATE_MULTIPLIER =
    1 / (*std::max_element(playback_rates_array.begin(), playback_rates_array.end()));
} // namespace VideoPlayback

#endif // VIDEO_PLAYBACK_CONSTANTS_H