#include <gtest/gtest.h>
#include <GTKImageUpdater.h>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <memory>
#include <gtkmm.h>

using namespace VideoPlayback;

class GTKImageUpdaterTest : public ::testing::Test {
protected:
    void SetUp() override {
        updater = std::make_unique<GTKImageUpdater>();
    }
    std::unique_ptr<GTKImageUpdater> updater;
};

TEST_F(GTKImageUpdaterTest, ConstructorInitializesDefaults) {
    ASSERT_NE(updater->getImageObject(), nullptr);
    ASSERT_NE(updater->getGooCanvasImage(), Glib::RefPtr<Goocanvas::Image>());
}

TEST_F(GTKImageUpdaterTest, SetImageSizeValid) {
    updater->setImageSize(200, 150);
    // No direct getter, but should not throw or log error
}

TEST_F(GTKImageUpdaterTest, SetImageSizeInvalid) {
    updater->setImageSize(5, 5); // Should log error, but not crash
}

TEST_F(GTKImageUpdaterTest, UpdateImageWithEmptyFrame) {
    cv::Mat empty;
    updater->updateImage(empty); // Should log warning, but not crash
}

TEST_F(GTKImageUpdaterTest, UpdateImageWithValidFrame) {
    cv::Mat frame(100, 100, CV_8UC3, cv::Scalar(255, 0, 0));
    updater->updateImage(frame);
    // Should update pixbuf and not throw
}

TEST_F(GTKImageUpdaterTest, GetImageReturnsSameAsGooCanvasImage) {
    ASSERT_EQ(updater->getImage().get(), updater->getGooCanvasImage().get());
}

int main(int argc, char** argv) {
    Gtk::Main kit(argc, argv); // Initialize GTK before any tests
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
