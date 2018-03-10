using DFGame;
using Cloudy;
using Gtk;

[GtkTemplate (ui="/org/df458/CloudyEditor/MainWindow.ui")]
public class MainWindow : ApplicationWindow
{
    [GtkChild]
    private DFGame.Viewport viewport;
    [GtkChild]
    private Button bt_new;
    [GtkChild]
    private Button bt_load;
    [GtkChild]
    private Button bt_save;

    [GtkChild]
    private RadioToolButton move_button;
    [GtkChild]
    private RadioToolButton goal_button;
    [GtkChild]
    private RadioToolButton bonus_button;
    [GtkChild]
    private RadioToolButton cloud_button;
    [GtkChild]
    private RadioToolButton strong_cloud_button;
    [GtkChild]
    private RadioToolButton spike_button;
    [GtkChild]
    private RadioToolButton rail_button;
    [GtkChild]
    private RadioToolButton straight_rail_button;
    [GtkChild]
    private RadioToolButton circular_rail_button;

    [GtkChild]
    private RadioButton white_button;
    [GtkChild]
    private RadioButton red_button;
    [GtkChild]
    private RadioButton blue_button;

    [GtkChild]
    private CheckButton rail_snap_button;

    private string current_path = null;

    private Gdk.Cursor selection_cursor = new Gdk.Cursor.from_name(Gdk.Display.get_default(), "grab");
    private Gdk.Cursor drag_cursor = new Gdk.Cursor.from_name(Gdk.Display.get_default(), "grabbing");

    private CssProvider style_provider = new CssProvider();

    public void prepare()
    {
        viewport.make_current();
        Game.init(viewport);

        style_provider.parsing_error.connect((section, e) => { warning(e.message); });
        style_provider.load_from_resource("/org/df458/CloudyEditor/cloudy.css");
        StyleContext.add_provider_for_screen(this.get_style_context().screen, style_provider, STYLE_PROVIDER_PRIORITY_APPLICATION);

        viewport.update_step.connect(on_loop);
        viewport.update_interval = 60;

        Game.set_state(Game.State.EDITOR);
    }

    private bool on_loop(float dt)
    {
        bool c = Game.loop(dt);

        switch(Editor.get_cursor_state()) {
            case Editor.Cursor.DEFAULT:
                bt_new.get_parent_window().cursor = null;
                break;
            case Editor.Cursor.CAN_DRAG:
                bt_new.get_parent_window().cursor = selection_cursor;
                break;
            case Editor.Cursor.DRAG:
                bt_new.get_parent_window().cursor = drag_cursor;
                break;
        }

        return c;
    }

    [GtkCallback]
    public void on_resize()
    {
        Game.resize_camera(viewport.get_allocated_width(), viewport.get_allocated_height());
    }

    [GtkCallback]
    public bool on_viewport_enter(Widget w, Gdk.EventCrossing c)
    {
        Editor.set_tool_enabled(true);
        return false;
    }

    [GtkCallback]
    public bool on_viewport_leave(Widget w, Gdk.EventCrossing c)
    {
        Editor.set_tool_enabled(false);
        return false;
    }

    [GtkCallback]
    public void game_toggle()
    {
        bt_new.sensitive = !bt_new.sensitive;
        bt_load.sensitive = !bt_load.sensitive;
        bt_save.sensitive = !bt_save.sensitive;
        if(Game.get_state() == Game.State.GAME)
            Game.set_state(Game.State.EDITOR);
        else {
            Game.set_state(Game.State.GAME);
            viewport.has_focus = true;
        }
    }

    [GtkCallback]
    private void new_requested()
    {
        current_path = null;
        Entities.cleanup(true);
    }

    [GtkCallback]
    private void save_requested()
    {
        if(current_path == null) {
            FileChooserNative chooser = new FileChooserNative("Save Level", this, FileChooserAction.SAVE, "Save", "Cancel");

            if(chooser.run() != ResponseType.CANCEL) {
                current_path = chooser.get_file().get_path();
                IO.save(current_path);
            }
        } else {
            IO.save(current_path);
        }
    }

    [GtkCallback]
    private void load_requested()
    {
        FileChooserNative chooser = new FileChooserNative("Open Level", this, FileChooserAction.OPEN, "Open", "Cancel");
        if(chooser.run() != ResponseType.CANCEL)
            IO.load(chooser.get_file().get_path());
    }

    [GtkCallback]
    private void tool_changed()
    {
        if(move_button.active) {
            Editor.set_tool(Editor.Tool.MOVE);
            rail_snap_button.sensitive = true;
        } else if(goal_button.active) {
            Editor.set_tool(Editor.Tool.GOAL);
            rail_snap_button.sensitive = true;
        } else if (bonus_button.active) {
            Editor.set_tool(Editor.Tool.BONUS);
            rail_snap_button.sensitive = true;
        } else if (cloud_button.active) {
            Editor.set_tool(Editor.Tool.CLOUD);
            rail_snap_button.sensitive = true;
        } else if (strong_cloud_button.active) {
            Editor.set_tool(Editor.Tool.STRONG_CLOUD);
            rail_snap_button.sensitive = true;
        } else if(spike_button.active) {
            Editor.set_tool(Editor.Tool.SPIKE);
            rail_snap_button.sensitive = true;
        } else if(rail_button.active) {
            Editor.set_tool(Editor.Tool.RAIL);
            rail_snap_button.sensitive = false;
        } else if(straight_rail_button.active) {
            Editor.set_tool(Editor.Tool.STRAIGHT_RAIL);
            rail_snap_button.sensitive = false;
        } else if(circular_rail_button.active) {
            Editor.set_tool(Editor.Tool.CIRCULAR_RAIL);
            rail_snap_button.sensitive = false;
        }
    }

    [GtkCallback]
    private void color_changed()
    {
        if(red_button.active) {
            Editor.set_color(Game.Color.RED);
        } else if(white_button.active) {
            Editor.set_color(Game.Color.NONE);
        } else if (blue_button.active) {
            Editor.set_color(Game.Color.BLUE);
        }
    }

    [GtkCallback]
    private void toggle_rail_snap()
    {
        Editor.set_snap_to_rail(rail_snap_button.active);
    }
}
