#include "raylib-cpp.hpp"

int main() {
  // Initialization
  SetTraceLogLevel(LOG_DEBUG);

  //--------------------------------------------------------------------------------------
  int screenWidth = 800;
  int screenHeight = 480;
  raylib::Color textColor = raylib::Color::RayWhite();
  raylib::Window window(screenWidth, screenHeight, "MLGE");
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);

  SetTargetFPS(60);
  //--------------------------------------------------------------------------------------

  // Main game loop
  while (!window.ShouldClose()) { // Detect window close button or ESC key
    // Update
    //----------------------------------------------------------------------------------
    // Update your variables here
    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();
    {
      window.ClearBackground(BLACK);
      DrawFPS(10, 10);

      textColor.DrawText("All your codebase are belong to us", 200, 200, 20);
    }
    EndDrawing();
    //----------------------------------------------------------------------------------
  }

  return 0;
}
