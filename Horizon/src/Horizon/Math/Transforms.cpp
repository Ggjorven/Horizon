#include "hzpch.h"
#include "Transforms.hpp"

#include "Horizon/Core/Logging.hpp"

#include <glm/gtx/norm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Hz::Math
{

	static glm::vec3 Scale(const glm::vec3& v, float desiredLength)
	{
		return v * desiredLength / length(v);
	}

	void ComposeTransform(glm::mat4& transform, const glm::vec3& translation, const glm::vec3& rotation, const glm::vec3& scale)
	{
		ComposeTransform(transform, translation, glm::quat(glm::radians(rotation)), scale);
	}

	void ComposeTransform(glm::mat4& transform, const glm::vec3& translation, const glm::quat& rotation, const glm::vec3& scale)
	{
		glm::mat trans = glm::translate(glm::mat4(1.0f), translation);
		glm::mat4 rot = glm::toMat4(rotation);
		glm::mat sca = glm::scale(glm::mat4(1.0f), scale);

		transform = trans * rot * sca;
	}

	bool DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale)
	{
		glm::quat quat = {};
		bool result = DecomposeTransform(transform, translation, quat, scale);

		rotation = glm::eulerAngles(quat);

		return result;
	}

	bool DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::quat& rotation, glm::vec3& scale)
	{
		using T = float;

		glm::mat4 LocalMatrix(transform);

		if (glm::epsilonEqual(LocalMatrix[3][3], static_cast<T>(0), glm::epsilon<T>()))
			return false;


		// Assume matrix is already normalized
		HZ_ASSERT(glm::epsilonEqual(LocalMatrix[3][3], static_cast<T>(1), static_cast<T>(0.00001)), "Already normalized.");

		// Ignore perspective
		HZ_ASSERT(
			glm::epsilonEqual(LocalMatrix[0][3], static_cast<T>(0), glm::epsilon<T>()) &&
			glm::epsilonEqual(LocalMatrix[1][3], static_cast<T>(0), glm::epsilon<T>()) &&
			glm::epsilonEqual(LocalMatrix[2][3], static_cast<T>(0), glm::epsilon<T>()),
			"Ignore perspective.");

		// Next take care of translation (easy).
		translation = glm::vec3(LocalMatrix[3]);
		LocalMatrix[3] = glm::vec4(0, 0, 0, LocalMatrix[3].w);

		glm::vec3 Row[3] = {};

		// Now get scale and shear.
		for (glm::length_t i = 0; i < 3; ++i)
			for (glm::length_t j = 0; j < 3; ++j)
				Row[i][j] = LocalMatrix[i][j];

		// Compute X scale factor and normalize first row.
		scale.x = glm::length(Row[0]);
		Row[0] = Scale(Row[0], static_cast<T>(1));

		// Now, compute Y scale and normalize 2nd row.
		scale.y = glm::length(Row[1]);
		Row[1] = Scale(Row[1], static_cast<T>(1));

		// Next, get Z scale and normalize 3rd row.
		scale.z = glm::length(Row[2]);
		Row[2] = Scale(Row[2], static_cast<T>(1));

		// Rotation as quaternion
		int i, j, k = 0;
		T root, trace = Row[0].x + Row[1].y + Row[2].z;
		if (trace > static_cast<T>(0))
		{
			root = glm::sqrt(trace + static_cast<T>(1));
			rotation.w = static_cast<T>(0.5) * root;
			root = static_cast<T>(0.5) / root;
			rotation.x = root * (Row[1].z - Row[2].y);
			rotation.y = root * (Row[2].x - Row[0].z);
			rotation.z = root * (Row[0].y - Row[1].x);
		} // End if > 0
		else
		{
			static int Next[3] = { 1, 2, 0 };
			i = 0;
			if (Row[1].y > Row[0].x) i = 1;
			if (Row[2].z > Row[i][i]) i = 2;
			j = Next[i];
			k = Next[j];

			root = glm::sqrt(Row[i][i] - Row[j][j] - Row[k][k] + static_cast<T>(1.0));

			rotation[i] = static_cast<T>(0.5) * root;
			root = static_cast<T>(0.5) / root;
			rotation[j] = root * (Row[i][j] + Row[j][i]);
			rotation[k] = root * (Row[i][k] + Row[k][i]);
			rotation.w = root * (Row[j][k] - Row[k][j]);
		} // End if <= 0

		return true;
	}
}