using DFGame;

public static int main (string[] args)
{
    EditorApp app = new EditorApp();
    typeof(Viewport).ensure();
    return app.run(args);
}
