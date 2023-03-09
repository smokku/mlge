const std = @import("std");

const raylib = @cImport(
    @cInclude("raylib.h"),
);

pub fn main() void {
    raylib.SetTraceLogLevel(4);

    raylib.InitWindow(640, 480, "MLGE");
    raylib.SetConfigFlags(raylib.FLAG_WINDOW_RESIZABLE);
    raylib.SetTargetFPS(60);

    defer raylib.CloseWindow();

    while (!raylib.WindowShouldClose()) {
        raylib.BeginDrawing();
        defer raylib.EndDrawing();

        raylib.ClearBackground(raylib.BLACK);
        raylib.DrawFPS(10, 10);

        raylib.DrawText("All your codebase are belong to us", 100, 100, 20, raylib.YELLOW);
    }
}
