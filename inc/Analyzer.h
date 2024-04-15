#ifndef ANALYZER_H
#define ANALYZER_H

#include <gtkmm.h>
#include <goocanvasmm.h>

#include <memory>

#include <Synchronizer.h>
#include <VideoChannel.h>
#include <Logger.h>

class Analyzer  
{
    public:
        Analyzer(const Glib::RefPtr<Gtk::Builder> &pBuilder);
        ~Analyzer();

        // Signal callbacks
        void load_left_video();
    private:
        // maintain a reference to the GTK builder to access the GUI components
        Glib::RefPtr<Gtk::Builder> m_builder;

        // the analyzer owns the GooCanvas objects that we'll add to the window
        Goocanvas::Canvas *m_canvas_left, *m_canvas_right;

        // the analyzer owns the video channels
        VideoChannel m_channel_left, m_channel_right;

        // the analyzer owns the synchronizer
        std::unique_ptr<Synchronizer> m_pSynchronizer;

        Logger& m_logger; 
};

#endif // ANALYZER_H