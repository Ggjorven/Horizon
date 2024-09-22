#pragma once

#include "Horizon/Core/Core.hpp"
#include "Horizon/Core/Events.hpp"

#include <Pulse/Types/TypeUtils.hpp>

#include <vector>

namespace Hz
{

	// Note: This class is used for easily adding extended
	// functionality to the Horizon library.
	// Note 2: This class is currently unused.
	class Extension
	{
	public:
		Extension(const std::string& name = "Unnamed Extension");
		virtual ~Extension() = default;

		virtual void OnInitBegin() {}
		virtual void OnInitEnd() {}

		virtual void OnDestroyBegin() {}
		virtual void OnDestroyEnd() {}

		virtual void OnUpdateBegin(float deltaTime) {}
		virtual void OnUpdateEnd(float deltaTime) {}

		virtual void OnRenderBegin() {}
		virtual void OnRenderEnd() {}

		virtual void OnUIBegin() {}
		virtual void OnUIEnd() {}

		virtual void OnEvent(Event& e) {}

		inline const std::string& GetName() const { return m_DebugName; }

	protected:
		std::string m_DebugName = {};
	};



	class ExtensionList
	{
	public:
		ExtensionList() = default;
		~ExtensionList() = default;

		void OnInitBegin();
		void OnInitEnd();

		void OnDestroyBegin();
		void OnDestroyEnd();

		void OnUpdateBegin(float deltaTime);
		void OnUpdateEnd(float deltaTime);

		void OnRenderBegin();
		void OnRenderEnd();

		void OnUIBegin();
		void OnUIEnd();

		void OnEvent(Event& e);

		// Adding a extension
		template<typename TExtension>
		void Add() requires (Pulse::Types::InheritsFrom<Extension, TExtension>);

		// Iterators
		inline std::vector<Unique<Extension>>::iterator			begin()			{ return m_Extensions.begin(); }
		inline std::vector<Unique<Extension>>::iterator			end()			{ return m_Extensions.end(); }
		inline std::vector<Unique<Extension>>::const_iterator	begin()	const	{ return m_Extensions.begin(); }
		inline std::vector<Unique<Extension>>::const_iterator	end() const		{ return m_Extensions.end(); }

		inline bool Empty() const { return m_Extensions.empty(); }
		
	private:
		std::vector<Unique<Extension>> m_Extensions = { };
	};



	template<typename TExtension>
	inline void ExtensionList::Add() requires (Pulse::Types::InheritsFrom<Extension, TExtension>)
	{
		m_Extensions.emplace_back(Unique<TExtension>::Create());
	}

}