#include "Components.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>

namespace Sandbox
{

    /*
    glm::mat4 TransformComponent::GetTransform() const
    {
        return glm::translate(glm::mat4(1.0f), Translation)
            * glm::toMat4(Rotation)
            * glm::scale(glm::mat4(1.0f), Scale);
    }

    void TransformComponent::SetTransform(const glm::mat4& transform)
    {
        Math::DecomposeTransform(transform, Translation, Rotation, Scale);
        RotationEuler = glm::eulerAngles(Rotation);
    }

    glm::vec3 TransformComponent::GetRotationEuler() const
    {
        return RotationEuler;
    }

    void TransformComponent::SetRotationEuler(const glm::vec3& euler)
    {
        RotationEuler = euler;
        Rotation = glm::quat(RotationEuler);
    }

    glm::quat TransformComponent::GetRotation() const
    {
        return Rotation;
    }

    void TransformComponent::SetRotation(const glm::quat& quat)
    {
        auto originalEuler = RotationEuler;
        Rotation = quat;
        RotationEuler = glm::eulerAngles(Rotation);

        // Attempt to avoid 180deg flips in the Euler angles when we SetRotation(quat)
        if (
            (fabs(RotationEuler.x - originalEuler.x) == glm::pi<float>()) &&
            (fabs(RotationEuler.z - originalEuler.z) == glm::pi<float>())
        )
        {
            RotationEuler.x = originalEuler.x;
            RotationEuler.y = glm::pi<float>() - RotationEuler.y;
            RotationEuler.z = originalEuler.z;
        }
    }
    */

}
