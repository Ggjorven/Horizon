#pragma once

#include "Horizon/Core/Extension.hpp"

#include "Horizon/Renderer/Renderpass.hpp"

// This only works for the vulkan implementation
class ImGuiExtension : public Hz::Extension
{
public:
	void OnInitEnd() override;
	void OnDestroyBegin() override;

	void OnUIBegin() override;
	void OnUIEnd() override;

	void OnEvent(Hz::Event& e) override;

	inline const bool ImplementsUI() const override { return true; }
	inline const std::string GetName() const override { return "ImGui"; }

private:
	Hz::Ref<Hz::Renderpass> m_Renderpass = nullptr;
};