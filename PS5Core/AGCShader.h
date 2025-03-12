/******************************************************************************
This file is part of the Newcastle PS5 Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Shader.h"

#include <agc.h>

namespace NCL::PS5 {
	class MemoryAllocator;
	class AGCShader : public NCL::Rendering::Shader {
	public:
		AGCShader(const std::string&filename, MemoryAllocator& a);

		void ReloadShader() override;

		const sce::Agc::Shader* GetAGCPointer() const {
			return binary;
		}

	protected:

		uint8_t* allocation = nullptr;
		sce::Agc::Shader* binary = nullptr;
	private:
	};
}