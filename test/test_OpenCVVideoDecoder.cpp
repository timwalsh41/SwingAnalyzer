#include <OpenCVVideoDecoder.h>

#include <gtest/gtest.h>

#include <string>

using namespace VideoPlayback;

class OpenCVVideoDecoderTest : public ::testing::Test
{
    protected:
        OpenCVVideoDecoder decoder;

        void SetUp() override {
            // Setup code if needed
        }

        void TearDown() override {
            decoder.unloadVideo();
        }
};

TEST_F(OpenCVVideoDecoderTest, NotLoadedInitially)
{
    EXPECT_FALSE(decoder.isLoaded());
}

TEST_F(OpenCVVideoDecoderTest, GetCurrentFrameIndexNotLoaded)
{
    EXPECT_EQ(decoder.getCurrentFrameIndex(), -1);
}

TEST_F(OpenCVVideoDecoderTest, LoadInvalidVideo)
{
    VideoLoadStatus status = decoder.loadVideo("invalid_video.mp4");
    EXPECT_EQ(status, VideoLoadStatus::VIDEO_LOAD_FILE_NOT_FOUND);
    EXPECT_FALSE(decoder.isLoaded());
}

TEST_F(OpenCVVideoDecoderTest, LoadValidVideo)
{
    VideoLoadStatus status = decoder.loadVideo("test_video.mp4");
    EXPECT_EQ(status, VideoLoadStatus::VIDEO_LOAD_SUCCESS);
    EXPECT_TRUE(decoder.isLoaded());
}