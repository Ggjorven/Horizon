#pragma once

#include "Horizon/Core/Memory.hpp"

#include "Horizon/Renderer/RendererSpecification.hpp"
#include "Horizon/Renderer/Shader.hpp"

#include <vulkan/vulkan.h>

namespace Hz
{

	class VulkanShader : public Shader
	{
	public:
		VulkanShader(const ShaderSpecification& specs);
		~VulkanShader();

        inline const ShaderSpecification& GetSpecification() const override { return m_Specification; }

        inline const VkShaderModule GetShader(ShaderStage stage) { return m_Shaders[stage]; }
        inline const std::unordered_map<ShaderStage, VkShaderModule>& GetShaders() { return m_Shaders; }

    private:
		static VkShaderModule CreateShaderModule(const std::vector<char>& code);

	private:
        ShaderSpecification m_Specification;
		std::unordered_map<ShaderStage, VkShaderModule> m_Shaders = {};
	};

}
