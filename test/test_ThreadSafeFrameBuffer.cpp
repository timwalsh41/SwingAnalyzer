// Unit tests for ThreadSafeFrameBuffer
#include <ThreadSafeFrameBuffer.h>

#include <gtest/gtest.h>
#include <opencv2/opencv.hpp>

#include <thread>

using namespace VideoPlayback;

class ThreadSafeFrameBufferTest : public ::testing::Test
{
    protected:
        ThreadSafeFrameBuffer buffer;
};

TEST_F(ThreadSafeFrameBufferTest, InitiallyHasNoFrame)
{
    EXPECT_FALSE(buffer.hasFrame());
}

TEST_F(ThreadSafeFrameBufferTest, UpdateFrameSetsFrame)
{
    cv::Mat frame = cv::Mat::ones(10, 10, CV_8UC3);
    buffer.updateFrame(frame);
    EXPECT_TRUE(buffer.hasFrame());
}

TEST_F(ThreadSafeFrameBufferTest, UpdateFrameWithEmptyDoesNotSetFrame)
{
    cv::Mat emptyFrame;
    buffer.updateFrame(emptyFrame);
    EXPECT_FALSE(buffer.hasFrame());
}

TEST_F(ThreadSafeFrameBufferTest, GetDisplayFrameReturnsCloneOfBuffer)
{
    cv::Mat frame = cv::Mat::ones(5, 5, CV_8UC3);
    buffer.updateFrame(frame);
    cv::Mat out = buffer.getDisplayFrame(cv::Size(0, 0));
    ASSERT_FALSE(out.empty());
    EXPECT_EQ(out.rows, 5);
    EXPECT_EQ(out.cols, 5);
    EXPECT_EQ(out.type(), frame.type());
    // Should not be the same memory
    EXPECT_NE(out.data, frame.data);
}

TEST_F(ThreadSafeFrameBufferTest, GetDisplayFrameResizesCorrectly)
{
    cv::Mat frame = cv::Mat::ones(10, 20, CV_8UC3);
    buffer.updateFrame(frame);
    cv::Mat out = buffer.getDisplayFrame(cv::Size(5, 5));
    ASSERT_FALSE(out.empty());
    EXPECT_EQ(out.rows, 5);
    EXPECT_EQ(out.cols, 5);
}

TEST_F(ThreadSafeFrameBufferTest, GetDisplayFrameReturnsEmptyIfNoFrame)
{
    cv::Mat out = buffer.getDisplayFrame(cv::Size(10, 10));
    EXPECT_TRUE(out.empty());
}

TEST_F(ThreadSafeFrameBufferTest, ThreadSafety) {
    cv::Mat frame = cv::Mat::ones(10, 10, CV_8UC3);
    auto writer = [&]() {
        for (int i = 0; i < 100; ++i) buffer.updateFrame(frame);
    };
    auto reader = [&]() {
        for (int i = 0; i < 100; ++i) buffer.getDisplayFrame(cv::Size(5, 5));
    };
    std::thread t1(writer), t2(reader);
    t1.join();
    t2.join();
    EXPECT_TRUE(buffer.hasFrame());
}
