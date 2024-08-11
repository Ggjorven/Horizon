#include "hzpch.h"
#include "VulkanDescriptors.hpp"

#include "Horizon/Core/Logging.hpp"

#include "Horizon/Renderer/GraphicsContext.hpp"
#include "Horizon/Renderer/Renderer.hpp"
#include "Horizon/Renderer/Pipeline.hpp"

#include "Horizon/Vulkan/VulkanUtils.hpp"
#include "Horizon/Vulkan/VulkanContext.hpp"
#include "Horizon/Vulkan/VulkanPipeline.hpp"
#include "Horizon/Vulkan/VulkanCommandBuffer.hpp"

namespace Hz
{

    VulkanDescriptorSet::VulkanDescriptorSet(uint32_t setID, const std::vector<VkDescriptorSet>& sets)
        : m_SetID(setID), m_DescriptorSets(sets)
    {
    }

    void VulkanDescriptorSet::Bind(Ref<Pipeline> pipeline, Ref<CommandBuffer> commandBuffer, PipelineBindPoint bindPoint, const std::vector<uint32_t>& dynamicOffsets)
    {
		auto vkPipelineLayout = HzCast(VulkanPipeline, pipeline->Src())->GetVkPipelineLayout();
		auto vkCmdBuf = HzCast(VulkanCommandBuffer, commandBuffer->Src())->GetVkCommandBuffer(Renderer::GetCurrentFrame());

		vkCmdBindDescriptorSets(vkCmdBuf, (VkPipelineBindPoint)bindPoint, vkPipelineLayout, m_SetID, 1, &m_DescriptorSets[Renderer::GetCurrentFrame()], static_cast<uint32_t>(dynamicOffsets.size()), dynamicOffsets.data());
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
        const VulkanContext& context = *HzCast(VulkanContext, GraphicsContext::Src());

        for (auto& pool : m_DescriptorPools)
				vkDestroyDescriptorPool(context.GetDevice()->GetVkDevice(), pool.second, nullptr);

        for (auto& layout : m_DescriptorLayouts)
            vkDestroyDescriptorSetLayout(context.GetDevice()->GetVkDevice(), layout.second, nullptr);
    }

    void VulkanDescriptorSets::SetAmountOf(uint32_t setID, uint32_t amount)
    {
        const VulkanContext& context = *HzCast(VulkanContext, GraphicsContext::Src());

        vkDestroyDescriptorPool(context.GetDevice()->GetVkDevice(), m_DescriptorPools[setID], nullptr);

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

    const DescriptorSetLayout& VulkanDescriptorSets::GetLayout(uint32_t setID)
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
        const VulkanContext& context = *HzCast(VulkanContext, GraphicsContext::Src());

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
        VK_CHECK_RESULT(vkCreateDescriptorSetLayout(context.GetDevice()->GetVkDevice(), &layoutInfo, nullptr, &m_DescriptorLayouts[setID]));
    }

    void VulkanDescriptorSets::CreateDescriptorPool(uint32_t setID, uint32_t amount)
    {
        const VulkanContext& context = *HzCast(VulkanContext, GraphicsContext::Src());

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
        VK_CHECK_RESULT(vkCreateDescriptorPool(context.GetDevice()->GetVkDevice(), &poolInfo, nullptr, &m_DescriptorPools[setID]));
    }

    void VulkanDescriptorSets::CreateDescriptorSets(uint32_t setID, uint32_t amount)
    {
        const VulkanContext& context = *HzCast(VulkanContext, GraphicsContext::Src());

		const uint32_t framesInFlight = (uint32_t)Renderer::GetSpecification().Buffers;

		std::vector<VkDescriptorSet> descriptorSets = { };
		std::vector<VkDescriptorSetLayout> layouts((size_t)framesInFlight * amount, m_DescriptorLayouts[setID]);

		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = m_DescriptorPools[setID];
		allocInfo.descriptorSetCount = framesInFlight * amount;
		allocInfo.pSetLayouts = layouts.data();

		descriptorSets.resize((size_t)framesInFlight * amount);

        VK_CHECK_RESULT(vkAllocateDescriptorSets(context.GetDevice()->GetVkDevice(), &allocInfo, descriptorSets.data()));
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

            m_DescriptorSets[setID][i] = Ref<DescriptorSet>::Create(new VulkanDescriptorSet(setID, setCombo));
			index += framesInFlight;
		}
    }

}
