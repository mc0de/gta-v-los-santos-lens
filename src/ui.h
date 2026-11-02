#pragma once
// Screen drawing, keyboard input and notifications.

#include "invoke.h"

struct Color { int r, g, b, a; };

constexpr Color kWhite   { 255, 255, 255, 220 };
constexpr Color kRed     { 255,  60,  60, 230 };
constexpr Color kGreen   {  80, 255, 120, 230 };
constexpr Color kBlue    {  80, 160, 255, 230 };
constexpr Color kGrey    { 140, 140, 140, 160 };
constexpr Color kYellow  { 255, 210,  60, 230 };
constexpr Color kMagenta { 255,  60, 255, 255 };
constexpr Color kCyan    {  60, 230, 255, 240 };
constexpr Color kOrange  { 255, 150,  40, 240 };
constexpr Color kLilac   { 190, 150, 255, 230 };

struct Box2D { float x0, y0, x1, y1; };

float Distance(const Vec3& a, const Vec3& b);
Color WithAlpha(Color c, float alphaScale);

bool WorldToScreen(float x, float y, float z, float& sx, float& sy);
float AspectRatio();

void Rect(float cx, float cy, float w, float h, Color c);
void Line3D(const Vec3& a, const Vec3& b, Color c);
// A world-space line that is never hidden behind geometry or models.
void OverlayLine(const Vec3& a, const Vec3& b, Color c);
void Text(float x, float y, const char* s, Color c, float scale = 0.26f, bool centre = true);
// Draws a sprite from a streamed texture dictionary; returns false while the dictionary is still loading.
bool Sprite(const char* dict, const char* name, float x, float y, float w, float h, float rotation, Color c);
void Notify(const char* s);

// Projects the entity's model bounding box (8 corners) and returns the enclosing screen rect.
bool ScreenBox(Entity e, Box2D& out);
void DrawBox(const Box2D& b, Color c);
// Small horizontal bar centred above the box.
void DrawHealthBar(const Box2D& b, float frac);
constexpr float kHealthBarGap = 0.006f; // distance from box top to bar centre

// Keyboard state, ignored while the game window isn't focused.
bool KeyPressed(int vk);
// Like KeyPressed, but repeats while held (for menu navigation).
bool KeyRepeat(int vk);
