#include "hzpch.h"
#include "VulkanShader.hpp"

#include "Horizon/Core/Logging.hpp"

#include "Horizon/Renderer/Renderer.hpp"
#include "Horizon/Renderer/GraphicsContext.hpp"
#include "Horizon/Renderer/CommandBuffer.hpp"

#include "Horizon/Vulkan/VulkanUtils.hpp"
#include "Horizon/Vulkan/VulkanContext.hpp"
#include "Horizon/Vulkan/VulkanCommandBuffer.hpp"

namespace Hz
{

    VulkanShader::VulkanShader(const ShaderSpecification& specs)
        : m_Specification(specs)
    {
        for (auto& [stage, code] : specs.ShaderCode)
            m_Shaders[stage] = CreateShaderModule(code);
    }

    VulkanShader::~VulkanShader()
    {
        // Note: This might be redundant since shaders are only usefuls when a pipeline needs to be created.
        // But better safe than sorry.
        Renderer::Free([shaders = m_Shaders]()
        {
            auto device = VulkanContext::GetDevice()->GetVkDevice();

            for (auto& [stage, shader] : shaders)
                vkDestroyShaderModule(device, shader, nullptr);
        });
    }

    VkShaderModule VulkanShader::CreateShaderModule(const std::vector<char>& code)
    {
		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VkShaderModule shaderModule = VK_NULL_HANDLE;
        VK_CHECK_RESULT(vkCreateShaderModule(VulkanContext::GetDevice()->GetVkDevice(), &createInfo, nullptr, &shaderModule));

		return shaderModule;
    }

}
