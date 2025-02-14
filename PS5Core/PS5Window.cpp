/******************************************************************************
This file is part of the Newcastle PS5 Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#include "PS5Window.h"
#include "Pad.h"
#include <sys\_defines\_sce_ok.h>

using namespace NCL;
using namespace PS5;

PS5Window::PS5Window(const std::string& title, int sizeX, int sizeY) {
	windowTitle = title;
	size.x = sizeX;
	size.y = sizeY;

	SceUserServiceInitializeParams params;
	memset((void*)&params, 0, sizeof(params));
	params.priority = SCE_KERNEL_PRIO_FIFO_DEFAULT;
	if (sceUserServiceInitialize(&params) != SCE_OK) {
		std::cout << "PS4Input: Failed to initialise User Service!" << std::endl;
	};


	SceUserServiceUserId userId;
	int ret = sceUserServiceGetInitialUser(&userId);


	scePadInit();

	testController = new PS5Controller(userId);

	keyboard = new DummyKeyboard();
	mouse = new DummyMouse();
}

PS5Window::~PS5Window(void) {
	sceUserServiceTerminate();
}

bool PS5Window::InternalUpdate() {
	testController->Update(1.0f);
	return true;
}