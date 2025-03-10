/******************************************************************************
This file is part of the Newcastle PS5 Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Controller.h"
#include "pad.h"

namespace NCL::PS5 {
	class PS5Controller : public Controller {
	public:
		PS5Controller(SceUserServiceUserId id);
		virtual ~PS5Controller(void);
	
		void	Update(float dt)		override;
		float	GetAxis(uint32_t axis)	const override;
		float	GetButtonAnalogue(uint32_t button) const override;
		bool	GetButton(uint32_t button) const override;

	protected:
	private:
		static const int AXIS_COUNT = 8;
		static const int BUTTON_COUNT = 16;

		uint32_t padHandle;
		ScePadControllerInformation padInfo;

		float	axes[AXIS_COUNT];
		float	buttons[BUTTON_COUNT];
	};
}