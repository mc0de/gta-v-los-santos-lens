#pragma once
// Garage car photos: a scripted-camera shot captured from the game window, saved as PNG, and
// drawn with ScriptHookV's texture API.

#include "invoke.h"
#include <string>

// <plugin folder>\LosSantosLens_garage\ (created if missing).
std::string PhotoDir();

// Points a camera at the car for a few frames, captures the game window and writes a 512x288 PNG
// to PhotoDir() + file. Calls scriptWait. Returns false on any failure (game not in the foreground,
// all-black capture, encoder error). The camera and HUD are always restored.
bool TakeCarPhoto(Vehicle v, const std::string& file);

// Draws PhotoDir() + file centred at x/y, w/h in screen fractions. Returns false without drawing
// when the file is missing or can't be loaded. Textures are cached by file name.
bool DrawPhoto(const std::string& file, float x, float y, float w, float h);

// Forgets every cached texture id. ScriptHookV deletes all textures when it reloads scripts
// (loading a save from in-game), so call this at the start of ScriptMain.
void PhotoResetTextures();
