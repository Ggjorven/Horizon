#pragma once

#include <Pulse/Core/Ref.hpp>
#include <Pulse/Core/Unique.hpp>
#include <Pulse/Core/WeakRef.hpp>

namespace Hz
{

	using RefCounted = Pulse::RefCounted;


	template<typename T>
	using Ref = Pulse::Ref<T>;

	template<typename T>
	using Unique = Pulse::Unique<T>;

	template<typename T>
	using WeakRef = Pulse::WeakRef<T>;

}
