#pragma once

#include <Pulse/Core/Ref.hpp>
#include <Pulse/Core/Unique.hpp>
#include <Pulse/Core/WeakRef.hpp>

#include <Pulse/Core/Defines.hpp>

#include <Pulse/Text/Format.hpp>

#include <Pulse/Enum/Enum.hpp>

namespace Hz
{

    ///////////////////////////////////////////////////////////
    // Memory related
    ///////////////////////////////////////////////////////////
	using RefCounted = Pulse::RefCounted;

	template<typename T>
	using Ref = Pulse::Ref<T>;

	template<typename T>
	using Unique = Pulse::Unique<T>;

	template<typename T>
	using WeakRef = Pulse::WeakRef<T>;

    ///////////////////////////////////////////////////////////
    // Defines
    ///////////////////////////////////////////////////////////
    using byte = Pulse::byte;

    using i8 = Pulse::i8;
    using i16 = Pulse::i16;
    using i32 = Pulse::i32;
    using i64 = Pulse::i64;

    using u8 = Pulse::u8;
    using u16 = Pulse::u16;
    using u32 = Pulse::u32;
    using u64 = Pulse::u64;

    using f32 = Pulse::f32;
    using f64 = Pulse::f64;

    ///////////////////////////////////////////////////////////
    // Text
    ///////////////////////////////////////////////////////////
    namespace Text
    {

        template<typename ...TArgs>
        [[nodiscard]] inline std::string Format(std::string_view fmt, const TArgs& ...args)
        {
            return Pulse::Text::Format(fmt, args...);
        }

    }

    ///////////////////////////////////////////////////////////
    // Enum
    ///////////////////////////////////////////////////////////
    namespace Enum
    {
        using namespace Pulse::Enum;

        #define ENABLE_BITWISE(enum) PULSE_BIT_OPERATORS(enum)
    }

}
