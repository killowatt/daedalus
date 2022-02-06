#pragma once

#include <vector>

class VulkanShader
{
public:
	VulkanShader() {}

	std::vector<uint8_t> VertexBytes;
	std::vector<uint8_t> FragmentBytes;

	static VulkanShader* CreateFromSPIRV(std::vector<uint8_t> vertexBytes, std::vector<uint8_t> fragmentBytes)
	{
		VulkanShader* shader = new VulkanShader();

		shader->VertexBytes = vertexBytes;
		shader->FragmentBytes = fragmentBytes;

		return shader;
	}
};