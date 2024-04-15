#include <AnalyzerWindow.h>

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <videoPath>\n";
        return 1;
    }

    Glib::RefPtr<Gtk::Application> app = Gtk::Application::create(argc, argv, "swing.analyzer", Gio::APPLICATION_HANDLES_COMMAND_LINE);
    Glib::RefPtr<Gtk::Builder> builder = Gtk::Builder::create();

    try
    {
        builder->add_from_file("glade/SwingAnalyzer.glade");
    }
    catch(const Glib::FileError& ex)
    {
        std::cerr << "FileError: " << ex.what() << std::endl;
        return 1;
    }
    catch(const Glib::MarkupError& ex)
    {
        std::cerr << "MarkupError: " << ex.what() << std::endl;
        return 1;
    }
    catch(const Gtk::BuilderError& ex)
    {
        std::cerr << "BuilderError: " << ex.what() << std::endl;
        return 1;
    }
    
    AnalyzerWindow* pWindow;
    builder->get_widget_derived("MainWindow", pWindow, argv[1]);
    if(pWindow)
    {
        app->signal_command_line().connect([&app, &pWindow](const Glib::RefPtr<Gio::ApplicationCommandLine>&) -> int {
            app->add_window(*pWindow);
            pWindow->show();
            return 0;
        }, false);

        // Start the GTK+ event loop
        return app->run();
    }

    return 1;
}