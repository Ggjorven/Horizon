#include "hzpch.h"
#include "Descriptors.hpp"

#include "Horizon/Core/Logging.hpp"

#include "Horizon/Renderer/Renderer.hpp"
#include "Horizon/Renderer/GraphicsContext.hpp"
#include "Horizon/Renderer/Pipeline.hpp"

#include "Horizon/Vulkan/VulkanDescriptors.hpp"

namespace Hz
{

    ///////////////////////////////////////////////////////////
    // Specifications
    ///////////////////////////////////////////////////////////
    Descriptor::Descriptor(DescriptorType type, uint32_t binding, const std::string& name, ShaderStage stage, uint32_t count)
        : Type(type), Binding(binding), Name(name), Stage(stage), Count(count)
    {
    }

    DescriptorSetLayout::DescriptorSetLayout(uint32_t setID, const std::vector<Descriptor>& descriptors)
        : SetID(setID)
	{
		for (auto& descriptor : descriptors)
			Descriptors[descriptor.Name] = descriptor;
	}

    DescriptorSetLayout::DescriptorSetLayout(uint32_t setID, const std::initializer_list<Descriptor>& descriptors)
        : DescriptorSetLayout(setID, std::vector(descriptors))
	{
	}

    Descriptor DescriptorSetLayout::GetDescriptorByName(const std::string& name) const
    {
        auto it = Descriptors.find(name);
		if (it == Descriptors.end())
		{
			HZ_LOG_ERROR("Failed to find descriptor by name: '{0}'", name);
			return {};
		}

		return it->second;
    }

    std::unordered_set<DescriptorType> DescriptorSetLayout::UniqueTypes() const
    {
        std::unordered_set<DescriptorType> unique = { };

		for (const auto& e : Descriptors)
			unique.insert(e.second.Type);

		return unique;
    }

    uint32_t DescriptorSetLayout::AmountOf(DescriptorType type) const
    {
        uint32_t count = 0;

		for (const auto& e : Descriptors)
		{
			if (e.second.Type == type)
				count++;
		}

		return count;
    }

    DescriptorSetGroup::DescriptorSetGroup(uint32_t amount, const DescriptorSetLayout& layout)
        : Amount(amount), Layout(layout)
    {
    }

    Uploadable::Uploadable(Type value, Descriptor element)
        : Value(value), Element(element)
    {
    }

    ///////////////////////////////////////////////////////////
    // Core class
    ///////////////////////////////////////////////////////////
    DescriptorSet::DescriptorSet(DescriptorSetType* src)
        : m_Instance(src)
    {
    }

    DescriptorSet::~DescriptorSet()
    {
        delete m_Instance;
    }

    void DescriptorSet::Bind(Ref<Pipeline> pipeline, Ref<CommandBuffer> commandBuffer, PipelineBindPoint bindPoint, const std::vector<uint32_t>& dynamicOffsets)
    {
        m_Instance->Bind(pipeline, commandBuffer, bindPoint, dynamicOffsets);
    }

    void DescriptorSet::Upload(const std::initializer_list<Uploadable>& elements)
    {
        m_Instance->Upload(elements);
    }

    DescriptorSets::DescriptorSets(const std::initializer_list<DescriptorSetGroup>& specs)
        : m_Instance(new DescriptorSetsType(std::move(specs)))
    {
    }

    DescriptorSets::DescriptorSets(DescriptorSetsType* src)
        : m_Instance(src)
    {
    }

    DescriptorSets::~DescriptorSets()
    {
        delete m_Instance;
    }

    void DescriptorSets::SetAmountOf(uint32_t setID, uint32_t amount)
    {
        m_Instance->SetAmountOf(setID, amount);
    }

    uint32_t DescriptorSets::GetAmountOf(uint32_t setID) const
    {
        return m_Instance->GetAmountOf(setID);
    }

    const DescriptorSetLayout& DescriptorSets::GetLayout(uint32_t setID) const
    {
        return m_Instance->GetLayout(setID);
    }

    std::vector<Ref<DescriptorSet>>& DescriptorSets::GetSets(uint32_t setID)
    {
        return m_Instance->GetSets(setID);
    }

    Ref<DescriptorSets> DescriptorSets::Create(const std::initializer_list<DescriptorSetGroup>& specs)
    {
        return Ref<DescriptorSets>::Create(std::move(specs));
    }

}
