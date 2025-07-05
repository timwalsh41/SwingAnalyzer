#include <gtest/gtest.h>
#include <OpenCVVideoDecoder.h>
#include <string>

using namespace VideoPlayback;

class OpenCVVideoDecoderTest : public ::testing::Test {
protected:
    OpenCVVideoDecoder decoder;

    void SetUp() override {
        // Setup code if needed
    }

    void TearDown() override {
        decoder.unloadVideo();
    }
};

// Example test: decoder should not be loaded initially
TEST_F(OpenCVVideoDecoderTest, NotLoadedInitially) {
    EXPECT_FALSE(decoder.isLoaded());
}

// Example test: getCurrentFrameIndex returns -1 if not loaded
TEST_F(OpenCVVideoDecoderTest, GetCurrentFrameIndexNotLoaded) {
    EXPECT_EQ(decoder.getCurrentFrameIndex(), -1);
}

// Add more tests as needed, e.g., loading a video, seeking, etc.
