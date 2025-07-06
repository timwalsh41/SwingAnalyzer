// Unit tests for VideoFrameManager
#include <VideoFrameManager.h>

#include <gtest/gtest.h>

using namespace VideoPlayback;

class VideoFrameManagerTest : public ::testing::Test {
protected:
    VideoFrameManager manager;
};

TEST_F(VideoFrameManagerTest, DefaultConstructorInitializesToZero)
{
    EXPECT_EQ(manager.getStartFrame(), 0);
    EXPECT_EQ(manager.getEndFrame(), 0);
    EXPECT_EQ(manager.getTotalFrames(), 0);
    EXPECT_EQ(manager.getEffectiveLength(), 1);
}

TEST_F(VideoFrameManagerTest, SetTotalFramesSetsRange)
{
    manager.setTotalFrames(10);
    EXPECT_EQ(manager.getTotalFrames(), 10);
    EXPECT_EQ(manager.getStartFrame(), 0);
    EXPECT_EQ(manager.getEndFrame(), 9);
    EXPECT_EQ(manager.getEffectiveLength(), 10);
}

TEST_F(VideoFrameManagerTest, SetFrameRangeValid)
{
    manager.setFrameRange(2, 7, 10);
    EXPECT_EQ(manager.getStartFrame(), 2);
    EXPECT_EQ(manager.getEndFrame(), 7);
    EXPECT_EQ(manager.getTotalFrames(), 10);
    EXPECT_EQ(manager.getEffectiveLength(), 6);
}

TEST_F(VideoFrameManagerTest, SetFrameRangeEndMinusOneMeansLast)
{
    manager.setFrameRange(3, -1, 8);
    EXPECT_EQ(manager.getStartFrame(), 3);
    EXPECT_EQ(manager.getEndFrame(), 7);
    EXPECT_EQ(manager.getTotalFrames(), 8);
}

TEST_F(VideoFrameManagerTest, SetStartFrameValidAndInvalid)
{
    manager.setFrameRange(0, 5, 6);
    manager.setStartFrame(2);
    EXPECT_EQ(manager.getStartFrame(), 2);
    manager.setStartFrame(6); // invalid, should not change
    EXPECT_EQ(manager.getStartFrame(), 2);
}

TEST_F(VideoFrameManagerTest, SetEndFrameValidAndInvalid)
{
    manager.setFrameRange(1, 4, 6);
    manager.setEndFrame(3);
    EXPECT_EQ(manager.getEndFrame(), 3);
    manager.setEndFrame(1); // invalid, should not change
    EXPECT_EQ(manager.getEndFrame(), 3);
    manager.setEndFrame(-1); // should set to last
    EXPECT_EQ(manager.getEndFrame(), 5);
}

TEST_F(VideoFrameManagerTest, IsValidFrame)
{
    manager.setFrameRange(2, 5, 10);
    EXPECT_TRUE(manager.isValidFrame(2));
    EXPECT_TRUE(manager.isValidFrame(5));
    EXPECT_FALSE(manager.isValidFrame(1));
    EXPECT_FALSE(manager.isValidFrame(6));
}

TEST_F(VideoFrameManagerTest, ClampFrame)
{
    manager.setFrameRange(3, 7, 10);
    EXPECT_EQ(manager.clampFrame(2), 3);
    EXPECT_EQ(manager.clampFrame(8), 7);
    EXPECT_EQ(manager.clampFrame(5), 5);
}

TEST_F(VideoFrameManagerTest, FrameNavigation)
{
    manager.setFrameRange(1, 4, 6);
    EXPECT_EQ(manager.getNextFrame(1), 2);
    EXPECT_EQ(manager.getNextFrame(4), 4);
    EXPECT_EQ(manager.getPreviousFrame(4), 3);
    EXPECT_EQ(manager.getPreviousFrame(1), 1);
    EXPECT_EQ(manager.stepFrame(2, 2), 4);
    EXPECT_EQ(manager.stepFrame(2, -2), 1);
    EXPECT_EQ(manager.stepFrame(2, 10), 4);
    EXPECT_EQ(manager.stepFrame(2, -10), 1);
}

TEST_F(VideoFrameManagerTest, ValidateFrameRangeResetsOnInvalid)
{
    manager.setFrameRange(5, 2, 10); // start > end, should reset
    EXPECT_EQ(manager.getStartFrame(), 0);
    EXPECT_EQ(manager.getEndFrame(), 9);
}
