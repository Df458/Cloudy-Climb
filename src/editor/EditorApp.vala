public class EditorApp : Gtk.Application
{
    public EditorApp()
    {
        activate.connect(activate_response);
        shutdown.connect(shutdown_response);
    }

    private void activate_response()
    {
        MainWindow win = new MainWindow();

        this.add_window(win);
        win.present();

        win.prepare();
    }

    private void shutdown_response()
    {
        Cloudy.Game.cleanup();
    }
}
