#pragma once

#include "Horizon/Core/Memory.hpp"
#include "Horizon/Core/Logging.hpp"

#include "Horizon/Renderer/RendererSpecification.hpp"
#include "Horizon/Renderer/Descriptors.hpp"

#include <array>
#include <vector>
#include <optional>
#include <filesystem>
#include <type_traits>
#include <unordered_map>

namespace Hz
{

    class VulkanShader;

	class CommandBuffer;

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Specifications
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	enum class ShadingLanguage
    {
        None = 0, GLSL, HSLS /*Not supported atm.*/, SPIRV
    };

    struct ShaderSpecification
	{
	public:
        // Note: Currently the shader code is required to be in the SPIR-V format.
        std::unordered_map<ShaderStage, const std::vector<char>> ShaderCode = { };
	};

    ///////////////////////////////////////////////////////////
    // Classes
    ///////////////////////////////////////////////////////////
    class ShaderCompiler
    {
    public:
        // Compiles GLSL/HLSL(TODO) to SPIR-V which can be remapped to any shading language
        template<ShadingLanguage Language>
        static std::vector<char> Compile(ShaderStage stage, const std::string& code)
        {
            if constexpr (Language == ShadingLanguage::GLSL)
                return CompileGLSL(stage, code);

            HZ_ASSERT(false, "Shading language passed in is currently not supported.");
            return {};
        }

        // TODO: SPIR-V Cross

    private:
        static std::vector<char> CompileGLSL(ShaderStage stage, const std::string& code);
    };

	class Shader : public RefCounted // Note: Once this object has been used for pipeline creation it can die with no consequences.
	{
    public:
		Shader() = default;
		virtual ~Shader() = default;

        virtual const ShaderSpecification& GetSpecification() const = 0;

        static std::string ReadGLSL(const std::filesystem::path& path);
		static std::vector<char> ReadSPIRV(const std::filesystem::path& path);

		static Ref<Shader> Create(const ShaderSpecification& specs);
	};

}
