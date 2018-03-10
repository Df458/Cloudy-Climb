namespace Cloudy
{
    [CCode (cheader_filename = "../gameloop.h")]
    namespace Game
    {
        [CCode (cname = "init_game")]
            public static void init(DFGame.Viewport win);
        [CCode (cname = "cleanup_game")]
            public static void cleanup();
        [CCode (cname = "loop")]
            public static bool loop(float dt);
        [CCode (cname = "resize_camera")]
            public static bool resize_camera(int width, int height);

        [CCode (cname = "game_state", cprefix = "")]
            public enum State {
                LOGO,
                MENU,
                OPTIONS,
                GAME,
                EDITOR
            }

        [CCode (cname = "stage_color", cprefix = "COLOR_")]
            public enum Color {
                RED = 0,
                BLUE = 1,
                NONE = 2
            }

        [CCode (cname = "get_state")]
            public static State get_state();
        [CCode (cname = "set_state")]
            public static void set_state(State state);
    }

    [CCode (cheader_filename = "../io.h")]
    namespace IO
    {
        [CCode (cname = "save_level")]
        public static void save(string path);
        [CCode (cname = "load_level")]
        public static void load(string path, bool debug = true);
    }

    [CCode (cheader_filename = "editor.h")]
    namespace Editor
    {
        [CCode (cname = "tool", cprefix = "TOOL_")]
        public enum Tool {
            MOVE,
            GOAL,
            BONUS,
            CLOUD,
            STRONG_CLOUD,
            SPIKE,
            RAIL,
            STRAIGHT_RAIL,
            CIRCULAR_RAIL
        }
        [CCode (cname = "cursor_state", cprefix = "CURSOR_")]
        public enum Cursor {
            DEFAULT,
            CAN_DRAG,
            DRAG,
        }
        [CCode (cname = "get_cursor_state")]
        public static Cursor get_cursor_state();
        [CCode (cname = "set_tool")]
        public static void set_tool(Tool tool);
        [CCode (cname = "set_tool_color")]
        public static void set_color(Game.Color color);
        [CCode (cname = "set_tool_enabled")]
        public static void set_tool_enabled(bool enabled);
        [CCode (cname = "set_snap_to_rail")]
        public static void set_snap_to_rail(bool snap);
    }

    [CCode (cheader_filename = "../entity.h")]
    namespace Entities
    {
        [CCode (cname = "cleanup_entities")]
        void cleanup(bool debug);
    }
}
