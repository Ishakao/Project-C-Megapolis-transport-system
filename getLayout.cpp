#pragma once
#include <windows.h>
#include <string>

std::string getLayout() {
	HKL layout = GetKeyboardLayout(0);
	if (LOWORD(layout) == 0x0409)
		return "EN";
	else if (LOWORD(layout) == 0x0419)
		return "RU";
	return "IDK";
}

bool capsLock() {
	return (GetKeyState(VK_CAPITAL) & 0x0001) != 0;
}