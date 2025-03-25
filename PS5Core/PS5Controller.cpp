/******************************************************************************
This file is part of the Newcastle PS5 Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#include "PS5Controller.h"
#include <sys\_defines\_sce_ok.h>
#include <pad.h>

using namespace NCL;
using namespace PS5;

PS5Controller::PS5Controller(SceUserServiceUserId id) {
	padHandle = scePadOpen(id, SCE_PAD_PORT_TYPE_STANDARD, 0, NULL);
	userId = id;
	scePadGetControllerInformation(padHandle, &padInfo);
}

PS5Controller::~PS5Controller(void) {
	scePadClose(padHandle);
}

float ConvertAxis(uint8_t rawValue, uint8_t deadZone) {
	if (std::abs(rawValue - 128) <= deadZone) {
		return 0.0f;
	}
	return (rawValue / 128.0f) - 1.0f;
}

void	PS5Controller::Update(float dt) {
	ScePadData data;

	int ret = scePadReadState(padHandle, &data);

	if (ret == SCE_OK) {
		if (data.connected) {
			axes[0] = ConvertAxis(data.leftStick.x, padInfo.stickInfo.deadZoneLeft);
			axes[1] = ConvertAxis(data.leftStick.y, padInfo.stickInfo.deadZoneLeft);

			axes[2] = ConvertAxis(data.rightStick.x, padInfo.stickInfo.deadZoneRight);
			axes[3] = -ConvertAxis(data.rightStick.y, padInfo.stickInfo.deadZoneRight);

			axes[4] = 0.0f;
			axes[4] += ((data.buttons & SCE_PAD_BUTTON_RIGHT) ? 1.0f : 0.0f);
			axes[4] -= ((data.buttons & SCE_PAD_BUTTON_LEFT) ? 1.0f : 0.0f);

			axes[5] = 0.0f;
			axes[5] += ((data.buttons & SCE_PAD_BUTTON_UP) ? 1.0f : 0.0f);
			axes[5] -= ((data.buttons & SCE_PAD_BUTTON_DOWN) ? 1.0f : 0.0f);

			buttons[0] = ((data.buttons & SCE_PAD_BUTTON_TRIANGLE) ? 1.0f : 0.0f);
			buttons[1] = ((data.buttons & SCE_PAD_BUTTON_CIRCLE) ? 1.0f : 0.0f);
			buttons[2] = ((data.buttons & SCE_PAD_BUTTON_CROSS) ? 1.0f : 0.0f);
			buttons[3] = ((data.buttons & SCE_PAD_BUTTON_SQUARE) ? 1.0f : 0.0f);

			buttons[4] = data.analogButtons.l2 / 255.0f;
			buttons[5] = data.analogButtons.r2 / 255.0f;

			buttons[6] = ((data.buttons & SCE_PAD_BUTTON_L1) ? 1.0f : 0.0f);
			buttons[7] = ((data.buttons & SCE_PAD_BUTTON_R1) ? 1.0f : 0.0f);

			buttons[8] = ((data.buttons & SCE_PAD_BUTTON_L3) ? 1.0f : 0.0f);
			buttons[9] = ((data.buttons & SCE_PAD_BUTTON_R3) ? 1.0f : 0.0f);
		}
	}
};

float	PS5Controller::GetAxis(uint32_t axis) const {
	if (axis >= AXIS_COUNT) {
		return 0.0f;
	}
	return axes[axis];
};

float	PS5Controller::GetButtonAnalogue(uint32_t button) const {
	if (button >= BUTTON_COUNT) {
		return 0.0f;
	}
	return buttons[button];
};

bool	PS5Controller::GetButton(uint32_t button) const {
	if (button >= BUTTON_COUNT) {
		return false;
	}
	return buttons[button] > 0.1f ? true : false;
};