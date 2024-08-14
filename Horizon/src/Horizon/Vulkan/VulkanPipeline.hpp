#pragma once

#include "Horizon/Core/Memory.hpp"

#include "Horizon/Renderer/RendererSpecification.hpp"
#include "Horizon/Renderer/Pipeline.hpp"

#include <memory>
#include <unordered_map>

#include <vulkan/vulkan.h>

namespace Hz
{

	class VulkanDescriptorSets;

	class VulkanPipeline : public Pipeline
	{
	public:
		VulkanPipeline(const PipelineSpecification& specs, Ref<DescriptorSets> sets, Ref<Shader> shader, Ref<Renderpass> renderpass);
		VulkanPipeline(const PipelineSpecification& specs, Ref<DescriptorSets> sets, Ref<Shader> shader);
		~VulkanPipeline();

		void Use(Ref<CommandBuffer> commandBuffer, PipelineBindPoint bindPoint) override;

        void DispatchCompute(Ref<CommandBuffer> commandBuffer, uint32_t width, uint32_t height, uint32_t depth) override;

		inline const PipelineSpecification& GetSpecification() const override { return m_Specification; };

		inline const VkPipeline GetVkPipeline() const { return m_Pipeline; }
		inline const VkPipelineLayout GetVkPipelineLayout() const { return m_PipelineLayout; }

	private:
		void CreateGraphicsPipeline(Ref<DescriptorSets> sets, Ref<Shader> shader, Ref<Renderpass> renderpass);
		void CreateComputePipeline(Ref<DescriptorSets> sets, Ref<Shader> shader);
		void CreateRayTracingPipelineKHR(Ref<DescriptorSets> sets, Ref<Shader> shader);
		void CreateRayTracingPipelineNV(Ref<DescriptorSets> sets, Ref<Shader> shader);

		VkVertexInputBindingDescription GetBindingDescription();
		std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();

	private:
		PipelineSpecification m_Specification = {};

		VkPipeline m_Pipeline = VK_NULL_HANDLE;
		VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;

		friend class VulkanDescriptorSets;
	};

}
