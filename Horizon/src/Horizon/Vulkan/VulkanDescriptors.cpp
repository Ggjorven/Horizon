#include "hzpch.h"
#include "VulkanDescriptors.hpp"

#include "Horizon/Core/Logging.hpp"

#include "Horizon/Renderer/Renderer.hpp"
#include "Horizon/Renderer/GraphicsContext.hpp"
#include "Horizon/Renderer/Pipeline.hpp"

#include "Horizon/Vulkan/VulkanUtils.hpp"
#include "Horizon/Vulkan/VulkanImage.hpp"
#include "Horizon/Vulkan/VulkanBuffers.hpp"
#include "Horizon/Vulkan/VulkanContext.hpp"
#include "Horizon/Vulkan/VulkanPipeline.hpp"
#include "Horizon/Vulkan/VulkanCommandBuffer.hpp"

#include <Pulse/Types/TypeUtils.hpp>

namespace Hz
{

    VulkanDescriptorSet::VulkanDescriptorSet(uint32_t setID, const std::vector<VkDescriptorSet>& sets)
        : m_SetID(setID), m_DescriptorSets(sets)
    {
    }

    void VulkanDescriptorSet::Bind(Ref<Pipeline> pipeline, Ref<CommandBuffer> commandBuffer, PipelineBindPoint bindPoint, const std::vector<uint32_t>& dynamicOffsets)
    {
		auto vkPipelineLayout = pipeline.As<VulkanPipeline>()->GetVkPipelineLayout();
		auto vkCmdBuf = commandBuffer.As<VulkanCommandBuffer>()->GetVkCommandBuffer(Renderer::GetCurrentFrame());

		vkCmdBindDescriptorSets(vkCmdBuf, (VkPipelineBindPoint)bindPoint, vkPipelineLayout, m_SetID, 1, &m_DescriptorSets[Renderer::GetCurrentFrame()], static_cast<uint32_t>(dynamicOffsets.size()), dynamicOffsets.data());
	}

    void VulkanDescriptorSet::Upload(const std::initializer_list<Uploadable>& elements)
    {
        std::vector<VkWriteDescriptorSet> writes;
        writes.reserve(elements.size() * (size_t)Renderer::GetSpecification().Buffers);

        std::vector<VkDescriptorImageInfo> imageInfos = {};
        std::vector<VkDescriptorBufferInfo> bufferInfos = {};

        for (auto& [uploadable, descriptor] : elements)
        {
            std::visit([&](auto&& arg)
            {
                using T = Pulse::Types::Clean<decltype(arg)>;

                if (std::is_same_v<T, Ref<Image>>)                  UploadImage(writes, imageInfos, arg, descriptor);
                else if (std::is_same_v<T, Ref<UniformBuffer>>)     UploadUniformBuffer(writes, bufferInfos, arg, descriptor);
                else if (std::is_same_v<T, Ref<StorageBuffer>>)     UploadStorageBuffer(writes, bufferInfos, arg, descriptor);
            }, uploadable);
        }

        vkUpdateDescriptorSets(VulkanContext::GetDevice()->GetVkDevice(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
    }

    void VulkanDescriptorSet::UploadImage(std::vector<VkWriteDescriptorSet>& writes, std::vector<VkDescriptorImageInfo>& imageInfos, Ref<Image> image, Descriptor descriptor)
    {
        Ref<VulkanImage> src = image.As<VulkanImage>();

		const size_t framesInFlight = (size_t)Renderer::GetSpecification().Buffers;
		for (size_t i = 0; i < framesInFlight; i++)
		{
			VkDescriptorImageInfo& imageInfo = imageInfos.emplace_back();
			imageInfo.imageLayout = (VkImageLayout)src->m_Specification.Layout;
			imageInfo.imageView = src->m_ImageView;
			imageInfo.sampler = src->m_Sampler;

			VkWriteDescriptorSet& descriptorWrite = writes.emplace_back();
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = m_DescriptorSets[i];
			descriptorWrite.dstBinding = descriptor.Binding;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = (VkDescriptorType)descriptor.Type;
			descriptorWrite.descriptorCount = descriptor.Count;
			descriptorWrite.pImageInfo = &imageInfo;
		}
    }

    void VulkanDescriptorSet::UploadUniformBuffer(std::vector<VkWriteDescriptorSet>& writes, std::vector<VkDescriptorBufferInfo>& bufferInfos, Ref<UniformBuffer> buffer, Descriptor descriptor)
    {
        Ref<VulkanUniformBuffer> src = buffer.As<VulkanUniformBuffer>();

		const size_t framesInFlight = (size_t)Renderer::GetSpecification().Buffers;
		for (size_t i = 0; i < framesInFlight; i++)
		{
			VkDescriptorBufferInfo& bufferInfo = bufferInfos.emplace_back();
			bufferInfo.buffer = src->m_Buffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = src->m_Size;

			VkWriteDescriptorSet& descriptorWrite = writes.emplace_back();
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = m_DescriptorSets[i];
			descriptorWrite.dstBinding = descriptor.Binding;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = descriptor.Count;
			descriptorWrite.pBufferInfo = &bufferInfo;
		}
    }

    void VulkanDescriptorSet::UploadStorageBuffer(std::vector<VkWriteDescriptorSet>& writes, std::vector<VkDescriptorBufferInfo>& bufferInfos, Ref<StorageBuffer> buffer, Descriptor descriptor)
    {
        Ref<VulkanStorageBuffer> src = buffer.As<VulkanStorageBuffer>();

		const size_t framesInFlight = (size_t)Renderer::GetSpecification().Buffers;
		for (size_t i = 0; i < framesInFlight; i++)
		{
			VkDescriptorBufferInfo& bufferInfo = bufferInfos.emplace_back();
			bufferInfo.buffer = src->m_Buffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = src->m_Size;

			VkWriteDescriptorSet& descriptorWrite = writes.emplace_back();
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = m_DescriptorSets[i];
			descriptorWrite.dstBinding = descriptor.Binding;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			descriptorWrite.descriptorCount = descriptor.Count;
			descriptorWrite.pBufferInfo = &bufferInfo;
		}
    }

    VulkanDescriptorSets::VulkanDescriptorSets(const std::initializer_list<DescriptorSetGroup>& specs)
    {
        for (auto& group : specs)
		{
			m_OriginalLayouts[group.Layout.SetID] = group.Layout;

			CreateDescriptorSetLayout(group.Layout.SetID);
			CreateDescriptorPool(group.Layout.SetID, group.Amount);
			CreateDescriptorSets(group.Layout.SetID, group.Amount);
		}
    }

    VulkanDescriptorSets::~VulkanDescriptorSets()
    {
        Renderer::Free([descriptorPools = m_DescriptorPools, descriptorLayouts = m_DescriptorLayouts]()
        {
            auto device = VulkanContext::GetDevice()->GetVkDevice();

            for (auto& pool : descriptorPools)
                vkDestroyDescriptorPool(device, pool.second, nullptr);

            for (auto& layout : descriptorLayouts)
                vkDestroyDescriptorSetLayout(device, layout.second, nullptr);
        });
    }

    void VulkanDescriptorSets::SetAmountOf(uint32_t setID, uint32_t amount)
    {
        vkDestroyDescriptorPool(VulkanContext::GetDevice()->GetVkDevice(), m_DescriptorPools[setID], nullptr);

		CreateDescriptorPool(setID, amount);
		CreateDescriptorSets(setID, amount);
    }

    uint32_t VulkanDescriptorSets::GetAmountOf(uint32_t setID) const
    {
        auto it = m_DescriptorSets.find(setID);
		if (it == m_DescriptorSets.end())
		{
			HZ_LOG_ERROR("Failed to find descriptor set by ID: {0}", setID);
			return 0;
		}

		return (uint32_t)it->second.size();
    }

    const DescriptorSetLayout& VulkanDescriptorSets::GetLayout(uint32_t setID) const
    {
        auto it = m_OriginalLayouts.find(setID);
        HZ_VERIFY((it != m_OriginalLayouts.end()), "Failed to find descriptor set by ID: {0}", setID)

		return it->second;
    }

    std::vector<Ref<DescriptorSet>>& VulkanDescriptorSets::GetSets(uint32_t setID)
    {
        auto it = m_DescriptorSets.find(setID);
        HZ_VERIFY((it != m_DescriptorSets.end()), "Failed to find descriptor set by ID: {0}", setID)

		return it->second;
    }

    void VulkanDescriptorSets::CreateDescriptorSetLayout(uint32_t setID)
    {
        std::vector<VkDescriptorSetLayoutBinding> layouts = { };

		for (auto& element : m_OriginalLayouts[setID].Descriptors)
		{
			VkDescriptorSetLayoutBinding& layoutBinding = layouts.emplace_back();
			layoutBinding.binding = element.second.Binding;
			layoutBinding.descriptorType = (VkDescriptorType)element.second.Type;
			layoutBinding.descriptorCount = element.second.Count;
			layoutBinding.stageFlags = (VkShaderStageFlags)element.second.Stage;
			layoutBinding.pImmutableSamplers = nullptr; // Optional
		}

		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = (uint32_t)layouts.size();
		layoutInfo.pBindings = layouts.data();

		m_DescriptorLayouts[setID] = VK_NULL_HANDLE;
        VK_CHECK_RESULT(vkCreateDescriptorSetLayout(VulkanContext::GetDevice()->GetVkDevice(), &layoutInfo, nullptr, &m_DescriptorLayouts[setID]));
    }

    void VulkanDescriptorSets::CreateDescriptorPool(uint32_t setID, uint32_t amount)
    {
		// Note: Just for myself, the poolSizes is just the amount of elements of a certain type to able to allocate per pool
		std::vector<VkDescriptorPoolSize> poolSizes = { };
		poolSizes.reserve(m_OriginalLayouts[setID].UniqueTypes().size());

		const uint32_t framesInFlight = (uint32_t)Renderer::GetSpecification().Buffers;
		for (auto& type : m_OriginalLayouts[setID].UniqueTypes())
		{
			VkDescriptorPoolSize& poolSize = poolSizes.emplace_back();
			poolSize.type = (VkDescriptorType)type;
			poolSize.descriptorCount = m_OriginalLayouts[setID].AmountOf(type) * framesInFlight * amount;
		}

		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = (uint32_t)poolSizes.size();
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = framesInFlight * amount; // A set for every frame in flight

		m_DescriptorPools[setID] = VK_NULL_HANDLE;
        VK_CHECK_RESULT(vkCreateDescriptorPool(VulkanContext::GetDevice()->GetVkDevice(), &poolInfo, nullptr, &m_DescriptorPools[setID]));
    }

    void VulkanDescriptorSets::CreateDescriptorSets(uint32_t setID, uint32_t amount)
    {
		const uint32_t framesInFlight = (uint32_t)Renderer::GetSpecification().Buffers;

		std::vector<VkDescriptorSet> descriptorSets = { };
		std::vector<VkDescriptorSetLayout> layouts((size_t)framesInFlight * amount, m_DescriptorLayouts[setID]);

		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = m_DescriptorPools[setID];
		allocInfo.descriptorSetCount = framesInFlight * amount;
		allocInfo.pSetLayouts = layouts.data();

		descriptorSets.resize((size_t)framesInFlight * amount);

        VK_CHECK_RESULT(vkAllocateDescriptorSets(VulkanContext::GetDevice()->GetVkDevice(), &allocInfo, descriptorSets.data()));
		ConvertToVulkanDescriptorSets(setID, amount, descriptorSets);
    }

    void VulkanDescriptorSets::ConvertToVulkanDescriptorSets(uint32_t setID, uint32_t amount, std::vector<VkDescriptorSet>& sets)
    {
        m_DescriptorSets[setID].resize((size_t)amount);

		const uint32_t framesInFlight = (uint32_t)Renderer::GetSpecification().Buffers;

		uint32_t index = 0;
		for (uint32_t i = 0; i < amount; i++)
		{
			std::vector<VkDescriptorSet> setCombo = { };

			for (uint32_t j = 0; j < framesInFlight; j++)
				setCombo.push_back(sets[index + j]);

            m_DescriptorSets[setID][i] = Ref<VulkanDescriptorSet>::Create(setID, setCombo);
			index += framesInFlight;
		}
    }

}
