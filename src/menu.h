#pragma once

void MenuToggle();
// Garage hotkey: opens the menu on the Garage page, or closes it if that page is showing.
void MenuOpenGarage();
bool MenuIsOpen();
// Handles navigation and draws the menu while it's open.
void MenuTick();
