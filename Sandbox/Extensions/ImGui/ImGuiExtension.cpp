#include "ImGuiExtension.hpp"

#include "Horizon/Core/Logging.hpp"
#include "Horizon/Core/Window.hpp"

#include "Horizon/Vulkan/VulkanUtils.hpp"
#include "Horizon/Vulkan/VulkanContext.hpp"
#include "Horizon/Vulkan/VulkanRenderer.hpp"
#include "Horizon/Vulkan/VulkanRenderpass.hpp"

#include <GLFW/glfw3.h>

#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_vulkan.h"

using namespace Hz;

static VkDescriptorPool s_ImGuiPool = VK_NULL_HANDLE;

void ImGuiExtension::OnInitEnd()
{
	{
		std::vector<VkDescriptorPoolSize> poolSizes =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};
		
		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = (uint32_t)poolSizes.size();
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = 1000 * (uint32_t)Renderer::GetSpecification().Buffers;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

		VK_CHECK_RESULT(vkCreateDescriptorPool(VulkanContext::GetDevice()->GetVkDevice(), &poolInfo, nullptr, &s_ImGuiPool));
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO(); (void)io;

	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

	io.IniFilename = NULL;

	ImGui::StyleColorsDark();

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	ImGuiStyle& style = ImGui::GetStyle();

	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	GLFWwindow* window = static_cast<GLFWwindow*>(Window::Get().GetNativeWindow());
	ImGui_ImplGlfw_InitForVulkan(window, true);

	ImGui_ImplVulkan_InitInfo  initInfo = {};
	initInfo.Instance = VulkanContext::GetVkInstance();
	initInfo.PhysicalDevice = VulkanContext::GetPhysicalDevice()->GetVkPhysicalDevice();
	initInfo.Device = VulkanContext::GetDevice()->GetVkDevice();
	initInfo.QueueFamily = QueueFamilyIndices::Find(VulkanContext::GetSwapChain()->GetVkSurface(), VulkanContext::GetPhysicalDevice()->GetVkPhysicalDevice()).GraphicsFamily.value();
	initInfo.Queue = VulkanContext::GetDevice()->GetGraphicsQueue();
	initInfo.PipelineCache = VkUtils::Allocator::s_PipelineCache;
	initInfo.DescriptorPool = s_ImGuiPool;
	initInfo.Allocator = nullptr; // Optional, use nullptr to use the default allocator
	initInfo.MinImageCount = (uint32_t)VulkanContext::GetSwapChainImages().size();
	initInfo.ImageCount = (uint32_t)VulkanContext::GetSwapChainImages().size();
	initInfo.CheckVkResultFn = nullptr; // Optional, a callback function for Vulkan errors
	//init_info.MSAASamples = vkMSAASamples; // The number of samples per pixel in case of MSAA
	//init_info.Subpass = 0; // The index of the subpass where ImGui will be drawn

	// Create renderpass
	RenderpassSpecification specs = {};
	specs.ColourAttachment = VulkanContext::GetSwapChainImages();
	specs.ColourLoadOp = LoadOperation::Load; 						// Change based on needs
	specs.PreviousColourImageLayout = ImageLayout::PresentSrcKHR;	// Because before this pass there is pretty much always a renderpass with Presentation

	m_Renderpass = Ref<VulkanRenderpass>::Create(specs, CommandBuffer::Create());

	ImGui_ImplVulkan_Init(&initInfo, m_Renderpass.As<VulkanRenderpass>()->GetVkRenderPass());

	// Create fonts
	io.Fonts->AddFontDefault();
	{
		auto command = VulkanCommand(true);
		ImGui_ImplVulkan_CreateFontsTexture(command.GetVkCommandBuffer());
		command.EndAndSubmit();

		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}
}

void ImGuiExtension::OnDestroyBegin()
{
	Renderer::Free([pool = s_ImGuiPool]()
	{
		vkDestroyDescriptorPool(VulkanContext::GetDevice()->GetVkDevice(), pool, nullptr);

		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	});
}

void ImGuiExtension::OnUIBegin()
{
	Renderer::Begin(m_Renderpass);

	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}
}

void ImGuiExtension::OnUIEnd()
{
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2((float)Window::Get().GetWidth(), (float)Window::Get().GetHeight());

	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), m_Renderpass->GetCommandBuffer().As<VulkanCommandBuffer>()->GetVkCommandBuffer(Renderer::GetCurrentFrame()));

	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		GLFWwindow* backup_current_context = glfwGetCurrentContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		glfwMakeContextCurrent(backup_current_context);
	}

	Renderer::End(m_Renderpass);
	Renderer::Submit(m_Renderpass);
}

void ImGuiExtension::OnEvent(Hz::Event& e)
{
	EventHandler handler(e);

	handler.Handle<WindowResizeEvent>([this](WindowResizeEvent& e) -> bool
	{
		m_Renderpass->Resize(e.GetWidth(), e.GetHeight());
		return false;
	});
}
