/******************************************************************************
This file is part of the Newcastle PS5 Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#include "AGCShader.h"
#include "Assets.h"
#include "PS5MemoryAllocator.h"
#include <fstream>
#include <shader/shader_reflection.h>
#include "agc/error.h"

using namespace NCL;
using namespace PS5;

AGCShader::AGCShader(const std::string& filename, MemoryAllocator& allocator) {

	char* fileData = nullptr;
	size_t readSize = 0;

	if (Assets::ReadBinaryFile(Assets::SHADERDIR + "PSSL/" + filename, &fileData, readSize)) {
		//Now to allocate some GPU-visible memory for the file data to sit in...

		allocation = (uint8_t*)allocator.Allocate(readSize, sizeof(uint64_t) );
		memcpy(allocation, fileData, readSize);

		SceShaderBinaryHandle binaryHandle = sceShaderGetBinaryHandle(allocation);
		const void* header = sceShaderGetProgramHeader(binaryHandle);
		const void* program = sceShaderGetProgram(binaryHandle);

		SceError error = sce::Agc::createShader(&binary, const_cast<void*>(header), program);
	}
	delete fileData;
//	return shader;
}

void AGCShader::ReloadShader() {


}