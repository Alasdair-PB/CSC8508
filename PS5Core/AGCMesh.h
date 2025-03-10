/******************************************************************************
This file is part of the Newcastle PS5 Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Mesh.h"
#include <agc.h>

namespace NCL::PS5 {

	class AGCMesh : public Rendering::Mesh {
		friend class AGCRenderer;
	public:
		AGCMesh();

		void UploadToGPU(Rendering::RendererBase* renderer) override;

		void	BindVertexBuffers(sce::Agc::Core::StageBinder& binder);

		char*	GetAGCIndexData() const;
		bool	GetAGCBuffer(NCL::Rendering::VertexAttribute::Type, sce::Agc::Core::Buffer& outBuffer);

	private:
		std::vector< sce::Agc::Core::Buffer>				usedVertexBuffers;
		std::vector< NCL::Rendering::VertexAttribute::Type>	usedAttributeTypes;
		char* vertexData;
		char* indexData;
	};
}