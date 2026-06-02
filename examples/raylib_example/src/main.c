#include "raylib.h"

int main(void) {
  const int width = 800;
  const int height = 450;

  InitWindow(width, height, "bbs raylib package example");
  SetTargetFPS(60);

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawText("raylib was fetched as a bbs package", 160, 180, 20, DARKGRAY);
    DrawText("Close the window to exit.", 245, 220, 20, GRAY);
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
