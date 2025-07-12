// Unit tests for VideoChannelStateMachine
#include <VideoChannelState.h>

#include <gtest/gtest.h>

using namespace VideoPlayback;

class VideoChannelStateMachineTest : public ::testing::Test 
{
    protected:
        VideoChannelStateMachine stateMachine;
};

TEST_F(VideoChannelStateMachineTest, InitialStateIsUnloaded)
{
    EXPECT_EQ(stateMachine.getCurrentState(), VideoChannelState::UNLOADED);
    EXPECT_STREQ(stateMachine.getStateName(), "UNLOADED");
    EXPECT_FALSE(stateMachine.isLoaded());
    EXPECT_FALSE(stateMachine.isPlaying());
}

TEST_F(VideoChannelStateMachineTest, LoadVideoTransition)
{
    EXPECT_TRUE(stateMachine.transition(VideoChannelEvent::LOAD_VIDEO));
    EXPECT_EQ(stateMachine.getCurrentState(), VideoChannelState::STOPPED);
    EXPECT_TRUE(stateMachine.isLoaded());
    EXPECT_FALSE(stateMachine.isPlaying());
}

TEST_F(VideoChannelStateMachineTest, PlayTransition)
{
    stateMachine.transition(VideoChannelEvent::LOAD_VIDEO);
    EXPECT_TRUE(stateMachine.transition(VideoChannelEvent::PLAY));
    EXPECT_EQ(stateMachine.getCurrentState(), VideoChannelState::PLAYING);
    EXPECT_TRUE(stateMachine.isPlaying());
}

TEST_F(VideoChannelStateMachineTest, PauseTransition)
{
    stateMachine.transition(VideoChannelEvent::LOAD_VIDEO);
    stateMachine.transition(VideoChannelEvent::PLAY);
    EXPECT_TRUE(stateMachine.transition(VideoChannelEvent::PAUSE));
    EXPECT_EQ(stateMachine.getCurrentState(), VideoChannelState::STOPPED);
    EXPECT_FALSE(stateMachine.isPlaying());
}

TEST_F(VideoChannelStateMachineTest, UnloadVideoTransition)
{
    stateMachine.transition(VideoChannelEvent::LOAD_VIDEO);
    EXPECT_TRUE(stateMachine.transition(VideoChannelEvent::UNLOAD_VIDEO));
    EXPECT_EQ(stateMachine.getCurrentState(), VideoChannelState::UNLOADED);
    EXPECT_FALSE(stateMachine.isLoaded());
}

TEST_F(VideoChannelStateMachineTest, StepFrameAndSeekFrameStayStopped)
{
    stateMachine.transition(VideoChannelEvent::LOAD_VIDEO);
    EXPECT_TRUE(stateMachine.transition(VideoChannelEvent::STEP_FRAME));
    EXPECT_EQ(stateMachine.getCurrentState(), VideoChannelState::STOPPED);
    EXPECT_TRUE(stateMachine.transition(VideoChannelEvent::SEEK_FRAME));
    EXPECT_EQ(stateMachine.getCurrentState(), VideoChannelState::STOPPED);
}

TEST_F(VideoChannelStateMachineTest, StepFrameAndSeekFrameFromPlayingGoToStopped)
{
    stateMachine.transition(VideoChannelEvent::LOAD_VIDEO);
    stateMachine.transition(VideoChannelEvent::PLAY);
    EXPECT_TRUE(stateMachine.transition(VideoChannelEvent::STEP_FRAME));
    EXPECT_EQ(stateMachine.getCurrentState(), VideoChannelState::STOPPED);
    stateMachine.transition(VideoChannelEvent::PLAY);
    EXPECT_TRUE(stateMachine.transition(VideoChannelEvent::SEEK_FRAME));
    EXPECT_EQ(stateMachine.getCurrentState(), VideoChannelState::STOPPED);
}

TEST_F(VideoChannelStateMachineTest, InvalidTransitionsReturnFalse)
{
    EXPECT_FALSE(stateMachine.transition(VideoChannelEvent::PLAY)); // Can't play when unloaded
    stateMachine.transition(VideoChannelEvent::LOAD_VIDEO);
    EXPECT_FALSE(stateMachine.transition(VideoChannelEvent::PAUSE)); // Can't pause when stopped
    stateMachine.transition(VideoChannelEvent::PLAY);
    EXPECT_FALSE(stateMachine.transition(VideoChannelEvent::LOAD_VIDEO)); // Can't load when playing
}

TEST_F(VideoChannelStateMachineTest, GetEventNameReturnsCorrectString)
{
    EXPECT_STREQ(VideoChannelStateMachine::getEventName(VideoChannelEvent::LOAD_VIDEO), "LOAD_VIDEO");
    EXPECT_STREQ(VideoChannelStateMachine::getEventName(VideoChannelEvent::UNLOAD_VIDEO), "UNLOAD_VIDEO");
    EXPECT_STREQ(VideoChannelStateMachine::getEventName(VideoChannelEvent::PLAY), "PLAY");
    EXPECT_STREQ(VideoChannelStateMachine::getEventName(VideoChannelEvent::PAUSE), "PAUSE");
    EXPECT_STREQ(VideoChannelStateMachine::getEventName(VideoChannelEvent::STEP_FRAME), "STEP_FRAME");
    EXPECT_STREQ(VideoChannelStateMachine::getEventName(VideoChannelEvent::SEEK_FRAME), "SEEK_FRAME");
    EXPECT_STREQ(VideoChannelStateMachine::getEventName(VideoChannelEvent::ERROR_OCCURRED), "ERROR_OCCURRED");
}
