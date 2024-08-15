#include "hzpch.h"
#include "VulkanPipeline.hpp"

#include "Horizon/Core/Logging.hpp"

#include "Horizon/Renderer/Renderer.hpp"
#include "Horizon/Renderer/GraphicsContext.hpp"
#include "Horizon/Renderer/Shader.hpp"
#include "Horizon/Renderer/Renderpass.hpp"
#include "Horizon/Renderer/Descriptors.hpp"

#include "Horizon/Vulkan/VulkanUtils.hpp"
#include "Horizon/Vulkan/VulkanShader.hpp"
#include "Horizon/Vulkan/VulkanContext.hpp"
#include "Horizon/Vulkan/VulkanBuffers.hpp"
#include "Horizon/Vulkan/VulkanRenderpass.hpp"
#include "Horizon/Vulkan/VulkanDescriptors.hpp"
#include "Horizon/Vulkan/VulkanCommandBuffer.hpp"

static VKAPI_ATTR VkResult VKAPI_CALL CreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks*  pAllocator, VkPipeline* pPipelines)
{
    auto func = (PFN_vkCreateRayTracingPipelinesKHR)vkGetInstanceProcAddr(Hz::VulkanContext::GetVkInstance(), "vkCreateRayTracingPipelinesKHR");

	if (func != nullptr)
		return func(device, deferredOperation, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);

	return VK_ERROR_EXTENSION_NOT_PRESENT;
}

static VKAPI_ATTR VkResult VKAPI_CALL CreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoNV* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines)
{
    auto func = (PFN_vkCreateRayTracingPipelinesNV)vkGetInstanceProcAddr(Hz::VulkanContext::GetVkInstance(), "vkCreateRayTracingPipelinesNV");

	if (func != nullptr)
		return func(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);

	return VK_ERROR_EXTENSION_NOT_PRESENT;
}

namespace Hz
{

    VulkanPipeline::VulkanPipeline(const PipelineSpecification& specs, Ref<DescriptorSets> sets, Ref<Shader> shader, Ref<Renderpass> renderpass)
        : m_Specification(specs)
    {
        HZ_ASSERT((specs.Type == PipelineType::Graphics), "Used pipeline graphics constructor but Type != PipelineType::Graphics");
        CreateGraphicsPipeline(sets, shader, renderpass);
    }

    VulkanPipeline::VulkanPipeline(const PipelineSpecification& specs, Ref<DescriptorSets> sets, Ref<Shader> shader)
        : m_Specification(specs)
    {
        switch (specs.Type)
        {
        case PipelineType::Graphics:
            CreateGraphicsPipeline(sets, shader, nullptr);
            break;
        case PipelineType::Compute:
            CreateComputePipeline(sets, shader);
            break;
        case PipelineType::RayTracingKHR:
            CreateRayTracingPipelineKHR(sets, shader);
            break;
        case PipelineType::RayTracingNV:
            CreateRayTracingPipelineNV(sets, shader);
            break;

        default:
            HZ_LOG_ERROR("Invalid PipelineType selected.");
            break;
        }
    }

    VulkanPipeline::~VulkanPipeline()
    {
        Renderer::Free([pipeline = m_Pipeline, pipelineLayout = m_PipelineLayout]()
        {
            auto device = VulkanContext::GetDevice()->GetVkDevice();

            vkDestroyPipeline(device, pipeline, nullptr);
            vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
        });
    }

    void VulkanPipeline::Use(Ref<CommandBuffer> commandBuffer, PipelineBindPoint bindPoint)
    {
        Ref<VulkanCommandBuffer> src = commandBuffer.As<VulkanCommandBuffer>();

		vkCmdBindPipeline(src->GetVkCommandBuffer(Renderer::GetCurrentFrame()), (VkPipelineBindPoint)bindPoint, m_Pipeline);
    }

    void VulkanPipeline::DispatchCompute(Ref<CommandBuffer> commandBuffer, uint32_t width, uint32_t height, uint32_t depth)
    {
        Ref<VulkanCommandBuffer> src = commandBuffer.As<VulkanCommandBuffer>();

		vkCmdDispatch(src->GetVkCommandBuffer(Renderer::GetCurrentFrame()), width, height, depth);
    }

    void VulkanPipeline::CreateGraphicsPipeline(Ref<DescriptorSets> sets, Ref<Shader> shader, Ref<Renderpass> renderpass) // Renderpass may be null
    {
        auto vkShader = shader.As<VulkanShader>();

		std::vector<VkPipelineShaderStageCreateInfo> shaderStages = { };
		if (vkShader->GetShaders().contains(ShaderStage::Vertex))
		{
			VkPipelineShaderStageCreateInfo& vertShaderStageInfo = shaderStages.emplace_back();
			vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
			vertShaderStageInfo.module = vkShader->GetShader(ShaderStage::Vertex);
			vertShaderStageInfo.pName = "main";
		}

		if (vkShader->GetShaders().contains(ShaderStage::Fragment))
		{
			VkPipelineShaderStageCreateInfo& fragShaderStageInfo = shaderStages.emplace_back();
			fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			fragShaderStageInfo.module = vkShader->GetShader(ShaderStage::Fragment);
			fragShaderStageInfo.pName = "main";
		}

		auto bindingDescription = GetBindingDescription();
		auto attributeDescriptions = GetAttributeDescriptions();

		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		if (!m_Specification.Bufferlayout.GetElements().empty())
		{
			vertexInputInfo.vertexBindingDescriptionCount = 1;
			vertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)attributeDescriptions.size();
			vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
			vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
		}
		else
		{
			vertexInputInfo.vertexBindingDescriptionCount = 0;
			vertexInputInfo.vertexAttributeDescriptionCount = 0;
			vertexInputInfo.pVertexAttributeDescriptions = nullptr;
			vertexInputInfo.pVertexBindingDescriptions = nullptr;
		}

		VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		VkPipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = (VkPolygonMode)(m_Specification.Polygonmode);
		rasterizer.lineWidth = m_Specification.LineWidth;
		rasterizer.cullMode = (VkCullModeFlags)(m_Specification.Cullingmode);
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = m_Specification.Blending; // Note(Jorben): Set true for transparancy

		VkPipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		VkPipelineDepthStencilStateCreateInfo depthStencil = {};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.minDepthBounds = 0.0f; // Optional
		depthStencil.maxDepthBounds = 1.0f; // Optional
		depthStencil.stencilTestEnable = VK_FALSE;
		depthStencil.front = {}; // Optional
		depthStencil.back = {}; // Optional

		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamicState = {};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = (uint32_t)dynamicStates.size();
		dynamicState.pDynamicStates = dynamicStates.data();

		// Descriptor layouts
		auto vkDescriptorSets = sets.As<VulkanDescriptorSets>();

		std::vector<VkDescriptorSetLayout> descriptorLayouts = { };
		descriptorLayouts.reserve(vkDescriptorSets->m_DescriptorLayouts.size());

		for (auto& pair : vkDescriptorSets->m_DescriptorLayouts)
			descriptorLayouts.push_back(pair.second);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.setLayoutCount = (uint32_t)descriptorLayouts.size();
		pipelineLayoutInfo.pSetLayouts = descriptorLayouts.data();

        VK_CHECK_RESULT(vkCreatePipelineLayout(VulkanContext::GetDevice()->GetVkDevice(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout));

		// Create the actual graphics pipeline (where we actually use the shaders and other info)
		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = (uint32_t)shaderStages.size();
		pipelineInfo.pStages = shaderStages.data();
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = m_PipelineLayout;
		pipelineInfo.renderPass = (renderpass ? renderpass.As<VulkanRenderpass>()->GetVkRenderPass() : nullptr);
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
		pipelineInfo.basePipelineIndex = -1; // Optional

        VK_CHECK_RESULT(vkCreateGraphicsPipelines(VulkanContext::GetDevice()->GetVkDevice(), VkUtils::Allocator::s_PipelineCache, 1, &pipelineInfo, nullptr, &m_Pipeline));
    }

    void VulkanPipeline::CreateComputePipeline(Ref<DescriptorSets> sets, Ref<Shader> shader)
    {
        auto vkShader = shader.As<VulkanShader>();

		VkPipelineShaderStageCreateInfo computeShaderStageInfo = {};
		computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		computeShaderStageInfo.module = vkShader->GetShader(ShaderStage::Compute);
		computeShaderStageInfo.pName = "main";

		auto vkDescriptorSets = sets.As<VulkanDescriptorSets>();

		std::vector<VkDescriptorSetLayout> descriptorLayouts;
		descriptorLayouts.reserve(vkDescriptorSets->m_DescriptorLayouts.size());

		for (auto& pair : vkDescriptorSets->m_DescriptorLayouts)
			descriptorLayouts.push_back(pair.second);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorLayouts.data();

        VK_CHECK_RESULT(vkCreatePipelineLayout(VulkanContext::GetDevice()->GetVkDevice(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout));

		VkComputePipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		pipelineInfo.stage = computeShaderStageInfo;
		pipelineInfo.layout = m_PipelineLayout;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.basePipelineIndex = -1;

        VK_CHECK_RESULT(vkCreateComputePipelines(VulkanContext::GetDevice()->GetVkDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_Pipeline));
    }

    void VulkanPipeline::CreateRayTracingPipelineKHR(Ref<DescriptorSets> sets, Ref<Shader> shader)
    {
        auto vkShader = shader.As<VulkanShader>();

        std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups = { };
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages = { };
        std::unordered_map<ShaderStage, uint32_t> shaderStageIndices = { };

        // Shader Stages
        uint32_t currentIndex = 0;
        if (vkShader->GetShaders().contains(ShaderStage::RayGenKHR))
        {
            VkPipelineShaderStageCreateInfo& raygenShaderStageInfo = shaderStages.emplace_back();
            raygenShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            raygenShaderStageInfo.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
            raygenShaderStageInfo.module = vkShader->GetShader(ShaderStage::RayGenKHR);
            raygenShaderStageInfo.pName = "main";

            shaderStageIndices[ShaderStage::RayGenKHR] = currentIndex++;
        }

        if (vkShader->GetShaders().contains(ShaderStage::MissKHR))
        {
            VkPipelineShaderStageCreateInfo& missShaderStageInfo = shaderStages.emplace_back();
            missShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            missShaderStageInfo.stage = VK_SHADER_STAGE_MISS_BIT_KHR;
            missShaderStageInfo.module = vkShader->GetShader(ShaderStage::MissKHR);
            missShaderStageInfo.pName = "main";

            shaderStageIndices[ShaderStage::MissKHR] = currentIndex++;
        }

        if (vkShader->GetShaders().contains(ShaderStage::ClosestHitKHR))
        {
            VkPipelineShaderStageCreateInfo& hitShaderStageInfo = shaderStages.emplace_back();
            hitShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            hitShaderStageInfo.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
            hitShaderStageInfo.module = vkShader->GetShader(ShaderStage::ClosestHitKHR);
            hitShaderStageInfo.pName = "main";

            shaderStageIndices[ShaderStage::ClosestHitKHR] = currentIndex++;
        }

        if (vkShader->GetShaders().contains(ShaderStage::AnyHitKHR))
        {
            VkPipelineShaderStageCreateInfo& anyHitShaderStageInfo = shaderStages.emplace_back();
            anyHitShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            anyHitShaderStageInfo.stage = VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
            anyHitShaderStageInfo.module = vkShader->GetShader(ShaderStage::AnyHitKHR);
            anyHitShaderStageInfo.pName = "main";

            shaderStageIndices[ShaderStage::AnyHitKHR] = currentIndex++;
        }

        if (vkShader->GetShaders().contains(ShaderStage::IntersectionKHR))
        {
            VkPipelineShaderStageCreateInfo& intersectionShaderStageInfo = shaderStages.emplace_back();
            intersectionShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            intersectionShaderStageInfo.stage = VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
            intersectionShaderStageInfo.module = vkShader->GetShader(ShaderStage::IntersectionKHR);
            intersectionShaderStageInfo.pName = "main";

            shaderStageIndices[ShaderStage::IntersectionKHR] = currentIndex++;
        }

        // Shader groups
        if (vkShader->GetShaders().contains(ShaderStage::RayGenKHR))
        {
            VkRayTracingShaderGroupCreateInfoKHR& raygenGroupInfo = shaderGroups.emplace_back();
            raygenGroupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
            raygenGroupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
            raygenGroupInfo.generalShader = shaderStageIndices[ShaderStage::RayGenKHR];
            raygenGroupInfo.closestHitShader = VK_SHADER_UNUSED_KHR;
            raygenGroupInfo.anyHitShader = VK_SHADER_UNUSED_KHR;
            raygenGroupInfo.intersectionShader = VK_SHADER_UNUSED_KHR;
        }

        if (vkShader->GetShaders().contains(ShaderStage::MissKHR))
        {
            VkRayTracingShaderGroupCreateInfoKHR& missGroupInfo = shaderGroups.emplace_back();
            missGroupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
            missGroupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
            missGroupInfo.generalShader = shaderStageIndices[ShaderStage::MissKHR];
            missGroupInfo.closestHitShader = VK_SHADER_UNUSED_KHR;
            missGroupInfo.anyHitShader = VK_SHADER_UNUSED_KHR;
            missGroupInfo.intersectionShader = VK_SHADER_UNUSED_KHR;
        }

        if (vkShader->GetShaders().contains(ShaderStage::ClosestHitKHR))
        {
            VkRayTracingShaderGroupCreateInfoKHR& hitGroupInfo = shaderGroups.emplace_back();
            hitGroupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
            hitGroupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
            hitGroupInfo.generalShader = VK_SHADER_UNUSED_KHR;
            hitGroupInfo.closestHitShader = shaderStageIndices[ShaderStage::ClosestHitKHR];
            hitGroupInfo.anyHitShader = VK_SHADER_UNUSED_KHR;
            hitGroupInfo.intersectionShader = VK_SHADER_UNUSED_KHR;
        }

        if (vkShader->GetShaders().contains(ShaderStage::AnyHitKHR))
        {
            VkRayTracingShaderGroupCreateInfoKHR& anyHitGroupInfo = shaderGroups.emplace_back();
            anyHitGroupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
            anyHitGroupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
            anyHitGroupInfo.generalShader = VK_SHADER_UNUSED_KHR;
            anyHitGroupInfo.closestHitShader = VK_SHADER_UNUSED_KHR;
            anyHitGroupInfo.anyHitShader = shaderStageIndices[ShaderStage::AnyHitKHR];
            anyHitGroupInfo.intersectionShader = VK_SHADER_UNUSED_KHR;
        }

        if (vkShader->GetShaders().contains(ShaderStage::IntersectionKHR))
        {
            VkRayTracingShaderGroupCreateInfoKHR& intersectionGroupInfo = shaderGroups.emplace_back();
            intersectionGroupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
            intersectionGroupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR;
            intersectionGroupInfo.generalShader = VK_SHADER_UNUSED_KHR;
            intersectionGroupInfo.closestHitShader = VK_SHADER_UNUSED_KHR;
            intersectionGroupInfo.anyHitShader = VK_SHADER_UNUSED_KHR;
            intersectionGroupInfo.intersectionShader = shaderStageIndices[ShaderStage::IntersectionKHR];
        }

        // Descriptor layouts
        auto vkDescriptorSets = sets.As<VulkanDescriptorSets>();

        std::vector<VkDescriptorSetLayout> descriptorLayouts;
        descriptorLayouts.reserve(vkDescriptorSets->m_DescriptorLayouts.size());

        for (auto& pair : vkDescriptorSets->m_DescriptorLayouts)
            descriptorLayouts.push_back(pair.second);

        // Layout create info
        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorLayouts.data();

        VK_CHECK_RESULT(vkCreatePipelineLayout(VulkanContext::GetDevice()->GetVkDevice(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout));

        // Pipeline create info
        VkRayTracingPipelineCreateInfoKHR rayTracingPipelineCreateInfo = {};
        rayTracingPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
        rayTracingPipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
        rayTracingPipelineCreateInfo.pStages = shaderStages.data();
        rayTracingPipelineCreateInfo.groupCount = static_cast<uint32_t>(shaderGroups.size());
        rayTracingPipelineCreateInfo.pGroups = shaderGroups.data();
        rayTracingPipelineCreateInfo.maxPipelineRayRecursionDepth = m_Specification.MaxRayRecursion;
        rayTracingPipelineCreateInfo.pLibraryInfo = nullptr;
        rayTracingPipelineCreateInfo.pLibraryInterface = nullptr;
        rayTracingPipelineCreateInfo.pDynamicState = nullptr;
        rayTracingPipelineCreateInfo.flags = 0; // Adjust as needed

        // Create the ray tracing pipeline
        VK_CHECK_RESULT(CreateRayTracingPipelinesKHR(VulkanContext::GetDevice()->GetVkDevice(), VK_NULL_HANDLE, VkUtils::Allocator::s_PipelineCache, 1, &rayTracingPipelineCreateInfo, nullptr, &m_Pipeline));
    }

    void VulkanPipeline::CreateRayTracingPipelineNV(Ref<DescriptorSets> sets, Ref<Shader> shader)
    {
        auto vkShader = shader.As<VulkanShader>();

        std::vector<VkRayTracingShaderGroupCreateInfoNV> shaderGroups;
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
        std::unordered_map<ShaderStage, uint32_t> shaderStageIndices;

        // Shader Stages
        uint32_t currentIndex = 0;
        if (vkShader->GetShaders().contains(ShaderStage::RayGenNV))
        {
            VkPipelineShaderStageCreateInfo& raygenShaderStageInfo = shaderStages.emplace_back();
            raygenShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            raygenShaderStageInfo.stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
            raygenShaderStageInfo.module = vkShader->GetShader(ShaderStage::RayGenNV);
            raygenShaderStageInfo.pName = "main";

            shaderStageIndices[ShaderStage::RayGenNV] = currentIndex++;
        }

        if (vkShader->GetShaders().contains(ShaderStage::MissNV))
        {
            VkPipelineShaderStageCreateInfo& missShaderStageInfo = shaderStages.emplace_back();
            missShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            missShaderStageInfo.stage = VK_SHADER_STAGE_MISS_BIT_NV;
            missShaderStageInfo.module = vkShader->GetShader(ShaderStage::MissNV);
            missShaderStageInfo.pName = "main";

            shaderStageIndices[ShaderStage::MissNV] = currentIndex++;
        }

        if (vkShader->GetShaders().contains(ShaderStage::ClosestHitNV))
        {
            VkPipelineShaderStageCreateInfo& hitShaderStageInfo = shaderStages.emplace_back();
            hitShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            hitShaderStageInfo.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
            hitShaderStageInfo.module = vkShader->GetShader(ShaderStage::ClosestHitNV);
            hitShaderStageInfo.pName = "main";

            shaderStageIndices[ShaderStage::ClosestHitNV] = currentIndex++;
        }

        if (vkShader->GetShaders().contains(ShaderStage::AnyHitNV))
        {
            VkPipelineShaderStageCreateInfo& anyHitShaderStageInfo = shaderStages.emplace_back();
            anyHitShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            anyHitShaderStageInfo.stage = VK_SHADER_STAGE_ANY_HIT_BIT_NV;
            anyHitShaderStageInfo.module = vkShader->GetShader(ShaderStage::AnyHitNV);
            anyHitShaderStageInfo.pName = "main";

            shaderStageIndices[ShaderStage::AnyHitKHR] = currentIndex++;
        }

        if (vkShader->GetShaders().contains(ShaderStage::IntersectionNV))
        {
            VkPipelineShaderStageCreateInfo& intersectionShaderStageInfo = shaderStages.emplace_back();
            intersectionShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            intersectionShaderStageInfo.stage = VK_SHADER_STAGE_INTERSECTION_BIT_NV;
            intersectionShaderStageInfo.module = vkShader->GetShader(ShaderStage::IntersectionNV);
            intersectionShaderStageInfo.pName = "main";

            shaderStageIndices[ShaderStage::IntersectionNV] = currentIndex++;
        }

        // Shader groups
        if (vkShader->GetShaders().contains(ShaderStage::RayGenNV))
        {
            VkRayTracingShaderGroupCreateInfoNV& raygenGroupInfo = shaderGroups.emplace_back();
            raygenGroupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
            raygenGroupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
            raygenGroupInfo.generalShader = shaderStageIndices[ShaderStage::RayGenNV];
            raygenGroupInfo.closestHitShader = VK_SHADER_UNUSED_NV;
            raygenGroupInfo.anyHitShader = VK_SHADER_UNUSED_NV;
            raygenGroupInfo.intersectionShader = VK_SHADER_UNUSED_NV;
        }

        if (vkShader->GetShaders().contains(ShaderStage::MissNV))
        {
            VkRayTracingShaderGroupCreateInfoNV& missGroupInfo = shaderGroups.emplace_back();
            missGroupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
            missGroupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
            missGroupInfo.generalShader = shaderStageIndices[ShaderStage::MissNV];
            missGroupInfo.closestHitShader = VK_SHADER_UNUSED_NV;
            missGroupInfo.anyHitShader = VK_SHADER_UNUSED_NV;
            missGroupInfo.intersectionShader = VK_SHADER_UNUSED_NV;
        }

        if (vkShader->GetShaders().contains(ShaderStage::ClosestHitNV))
        {
            VkRayTracingShaderGroupCreateInfoNV& hitGroupInfo = shaderGroups.emplace_back();
            hitGroupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
            hitGroupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
            hitGroupInfo.generalShader = VK_SHADER_UNUSED_NV;
            hitGroupInfo.closestHitShader = shaderStageIndices[ShaderStage::ClosestHitNV];
            hitGroupInfo.anyHitShader = VK_SHADER_UNUSED_NV;
            hitGroupInfo.intersectionShader = VK_SHADER_UNUSED_NV;
        }

        if (vkShader->GetShaders().contains(ShaderStage::AnyHitNV))
        {
            VkRayTracingShaderGroupCreateInfoNV& anyHitGroupInfo = shaderGroups.emplace_back();
            anyHitGroupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
            anyHitGroupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
            anyHitGroupInfo.generalShader = VK_SHADER_UNUSED_NV;
            anyHitGroupInfo.closestHitShader = VK_SHADER_UNUSED_NV;
            anyHitGroupInfo.anyHitShader = shaderStageIndices[ShaderStage::AnyHitNV];
            anyHitGroupInfo.intersectionShader = VK_SHADER_UNUSED_NV;
        }

        if (vkShader->GetShaders().contains(ShaderStage::IntersectionNV))
        {
            VkRayTracingShaderGroupCreateInfoNV& intersectionGroupInfo = shaderGroups.emplace_back();
            intersectionGroupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
            intersectionGroupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_NV;
            intersectionGroupInfo.generalShader = VK_SHADER_UNUSED_NV;
            intersectionGroupInfo.closestHitShader = VK_SHADER_UNUSED_NV;
            intersectionGroupInfo.anyHitShader = VK_SHADER_UNUSED_NV;
            intersectionGroupInfo.intersectionShader = shaderStageIndices[ShaderStage::IntersectionNV];
        }

        // Descriptor layouts
        auto vkDescriptorSets = sets.As<VulkanDescriptorSets>();

        std::vector<VkDescriptorSetLayout> descriptorLayouts;
        descriptorLayouts.reserve(vkDescriptorSets->m_DescriptorLayouts.size());

        for (auto& pair : vkDescriptorSets->m_DescriptorLayouts)
            descriptorLayouts.push_back(pair.second);

        // Layout create info
        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorLayouts.data();

        VK_CHECK_RESULT(vkCreatePipelineLayout(VulkanContext::GetDevice()->GetVkDevice(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout));

        // Pipeline create info
        VkRayTracingPipelineCreateInfoNV rayTracingPipelineCreateInfo = {};
        rayTracingPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV;
        rayTracingPipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
        rayTracingPipelineCreateInfo.pStages = shaderStages.data();
        rayTracingPipelineCreateInfo.groupCount = static_cast<uint32_t>(shaderGroups.size());
        rayTracingPipelineCreateInfo.pGroups = shaderGroups.data();
        rayTracingPipelineCreateInfo.maxRecursionDepth = m_Specification.MaxRayRecursion;
        rayTracingPipelineCreateInfo.flags = 0; // Adjust as needed

        // Create the ray tracing pipeline
        VK_CHECK_RESULT(CreateRayTracingPipelinesNV(VulkanContext::GetDevice()->GetVkDevice(), VkUtils::Allocator::s_PipelineCache, 1, &rayTracingPipelineCreateInfo, nullptr, &m_Pipeline));
    }

    VkVertexInputBindingDescription VulkanPipeline::GetBindingDescription()
	{
		VkVertexInputBindingDescription description = {};
		description.binding = 0;
		description.stride = m_Specification.Bufferlayout.GetStride();
		description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return description;
	}

	std::vector<VkVertexInputAttributeDescription> VulkanPipeline::GetAttributeDescriptions()
	{
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions = {};
		attributeDescriptions.resize(m_Specification.Bufferlayout.GetElements().size());

		auto& elements = m_Specification.Bufferlayout.GetElements();
		for (size_t i = 0; i < elements.size(); i++)
		{
			attributeDescriptions[i].binding = 0;
			attributeDescriptions[i].location = elements[i].Location;
			attributeDescriptions[i].format = DataTypeToVkFormat(elements[i].Type);
			attributeDescriptions[i].offset = (uint32_t)elements[i].Offset;
		}

		return attributeDescriptions;
	}

}
