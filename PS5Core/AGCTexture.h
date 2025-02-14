/******************************************************************************
This file is part of the Newcastle PS5 Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Texture.h"

#include <agc.h>

namespace NCL::PS5 {
	class MemoryAllocator;

	class AGCTexture : public NCL::Rendering::Texture {
	public:
		AGCTexture(const std::string& filename, const MemoryAllocator& a);
		AGCTexture(const MemoryAllocator& a);
		~AGCTexture();

		const sce::Agc::Core::Texture* GetAGCPointer() const {
			return &agcTex;
		}
		sce::Agc::Core::Texture* GetAGCPointer() {
			return &agcTex;
		}
	protected:


	private:
		void* gpuAllocation;
		sce::Agc::Core::Texture agcTex;
	};
}