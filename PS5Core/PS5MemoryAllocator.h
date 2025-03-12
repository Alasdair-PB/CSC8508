/******************************************************************************
This file is part of the Newcastle PS5 Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#pragma once

namespace NCL::PS5 {

	class MemoryAllocator {
	public:

		void* Allocate(size_t size, size_t alignment) const;
	protected:
	private:
	};
}