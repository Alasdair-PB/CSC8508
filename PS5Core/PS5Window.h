/******************************************************************************
This file is part of the Newcastle PS5 Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Window.h"
#include "PS5Controller.h"

namespace NCL::PS5 {
	class PS5Window : public Window {
	public:
		PS5Window(const std::string& title, int sizeX, int sizeY);
		virtual ~PS5Window(void);

		PS5Controller* GetController() const {
			return testController;
		}

	protected:


		bool InternalUpdate() override;


	private:
		PS5Controller* testController;
	};
}