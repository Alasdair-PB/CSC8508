/******************************************************************************
This file is part of the Newcastle PS5 Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Buffer.h"

#include <agc.h>

namespace NCL::PS5 {
	class MemoryAllocator;

	class AGCBuffer : public NCL::Rendering::Buffer {
	public:
		AGCBuffer(const sce::Agc::Core::Buffer& buffer, void* mem) : agcBuffer(buffer), gpuAllocation(mem) {
		}
		~AGCBuffer() {

		}

		const sce::Agc::Core::Buffer* GetAGCPointer() const {
			return &agcBuffer;
		}
		sce::Agc::Core::Buffer* GetAGCPointer() {
			return &agcBuffer;
		}

		void* GetAllocatedMemory() {
			return gpuAllocation;
		}

	protected:


	private:		
		sce::Agc::Core::Buffer agcBuffer;
		void* gpuAllocation;
	};
}