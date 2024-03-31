#include <VideoWindow.h>

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <videoPath>\n";
        return 1;
    }

    Glib::RefPtr<Gtk::Application> app = Gtk::Application::create(argc, argv, "org.gtkmm.example", Gio::APPLICATION_HANDLES_COMMAND_LINE);
    VideoWindow window(argv[1]);

    app->signal_command_line().connect([&app, &window](const Glib::RefPtr<Gio::ApplicationCommandLine>&) -> int {
        app->add_window(window);
        window.show();
        return 0;
    }, false);

    return app->run();
}