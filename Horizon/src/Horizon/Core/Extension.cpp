#include "hzpch.h"
#include "Extension.hpp"

#include "Horizon/Core/Extension.hpp"

#define ALL_EXTENSIONS(func, ...)				\
	for (auto& extension : m_Extensions)		\
		extension->func(__VA_ARGS__)

namespace Hz
{

	void ExtensionList::OnInitBegin()
	{
		ALL_EXTENSIONS(OnInitBegin);
	}

	void ExtensionList::OnInitEnd()
	{
		ALL_EXTENSIONS(OnInitEnd);
	}

	void ExtensionList::OnDestroyBegin()
	{
		ALL_EXTENSIONS(OnDestroyBegin);
	}

	void ExtensionList::OnDestroyEnd()
	{
		ALL_EXTENSIONS(OnDestroyEnd);
		m_Extensions.clear();
	}

	void ExtensionList::OnUpdateBegin(float deltaTime)
	{
		ALL_EXTENSIONS(OnUpdateBegin, deltaTime);
	}

	void ExtensionList::OnUpdateEnd(float deltaTime)
	{
		ALL_EXTENSIONS(OnUpdateEnd, deltaTime);
	}

	void ExtensionList::OnRenderBegin()
	{
		ALL_EXTENSIONS(OnRenderBegin);
	}

	void ExtensionList::OnRenderEnd()
	{
		ALL_EXTENSIONS(OnRenderEnd);
	}

	void ExtensionList::OnUIBegin()
	{
		ALL_EXTENSIONS(OnUIBegin);
	}

	void ExtensionList::OnUIEnd()
	{
		ALL_EXTENSIONS(OnUIEnd);
	}

	void ExtensionList::OnEvent(Event& e)
	{
		ALL_EXTENSIONS(OnEvent, e);
	}

	bool ExtensionList::UISupport() const
	{
		for (const auto& extension : m_Extensions)
		{
			if (extension->ImplementsUI())
				return true;
		}

		return false;
	}

}