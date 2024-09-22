#include "hzpch.h"
#include "VulkanRenderpass.hpp"

#include "Horizon/IO/Logging.hpp"

#include "Horizon/Renderer/Renderer.hpp"
#include "Horizon/Renderer/GraphicsContext.hpp"

#include "Horizon/Vulkan/VulkanUtils.hpp"
#include "Horizon/Vulkan/VulkanContext.hpp"
#include "Horizon/Vulkan/VulkanImage.hpp"
#include "Horizon/Vulkan/VulkanCommandBuffer.hpp"

namespace Hz
{

    VulkanRenderpass::VulkanRenderpass(RenderpassSpecification specs, Ref<CommandBuffer> commandBuffer)
        : m_Specification(specs), m_CommandBuffer(commandBuffer)
    {
        HZ_ASSERT(((!m_Specification.ColourAttachment.empty()) || m_Specification.DepthAttachment), "No Colour or Depth image passed in.");

        std::pair<uint32_t, uint32_t> size = GetSize();

        CreateRenderpass();
        CreateFramebuffers(size.first, size.second);
    }

    VulkanRenderpass::~VulkanRenderpass()
    {
        Destroy();
    }

    void VulkanRenderpass::Resize(uint32_t width, uint32_t height)
    {
        Renderer::Free([frameBuffers = m_Framebuffers]() 
        {
            auto device = VulkanContext::GetDevice()->GetVkDevice();

            for (auto& framebuffer : frameBuffers)
                vkDestroyFramebuffer(device, framebuffer, nullptr);
        });

        CreateFramebuffers(width, height);
    }

    std::pair<uint32_t, uint32_t> VulkanRenderpass::GetSize() const
    {
        std::pair<uint32_t, uint32_t> size = {};

        if (!m_Specification.ColourAttachment.empty())
        {
            size.first = m_Specification.ColourAttachment[0]->GetSpecification().Width;
            size.second = m_Specification.ColourAttachment[0]->GetSpecification().Height;
        }
        else if (m_Specification.DepthAttachment)
        {
            size.first = m_Specification.DepthAttachment->GetSpecification().Width;
            size.second = m_Specification.DepthAttachment->GetSpecification().Height;
        }

        return size;
    }

    void VulkanRenderpass::CreateRenderpass()
    {
        ///////////////////////////////////////////////////////////
        // Renderpass
        ///////////////////////////////////////////////////////////
        std::vector<VkAttachmentDescription> attachments = { };
        std::vector<VkAttachmentReference> attachmentRefs = { };

        if (!m_Specification.ColourAttachment.empty())
        {
            VkAttachmentDescription& colorAttachment = attachments.emplace_back();
            colorAttachment.format = (VkFormat)m_Specification.ColourAttachment[0]->GetSpecification().Format;
            colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            colorAttachment.loadOp = (VkAttachmentLoadOp)m_Specification.ColourLoadOp;
            colorAttachment.storeOp = (VkAttachmentStoreOp)m_Specification.ColourStoreOp;
            colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            colorAttachment.initialLayout = (VkImageLayout)m_Specification.PreviousColourImageLayout;
            colorAttachment.finalLayout = (VkImageLayout)m_Specification.FinalColourImageLayout;

            VkAttachmentReference& colorAttachmentRef = attachmentRefs.emplace_back();
            colorAttachmentRef.attachment = (uint32_t)(attachments.size() - 1);
            colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }

        if (m_Specification.DepthAttachment)
        {
            VkAttachmentDescription& depthAttachment = attachments.emplace_back();
            depthAttachment.format = (VkFormat)m_Specification.DepthAttachment->GetSpecification().Format;
            depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            depthAttachment.loadOp = (VkAttachmentLoadOp)m_Specification.DepthLoadOp;
            depthAttachment.storeOp = (VkAttachmentStoreOp)m_Specification.DepthStoreOp;
            depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAttachment.initialLayout = (VkImageLayout)m_Specification.PreviousDepthImageLayout;
            depthAttachment.finalLayout = (VkImageLayout)m_Specification.FinalDepthImageLayout;

            VkAttachmentReference& depthAttachmentRef = attachmentRefs.emplace_back();
            depthAttachmentRef.attachment = (uint32_t)(attachments.size() - 1);
            depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        if (!m_Specification.ColourAttachment.empty())
        {
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments = &attachmentRefs[0];
        }
        else
        {
            subpass.colorAttachmentCount = 0;
            subpass.pColorAttachments = nullptr;
        }

        if (m_Specification.DepthAttachment)
        {
            if (!m_Specification.ColourAttachment.empty())
                subpass.pDepthStencilAttachment = &attachmentRefs[1];
            else
                subpass.pDepthStencilAttachment = &attachmentRefs[0];
        }

        std::array<VkSubpassDependency, 2> dependencies = { };
        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[1].srcSubpass = 0;
        dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = (uint32_t)attachments.size();
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = (uint32_t)dependencies.size();
        renderPassInfo.pDependencies = dependencies.data();

        VK_CHECK_RESULT(vkCreateRenderPass(VulkanContext::GetDevice()->GetVkDevice(), &renderPassInfo, nullptr, &m_RenderPass));
    }

    void VulkanRenderpass::CreateFramebuffers(uint32_t width, uint32_t height)
    {
        ///////////////////////////////////////////////////////////
        // Framebuffers
        ///////////////////////////////////////////////////////////
        // Framebuffer creation
        m_Framebuffers.resize(VulkanContext::GetSwapChain()->GetSwapChainImages().size());
        for (size_t i = 0; i < m_Framebuffers.size(); i++)
        {
            std::vector<VkImageView> attachments = { };
            if (!m_Specification.ColourAttachment.empty())
            {
                if (m_Specification.ColourAttachment.size() == 1)
                {
                    Ref<VulkanImage> vkImage = m_Specification.ColourAttachment[0].As<VulkanImage>();
                    attachments.push_back(vkImage->GetVkImageView());
                }
                else // If the size is not equal to 1 it has to be equal to the amount of swapchain images
                {
                    Ref<VulkanImage> vkImage = m_Specification.ColourAttachment[i].As<VulkanImage>();
                    attachments.push_back(vkImage->GetVkImageView());
                }
            }
            if (m_Specification.DepthAttachment)
            {
                Ref<VulkanImage> vkImage = m_Specification.DepthAttachment.As<VulkanImage>();
                attachments.push_back(vkImage->GetVkImageView());
            }

            VkFramebufferCreateInfo framebufferInfo = {};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = m_RenderPass;
            framebufferInfo.attachmentCount = (uint32_t)attachments.size();
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = width;
            framebufferInfo.height = height;
            framebufferInfo.layers = 1;

            VK_CHECK_RESULT(vkCreateFramebuffer(VulkanContext::GetDevice()->GetVkDevice(), &framebufferInfo, nullptr, &m_Framebuffers[i]));
        }
    }

    void VulkanRenderpass::Destroy()
    {
        Renderer::Free([frameBuffers = m_Framebuffers, renderpass = m_RenderPass]()
        {
            auto device = VulkanContext::GetDevice()->GetVkDevice();

            for (auto& framebuffer : frameBuffers)
                vkDestroyFramebuffer(device, framebuffer, nullptr);

            vkDestroyRenderPass(device, renderpass, nullptr);
        });
    }

}
