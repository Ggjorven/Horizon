#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <type_traits>

namespace Hz::Math
{
	
	// Note: These functions have not been thoroughly tested.

	// Rotation is in degrees
	void ComposeTransform(glm::mat4& transform, const glm::vec3& translation, const glm::vec3& rotation, const glm::vec3& scale);
	void ComposeTransform(glm::mat4& transform, const glm::vec3& translation, const glm::quat& rotation, const glm::vec3& scale);
	bool DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale);
	bool DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::quat& rotation, glm::vec3& scale);

	template<typename T>
	inline static T DivideAndRoundUp(T dividend, T divisor) requires (std::is_integral_v<T>)
	{
		return (dividend + divisor - 1) / divisor;
	}

	template<typename T, typename DivisorT>
	inline static T DivideAndRoundUp(T dividend, DivisorT divisor) requires (std::is_same_v<T, glm::uvec2> && std::is_integral_v<DivisorT>)
	{
		return { DivideAndRoundUp(dividend.x, divisor), DivideAndRoundUp(dividend.y, divisor) };
	}

}
