#pragma once

#include "Horizon/Core/Memory.hpp"

#include "Horizon/Renderer/RendererSpecification.hpp"
#include "Horizon/Renderer/Descriptors.hpp"

#include <vulkan/vulkan.h>

namespace Hz
{

	class VulkanPipeline;

	class VulkanDescriptorSet
	{
	public:
		VulkanDescriptorSet(uint32_t setID, const std::vector<VkDescriptorSet>& sets);
		~VulkanDescriptorSet() = default;

		void Bind(Ref<Pipeline> pipeline, Ref<CommandBuffer> commandBuffer, PipelineBindPoint bindPoint, const std::vector<uint32_t>& dynamicOffsets);

		inline uint32_t GetSetID() const { return m_SetID; }
		inline const VkDescriptorSet GetVkDescriptorSet(uint32_t index) const { return m_DescriptorSets[index]; }

        // !TODO: Add some kind of Upload function which can take multiple inputs to optimize for speed

	private:
		uint32_t m_SetID = 0;

		// Note: One for every frame in flight
		std::vector<VkDescriptorSet> m_DescriptorSets = { };
	};

	class VulkanDescriptorSets
	{
	public:
		VulkanDescriptorSets(const std::initializer_list<DescriptorSetGroup>& specs);
		~VulkanDescriptorSets();

        void SetAmountOf(uint32_t setID, uint32_t amount);
		uint32_t GetAmountOf(uint32_t setID) const;

		const DescriptorSetLayout& GetLayout(uint32_t setID);
		std::vector<Ref<DescriptorSet>>& GetSets(uint32_t setID);

	private:
		void CreateDescriptorSetLayout(uint32_t setID);
		void CreateDescriptorPool(uint32_t setID, uint32_t amount);
		void CreateDescriptorSets(uint32_t setID, uint32_t amount);
		void ConvertToVulkanDescriptorSets(uint32_t setID, uint32_t amount, std::vector<VkDescriptorSet>& sets);

	private:
		std::unordered_map<uint32_t, DescriptorSetLayout> m_OriginalLayouts = { };
		std::unordered_map<uint32_t, std::vector<Ref<DescriptorSet>>> m_DescriptorSets = { };

		std::unordered_map<uint32_t, VkDescriptorSetLayout> m_DescriptorLayouts = { };
		std::unordered_map<uint32_t, VkDescriptorPool> m_DescriptorPools = { };

		friend class VulkanPipeline;
	};

}
