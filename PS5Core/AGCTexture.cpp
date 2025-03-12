/******************************************************************************
This file is part of the Newcastle PS5 Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#include "AGCTexture.h"
#include "PS5MemoryAllocator.h"
#include "Assets.h"

#include <fstream>
#include <filesystem>

using namespace NCL;
using namespace PS5;

AGCTexture::AGCTexture(const std::string& filename, const MemoryAllocator& a) {
	std::filesystem::path path(filename + ".gnf");
	//path.replace_extension(".gnf");
	std::string realFile = Assets::TEXTUREDIR + path.string();
	FILE* fp = fopen(realFile.c_str(), "rb");

	if (!fp) {
		std::cout << __FUNCTION__ << "Can't load file " << filename << "\n";
		return;
	}

	//SCE_AGC_ASSERT_MSG_RETURN(fp, SCE_AGC_ERROR_FAILURE, "Could not open file %s", filename);
	fseek(fp, 0, SEEK_END);
	long sz = ftell(fp);
	if (sz <= 0)
	{
		SCE_AGC_ASSERT_MSG("Could not get the size of the file %s", filename);
		fclose(fp);
		//return SCE_AGC_ERROR_FAILURE;
	}
	fseek(fp, 0, SEEK_SET);


	gpuAllocation = a.Allocate(sz, 64 * 1024);
	printf("%s(%s): p=%p, sz=%lu\n", __func__, filename.c_str(), gpuAllocation, sz);
	size_t bytesRead = fread(gpuAllocation, 1, sz, fp);
	if (bytesRead != sz)
	{
		SCE_AGC_ASSERT_MSG("Could not read the entire contents of the file %s", filename);
		fclose(fp);
		//return SCE_AGC_ERROR_FAILURE;
	}
	fclose(fp);

	SceError err = sce::Agc::Core::translate(&agcTex, (sce::Gnf::GnfFileV5*)gpuAllocation);
	//if (err == SCE_OK)
	//{
	//	sce::Agc::Core::registerResource(&agcTex, "%s", filename.c_str());
	//}
}

AGCTexture::AGCTexture(const MemoryAllocator& a) {
	gpuAllocation = 0;

}

AGCTexture::~AGCTexture() {

}