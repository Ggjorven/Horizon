#pragma once

#include "Horizon/Core/Core.hpp"

#include "Horizon/Renderer/RendererSpecification.hpp"
#include "Horizon/Renderer/Descriptors.hpp"

#include <vulkan/vulkan.h>

namespace Hz
{

    class VulkanImage;
	class VulkanPipeline;
    class VulkanUniformBuffer;
    class VulkanStorageBuffer;

	class VulkanDescriptorSet : public DescriptorSet
	{
	public:
		VulkanDescriptorSet(uint32_t setID, const std::vector<VkDescriptorSet>& sets);
		~VulkanDescriptorSet() = default;

		void Bind(Ref<Pipeline> pipeline, Ref<CommandBuffer> commandBuffer, PipelineBindPoint bindPoint, const std::vector<uint32_t>& dynamicOffsets) override;

		inline uint32_t GetSetID() const { return m_SetID; }
		inline const VkDescriptorSet GetVkDescriptorSet(uint32_t index) const { return m_DescriptorSets[index]; }

        void Upload(const std::vector<Uploadable>& elements) override;
        void UploadAll(const std::vector<Uploadable>& elements) override;
        void QueueUpload(const std::vector<Uploadable>& elements) override;

    private:
        void UploadImage(std::vector<VkWriteDescriptorSet>& writes, std::vector<VkDescriptorImageInfo>& imageInfos, Ref<VulkanImage> image, Descriptor descriptor, uint32_t arrayIndex, uint32_t frame);
        void UploadUniformBuffer(std::vector<VkWriteDescriptorSet>& writes, std::vector<VkDescriptorBufferInfo>& bufferInfos, Ref<VulkanUniformBuffer> buffer, Descriptor descriptor, uint32_t arrayIndex, uint32_t frame);
        void UploadStorageBuffer(std::vector<VkWriteDescriptorSet>& writes, std::vector<VkDescriptorBufferInfo>& bufferInfos, Ref<VulkanStorageBuffer> buffer, Descriptor descriptor, uint32_t arrayIndex, uint32_t frame);

	private:
		uint32_t m_SetID = 0;

		// Note: One for every frame in flight
		std::vector<VkDescriptorSet> m_DescriptorSets = { };
	};

	class VulkanDescriptorSets : public DescriptorSets
	{
	public:
		VulkanDescriptorSets(const std::initializer_list<DescriptorSetGroup>& specs);
		~VulkanDescriptorSets();

        void SetAmountOf(uint32_t setID, uint32_t amount) override;
		uint32_t GetAmountOf(uint32_t setID) const override;

		const DescriptorSetLayout& GetLayout(uint32_t setID) const override;
		std::vector<Ref<DescriptorSet>>& GetSets(uint32_t setID) override;

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
